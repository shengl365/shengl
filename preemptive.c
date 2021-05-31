#include <8051.h>
	
#include "preemptive.h"


__data __at (0x30) char sp0;		//stack poiter 0-3
__data __at (0x31) char sp1;
__data __at (0x32) char sp2;
__data __at (0x33) char sp3;
__data __at (0x34) char bm;		//bitmap
__data __at (0x35) char curr_id;	//save running thread ID
__data __at (0x36) char t_slice;	//time slice (TH0 / 10)
__data __at (0x37) char tmp;		//save temporary address
__data __at (0x38) char timecount;	//#TF0(interrupt)


#define SAVESTATE \
        { __asm \
          	push ACC \
		push B \
		push DPL \
		push DPH \
		push PSW \
         __endasm; \
         if(curr_id == 0) sp0 = SP; \
	 else if(curr_id == 1) sp1 = SP;\
	 else if(curr_id == 2) sp2 = SP;\
	 else if(curr_id == 3) sp3 = SP;\
        }

#define RESTORESTATE \
        {if(curr_id == 0) SP = sp0; \
	 else if(curr_id == 1) SP = sp1; \
	 else if(curr_id == 2) SP = sp2; \
	 else if(curr_id == 3) SP = sp3; \
	__asm \
          	pop PSW \
		pop DPH \
		pop DPL \
		pop B \
		pop ACC \
         __endasm; \
        }

extern void main(void);

void Bootstrap(void) {

	TMOD = 0;  // timer 0 mode 0
	IE = 0x82;  // enable timer 0 interrupt; keep consumer polling
                // EA=1  -  ET2  ES  ET1  EX1  ET0=1  EX0
	TR0 = 1; // start timer 0
	bm = 0;

	ThreadCreate(main);
	//curr_id = 0;
	
	RESTORESTATE;

}


ThreadID ThreadCreate(FunctionPtr fp) {

	while(bm == 15);
	EA = 0;

	//a.update bitmask
	if(!(1 & bm)) {bm = bm+1;   curr_id = 0;}
	else if(!(2 & bm)) {bm = bm+2;   curr_id = 1;}
	else if(!(4 & bm)) {bm = bm+4;   curr_id = 2;}
	else if(!(8 & bm)) {bm = bm+8;   curr_id = 3;}

	//b.starting stack location for new thread
	
	if(curr_id == 0) sp0 = 0x3F; 
	else if(curr_id == 1) sp1 = 0x4F; 
	else if(curr_id == 2) sp2 = 0x5F;
	else if(curr_id == 3) sp3 = 0x6F;
	

        //c1.save the current SP in a temporary
	tmp = SP;

        //c2.set SP to the starting location for the new thread

	if(curr_id == 0) SP = sp0; 
	else if(curr_id == 1) SP = sp1;
	else if(curr_id == 2) SP = sp2;
	else if(curr_id == 3) SP = sp3;  

	//d.push the return address fp
	__asm
		push DPL
		push DPH
	__endasm;


	//e.assign a register to 0 and push it four times for ACC, B, DPL, DPH.
	ACC = 0;
	B = 0;
	DPL = 0;
	DPH = 0;


	
	//f.push PSW (processor status word)

	if(curr_id == 0) PSW = 0; 
	else if(curr_id == 1) PSW = 8; 
	else if(curr_id == 2) PSW = 16;
	else if(curr_id == 3) PSW = 24; 


	__asm	
		push ACC
		push B
		push DPL
		push DPH
		push PSW
	__endasm;

	//g.write the current stack pointer to the saved stack pointer
	if(curr_id == 0)sp0 = SP; 
	else if(curr_id == 1)sp1 = SP;
	else if(curr_id == 2)sp2 = SP;
	else if(curr_id == 3)sp3 = SP;


	//h.set SP to the saved SP in step c.

	SP = tmp;


	//i.finally, return the newly created thread ID.
	EA =1;
	return curr_id;

}
void ThreadYield(void) {
       SAVESTATE;
		
	if((2 & bm)) {curr_id = 1;}
	else if((4 & bm)) {curr_id = 2;}
	else if((8 & bm)) {curr_id = 3;}
	else if((1 & bm)) {curr_id = 0;}

       RESTORESTATE;
}

void myTimer0Handler(void) {
	EA = 0;
	SAVESTATE;
	//context switch:
	//0011
	if(bm==3){
		if(curr_id == 0) curr_id =1;
		else if(curr_id == 1) curr_id = 0;
	}
	//0101
	else if(bm==5){
		if(curr_id == 0) curr_id =2;
		else if(curr_id == 2) curr_id = 0;
	}
	//1001
	else if(bm==9){
		if(curr_id == 0) curr_id =3;
		else if(curr_id == 3) curr_id = 0;
	}
	//0110
	else if(bm==6){
		if(curr_id == 1) curr_id =2;
		else if(curr_id == 2) curr_id = 1;
	}
	//1010
	else if(bm==10){
		if(curr_id == 1) curr_id =3;
		else if(curr_id == 3) curr_id = 1;
	}
	//1100
	else if(bm==12){
		if(curr_id == 2) curr_id =3;
		else if(curr_id == 3) curr_id = 2;
	}
	//0111
	else if(bm==7){
		if(curr_id == 0) curr_id =1;
		else if(curr_id == 1) curr_id = 2;
		else if(curr_id == 2) curr_id = 0;
	}
	//1011
	else if(bm==11){
		if(curr_id == 0) curr_id =1;
		else if(curr_id == 1) curr_id = 3;
		else if(curr_id == 3) curr_id = 0;
	}
	//1101
	else if(bm==13){
		if(curr_id == 0) curr_id =2;
		else if(curr_id == 2) curr_id = 3;
		else if(curr_id == 3) curr_id = 0;
	}
	//1110
	else if(bm==7){
		if(curr_id == 1) curr_id =2;
		else if(curr_id == 2) curr_id = 3;
		else if(curr_id == 3) curr_id = 1;
	}
	//1111
	else if(bm==15){
		if(curr_id == 0) curr_id =1;
		else if(curr_id == 1) curr_id = 2;
		else if(curr_id == 2) curr_id = 3;
		else if(curr_id == 3) curr_id = 0;
	}

	RESTORESTATE;
	EA = 1;
	__asm
		reti
	__endasm;

}

void ID_change(){
	__critical{
		if(curr_id == 1) curr_id =0;
		else if(curr_id == 2) curr_id =0;
		else if(curr_id == 3) curr_id =0;
	}
}


//delays the thread by n time units
void delay(char n) {


	timecount=0;
	//ThreadYield();
	while(timecount < n) 
		while(TF0==0)
			timecount++;

}

//returns the “current time”
char now(void) {

	t_slice = TH0/0x10;
	if(t_slice ==0x0) return '0';
	else if(t_slice ==0x1) return '1';
	else if(t_slice ==0x2) return '2';
	else if(t_slice ==0x3) return '3';
	else if(t_slice ==0x4) return '4';
	else if(t_slice ==0x5) return '5';
	else if(t_slice ==0x6) return '6';
	else if(t_slice ==0x7) return '7';
	else if(t_slice ==0x8) return '8';
	else if(t_slice ==0x9) return '9';
	else if(t_slice ==0xA) return 'A';
	else if(t_slice ==0xB) return 'B';
	else if(t_slice ==0xC) return 'C';
	else if(t_slice ==0xD) return 'D';
	else if(t_slice ==0xE) return 'E';
	else return 'F';

}

void ThreadExit(void) {

	EA = 0;
	if(curr_id == 0) {bm = bm-1; sp0 = 0x3F;} 
	else if(curr_id == 1) {bm = bm-2; sp1 = 0x4F;} 
	else if(curr_id == 2) {bm = bm-4; sp2 = 0x5F;}
	else if(curr_id == 3) {bm = bm-8; sp3 = 0x6F;}


	while(bm==0); //exit loop here

	//else switch to exist thread

	if((2 & bm)) {curr_id = 1; SP = sp1;}
	else if((4 & bm)) {curr_id = 2; SP = sp2;}
	else if((8 & bm)) {curr_id = 3; SP = sp3;}
	else if((1 & bm)) {curr_id = 0; SP = sp0;}

	__asm	
		pop PSW
		pop DPH
		pop DPL
		pop B
		pop ACC
	__endasm;
	EA = 1;	

}

char get_currid(void) {
	return curr_id;
}


#include <8051.h>
#include "preemptive.h"
#define SPOT_SIZE 2

__data __at (0x20) char thread;			//pointer to running car_number
__data __at (0x21) char car_glob;		//global car_number
__data __at (0x22) char c;			//c,i for iterator
__data __at (0x23) char i;
__data __at (0x24) char next_char;		//parking string pointer
__data __at (0x25) char time;			//exit string pointer
__data __at (0x26) char spots[SPOT_SIZE];	//spot[0-1]=0(empty) or car_number
__data __at (0x28) char s;			//spot ascii number 31-32
__data __at (0x29) char sp;			//spot pointer 0-1
__data __at (0x2A) char park;			//park, leave save number of car
__data __at (0x2B) char leave;

__data __at (0x2D) char sem_spot;		//sem to block getting parking spot
__data __at (0x2E) char sem_cars;		//sem to block car thread create
__data __at (0x3A) char cars[5];		//car ascii number

char time_stamp(void){

	char t_text[] = "\n(car..exit.at:..";
	t_text[6] = leave;
	return t_text[i++];
}


char string_parking(void){

	char text[] = "\n(car..gotspot..at..";
	text[6] = park;	//car0
	text[13] = s;	//spot1/0
	return text[i++];
}

void Park(void) {

	//check if empty spot
	while(spots[sp]!=0) {
		EA = 0;
		sp=(sp+1)%2;
		EA = 1;
	}
	//no int wile get spot
	SemaphoreWait(sem_spot);

	//get_currid
	thread=get_currid();
	//cars[1]=1
	cars[thread] = car_glob;
	park = cars[thread];
	car_glob = car_glob+1;

	//get spot[]
	spots[sp] = cars[thread];

	//mark spot number
	if (sp==0) s='0';
	else s='1';
	SemaphoreSignal(sem_spot);

	EA = 0;
	c=0;
	i=0;

	for(c=0; c<21 ;c++) {
		if(c==20) next_char=now();
		else next_char = string_parking();
		SBUF = next_char;
		while(TI==0) {};
		TI=0; 

	}
	EA = 1;

	delay(0x10);

	c=0;
	i=0;
	thread=get_currid();
	if(spots[0]== cars[thread]) sp=0;
	if(spots[1]== cars[thread]) sp=1;

	SemaphoreWait(sem_spot);
	leave=spots[sp];
	spots[sp] = 0;
	SemaphoreSignal(sem_spot);

	EA=0;
	for(c=0; c<19;c++) {
		if(c==18) time=now();	
		else time = time_stamp();
		SBUF = time;
		while(TI==0) {};
		TI=0; 
	}
	EA=1;

	SemaphoreSignal(sem_cars);

	ThreadExit();
}


void main(void) {
	TMOD |=0x20;

	TH1 =-6;
	SCON =0x50;
	TR1 =1;

	SemaphoreCreate(sem_spot, 1);
	SemaphoreCreate(sem_cars, MAXTHREADS);          
	spots[0]=spots[1]=0;
	s='0';
	sp=0;
		
	car_glob = '0';
	thread=0;
	cars[0]=cars[1]=cars[2]=cars[3]=cars[4]=0;

	while(1){
		if(car_glob >= '4') break;
		SemaphoreWait(sem_cars);
		ThreadCreate(Park);
		ID_change();
	}
	
	while(1);
	ThreadExit();

}

void _sdcc_gsinit_startup(void) {
        __asm
                ljmp _Bootstrap
        __endasm;
}

void _mcs51_genRAMCLEAR(void) {}
void _mcs51_genXINIT(void) {}
void _mcs51_genXRAMCLEAR(void) {}

void timer0_ISR(void) __interrupt(1){
	__asm
		ljmp _myTimer0Handler
	__endasm;
}

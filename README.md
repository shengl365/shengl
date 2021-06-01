# Multithreading on 8051

### one car => one thread
### one spot => one resource

#### Each parking lot only for one car, here exist 4 cars and 2 spot

The UART transfers the following characters (not always the same)

> car0 gotspot0 \
> car1 gotspot1 \
> car1 exit \
> car2 gotspot1 \
> car0 exit \
> car2 exit \
> car3 gotspot0 \
> car3 exit

#### Configure explanation:

+ PSW = 00000000 (thread0) 00001000 (thread1) 00010000 (thread2) 00011000 (thread3)
+ IE = 10000010 (Enable EA=1 and ET=1 that enable timer interrupt)
+ SP => current stack pointer (sp0~sp3 save stack pointer of each thread)
+ bm => global bitmap of threads (bm=0001 => thread0 exist, bm=0011 => thread0 and thread1 exist)


#### Context Switch (RR)

If there are only thread0 and thread1, we context switch between thread ID 0 and 1
```
void myTimer0Handler(void) {
	EA = 0;
	SAVESTATE;
	//context switch:
	//0011
	if(bm==3){
		if(curr_id == 0) curr_id =1;
		else if(curr_id == 1) curr_id = 0;
```

#### Critical Region

For getting spots, cars are blocked if there is no enough spot(resource)
```
	while(spots[sp]!=0) {
		EA = 0;
		sp=(sp+1)%2;
		EA = 1;
```

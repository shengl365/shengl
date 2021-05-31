#define MAXTHREADS 4

#define CNAME(s) _ ## s

#define CLABEL(label) label ## $

#define SemaphoreCreate(s, n)  s = n // create a counting semaphore s 
                                  // that is initialized to n

#define SemaphoreSignal(s) \
	{__asm \
		inc CNAME(s) \
	 __endasm;}     

#define SemaphoreWait(s) SemaphoreWaitBody(s, __COUNTER__)

#define SemaphoreWaitBody(s, label) \
    { __asm \
CLABEL(label):  mov A, CNAME(s) \
       		jz CLABEL(label) \
		jb ACC.7, CLABEL(label) \
       		dec CNAME(s) \
      __endasm; } //


typedef char ThreadID; 
//where a ThreadID is a type for thread ID.  It is represented by a single byte, which is the native word size for 8051.
typedef void (*FunctionPtr)(void);  
//a FunctionPtr is a type for a function pointer, for the purpose of attaching code to a thread.  SDCC compiles it to a 2-byte pointer (assumed to be code space).

//API
ThreadID ThreadCreate(FunctionPtr);  
//This is the primary API for creating (and starting) a thread, and return its thread ID.  Once created, the thread is eligible to run.
void ThreadYield(void);  
//This is the API for a thread to yield its control to another thread, if any.  Later, it can resume control immediately after the ThreadYield() statement.
void ThreadExit(void);  
//This is the code for a thread to exit itself either explicitly or implicitly so that the resources can be recycled for another thread. For now we will not use it, but we will use it in a later assignment.

void ID_change();
void delay(char n);
char now(void);
char get_currid(void);

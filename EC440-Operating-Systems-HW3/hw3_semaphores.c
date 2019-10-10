#include <stdio.h>		// standard i/o included
#include <setjmp.h>		// included for usage of jmp_buf, setjmp(), and longjmp()
#include <stdlib.h>		// as per example on project2_discussion.pdf (pg. 13)
#include <pthread.h>	// included for function prototypes of pthread.h -- will implement definitions
#include <unistd.h>		// for use of ualarm()
#include <signal.h>		// for signal handler
#include <stdbool.h>	// for bool type
#include <semaphore.h>	// for semaphores

/* for states */
#define DEFAULT 0		// define the initial DEFAULT state (when struct thread is initialized, state is defaulted to 0)
#define READY 	1		// define state to be READY
#define RUNNING 2		// define state to be RUNNING
#define EXITED 	3		// define state to be EXITED
#define BLOCKED 4		// define state to be BLOCKED

/* for the jmp_buf */
#define JB_BX	0		// irrelevant for this project
#define JB_SI	1		// irrelevant for this project
#define JB_DI	2		// irrelevant for this project
#define JB_BP	3		// irrelevant for this project
#define JB_SP	4		// the stack pointer which should be modified manually with the heap
#define JB_PC	5		// the program counter which should be modified with the program address
#define MAX_TH	128		// the maximum numbers of threads

/* CREATING THE TCB 
 * define an object for threads so that threads are easily created 
 */
struct thread
{
	/* Thread ID */
	int thread_id;		// each thread needs an ID
	
	/* Thread Register Information */
	jmp_buf regis;		// jmp_buf holds register values
	
	/* Thread Stack Information */
	void * stkptr;		// pointer to the stack that's allocated on the heap

	/* Information About Thread Status */
	int thd_state;		// 0 for ready, 1 for running, 2 for exited/exiting

	thread * next_thread;
};

struct semaphore
{
	int counter;
	thread * first_thread;
	thread * last_thread;
	int initialized;
};

struct thread threads[MAX_TH];		// array of thread pointers that are currently NULL until stated otherwise
int active_thread_index = 0;		// active thread for when schedule() is run
int num_queued_threads = 0;			// there are currently 0 actively queued threads
unsigned int new_thread_id = 1;		// first thread ID that is usuable
struct sigaction handler;			// create a signal handler globally

sem_t new_sem;


/* GLOBAL VARIABLES FOR THE PTHREAD_T OBJECT 
 * & initialize the pthread_t global variables as well as other things necessary for the library
 */


int sem_init(sem_t * sem, int pshared, unsigned value) {

	struct semaphore * new_sem = malloc(sizeof(semaphore));		
	sem->__align = (long int)(new_sem);							// sem.__align is a pointer to our semaphore struct
	new_sem->counter = value;
	new_sem->first_thread = NULL;
	new_sem->initialized = 1;

}

int sem_wait(sem_t * sem) {

	int value = ((sem.__align)->counter);
	
	if(value > 0) {
		((sem.__align)->counter)--;
		return 1;
	}
	else if (value == 0) {
		threads[current_thread].state = BLOCKED;
		// add it to end of the queue
		schedule();
	}
	return 0;

}

int sem_post(sem_t * sem) {

	int value = ((sem.__align)->counter);
	if (value > 0) {		// no one is waiting
		value++;
		return 1;
	}
	else if (value == 0) {	//
		// check queue to see if there is anyone blocked
		// if yes: then wake up first one (set status as not blocked and dequeue it)
		return 1;
	}
	return 0;
}

int sem_destroy(sem_t * sem) {
	if ((sem.__align)->initialized) {
		// free resources
	}
	return 1;
}


//void schedule();					// function prototype for signal(SIGALRM, schedule) to work

void init_sigact()
{
	handler.sa_handler = schedule;						// set the handler variable to schedule
	handler.sa_flags = SA_NODEFER;						// use the flag mentioned before
	sigaction(SIGALRM, &handler, NULL);					// put it together? :D
	ualarm(50000, 50000);								// set up ualarm if num of "existing" threads is 0 and pthread_create() is successfully called
	// fprintf(stderr,"set up the alarm and handler\n");	
}

/* MANGLE AND DEMANGLE ALGORITHMS
 * given functions for mangling and demangling a pointer 
 */
static int ptr_mangle(int p)
{
 	unsigned int ret;
 	asm(" movl %1, %%eax;\n"
 		" xorl %%gs:0x18, %%eax;"
 		" roll $0x9, %%eax;"
 		" movl %%eax, %0;"
 	: "=r"(ret)
 	: "r"(p)
 	: "%eax"
 	);
 	return ret;
}

static int ptr_demangle(int p)
{
	unsigned int ret;
	asm( " movl %1, %%eax;\n"
		" rorl $0x9, %%eax;"
		" xorl %%gs:0x18, %%eax;"
		" movl %%eax, %0;"
	: "=r"(ret)
	: "r"(p)
	: "%eax"
	);
	return ret;
}

/* PTHREAD_CREATE()
 * implement pthread_create(), needed to create a thread 
 */
int pthread_create(
	pthread_t * thread,					// pointer to the thread struct (?)
	const pthread_attr_t * attr,		// always NULL per info given by project2_discussion.pdf
	void * (*start_routine)(void *),	// this is a function pointer to the function, i.e. program, being ran by the thread
	void * arg)							// void pointer to the argument of the function mentioned above
{

	/* When does pthread_create return non-zero value? */
	if(attr != NULL) return 1;			// if return value is 1, then attr was not NULL (for sake of this project)
	if(num_queued_threads == MAX_TH)	// if the number of threads is at a maximum
		return -1;						// return -1 because max threads reached
	
	/* Set Up Alarm */
	if(num_queued_threads == 0) 
	{
		init_sigact();					// call init_sigact()
		threads[0].thread_id = 0;		// thread_id = 0;		
		threads[0].thd_state = READY;	// main is ready!
		setjmp(threads[0].regis);		// setjmp to preserve main's registers
		num_queued_threads++;			// increment total number of threads
	}

	/* Find Next Free TCB */
	bool new_tcb_found = 0;						// BOOL to see if an index has already been found
	int new_tcb_index;							// the new index -- has to find one because queued threads are not at MAX_TH
	int i;										// used for indexing
	for(i = 1; i < MAX_TH; i++)
	{
		if(!new_tcb_found && 					// no free index has been found yet
		   (threads[i].thd_state == EXITED || 
			threads[i].thd_state == DEFAULT))   // EXITED state means FREE to be used
		{
			new_tcb_found = 1;					// set BOOL to indicate that index has been found
			new_tcb_index = i;					// note the index
		}
	}

	/* Thread ID */
	threads[new_tcb_index].thread_id = new_thread_id;	// give the thread some numerical ID
	* thread = new_thread_id;							// indicate new_thread_ID
	new_thread_id++;									// increment the new thread ID

	/* State of Thread */
	threads[new_tcb_index].thd_state = READY;			// set the state to '0' to indicate "READY"

	/* Set Up Stack */
	threads[new_tcb_index].stkptr = malloc(32767);							// allocate 32767 bytes of memory as described by the project documentation
	void * stack_ptr = threads[new_tcb_index].stkptr;						// make a copy of the stkptr because u cant free any other pointer but the one u malloc'd

	/* Insert Args & pthread_exit() */
	stack_ptr = stack_ptr + 32767;											// set the pointer to the top of the heap since the stack grows down
	stack_ptr = stack_ptr - 4;												// subtract 4 to put the next value in
	*((unsigned long int*)stack_ptr) = (unsigned long int) arg;				// next value is arg 
	stack_ptr = stack_ptr - 4;												// subtract 4 since the next thing we put in is also 4 bytes
	*((unsigned long int*)stack_ptr) = (unsigned long int) pthread_exit;	// put the location of the address in for pthread_exit()

	setjmp(threads[new_tcb_index].regis);																	// context switch all of the current registers -- then modify in next two lines																				
	threads[new_tcb_index].regis[0].__jmpbuf[JB_SP] = ptr_mangle((unsigned long int)stack_ptr);				// mangle the stack pointer (ESP) and store into JB_SP
	threads[new_tcb_index].regis[0].__jmpbuf[JB_PC] = ptr_mangle((unsigned long int)start_routine);			// mangle the pointer to the program (PC) and store into JB_PC

	num_queued_threads++;	// increment total number of threads

	schedule();
	return 0;	// should return 0 upon success -- however, how would I know if something was unsuccessful??
}

/* PTHREAD_EXIT()
 * implement pthread_exit(), used to exit threads 
 */
void pthread_exit(void * value_ptr)
{
	threads[active_thread_index].thd_state = EXITED;	// set state to EXITED
	free(threads[active_thread_index].stkptr);			// free the stack
	num_queued_threads--;								// decrement number of queued threads
	schedule();
	exit(0);
}

/* SCHEDULE()
 * used to implement the round robin algorithm where threads are incremented through
 * until a ready one is found
 */
void schedule()
{
	bool next_thread_found = 0;									// bool for when the next thread is found
	int current_thread = active_thread_index;					// keep track of current thread
	int count = 0;												// creating the count so that you don't loop more than once

	if(setjmp(threads[current_thread].regis))				    // at this point, the next active thread is found, so setjmp previous thread
		return;

	while(!next_thread_found && count < 128)					// repeat while a new thread is not found (or if u loop an entire circle)
	{
		if(active_thread_index == 127)							// if the index reaches 127, go to 0 next
			active_thread_index = 0;							
		else active_thread_index++;								// else, increment by 1

		if(threads[active_thread_index].thd_state == READY)		// if the next thread is ready
			next_thread_found = 1;								// set the bool to be 1

		count++;												// increment count
	}
	
	if(!next_thread_found) 										// if a thread is never found
		active_thread_index = current_thread;					// then just run the previous thread (this only occurs if main is the only thread)
	longjmp(threads[active_thread_index].regis, 1);				// longjmp the new active thread
}

/* PTHREAD_SELF()
 * Used to return the ID of the current active thread 
 */
pthread_t pthread_self(void)
{
	return threads[active_thread_index].thread_id;		// return the ID of the current thread
}







/* BELOW IS TEST CODE FOR SELF TESTING -- LEAVE COMMENTED */
// #define THREAD_CNT 3

// // waste some time
// void *count(void *arg) {
//         unsigned long int c = (unsigned long int)arg;
//         int i;
//         int j;
//         for (i = 0; i < c; i++) {
//                 if ((i % 10000) == 0) {
//                         printf("tid: 0x%x Just counted to %d of %ld\n",
//                         (unsigned int)pthread_self(), i, c);
//                 }
//         }
//     return arg;
// }

// int main(int argc, char **argv) {
//         pthread_t threads[THREAD_CNT];
//         int i;
//         unsigned long int cnt = 10000000;

//     // //create THRAD_CNT threads
//         for(i = 0; i<THREAD_CNT; i++) {
//                 pthread_create(&threads[i], NULL, count, (void *)((i+1)*cnt));
//         }

//         while(num_queued_threads > 1)
//         {

//         }

//     return 0;
// }
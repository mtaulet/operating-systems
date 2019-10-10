/*
*	Marta Taulet
*	EC440 Project 2
*	threads.c
*/

#include <pthread.h>
#include <stdio.h>
#include <setjmp.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#define MAX_NUM_THREADS 128
#define THREAD_NUM 5

#define STACK_SIZE_BYTES 32767  	// bytes
#define STACK_SIZE_INT 8191			// ints = bytes/4
#define QUANTUM 50    				// ROUND ROBIN QUANTUM (ms)

// Registers
#define JB_BX 0
#define JB_SI 1
#define JB_DI 2
#define JB_BP 3
#define JB_SP 4
#define JB_PC 5

enum Status {
	Created,
	Ready,
	Running,
	Exited
};


struct Thread {
	int id;
	enum Status status;
	unsigned long * stack;
	jmp_buf registers;
	int sp, pc;
};

static int runningThread = 0;
static struct Thread TCB_table [MAX_NUM_THREADS];
static int createdThreads [MAX_NUM_THREADS] = {0};
static int active_threads = 0;

int thread_self(void){
	return TCB_table[runningThread].id;
}

void * count (void* arg) {

	TCB_table[runningThread].status = Running;

  unsigned long int c = \
    (unsigned long int)arg;
  int i;
  for (i = 0; i < c; i++) {
    if ((i % 100) == 0) {
      printf("id: %x cntd to %d of %ld\n", \
      (unsigned int)thread_self(), i, c);
    }
  }
  return arg;
}



void scheduler() {

	int * arg;

	if(setjmp(TCB_table[runningThread].registers) == 0)   // First time calling setjump
	{
		runningThread++;
		while(TCB_table[runningThread].status != Ready) 
		{
			runningThread = (runningThread + 1) % MAX_NUM_THREADS;
		}
		if (TCB_table[runningThread].status == Created) {
			TCB_table[runningThread].registers[0].__jmpbuf[JB_SP] = TCB_table[runningThread].sp;
			TCB_table[runningThread].registers[0].__jmpbuf[JB_PC] = TCB_table[runningThread].pc; // DOES IT JUMP TO star_funct RIGHT HERE?
			TCB_table[runningThread].status = Ready;

		}
		else {
		}
		longjmp(TCB_table[runningThread].registers, 1);

	}
	else				// When longjump is called
	{
		// Resume execution
		printf("Resuming execution of thread %d\n", thread_self());
	}
}



static int ptr_mangle(int p) {
	unsigned int ret;
	asm(" movl %1, %%eax;\n"
	" xorl %%gs:0x18, %%eax;" " roll $0x9, %%eax;"
	" movl %%eax, %0;"
	: "=r"(ret)
	: "r"(p)
	: "%eax"
	);
	return ret;
}

// Clean up a thread's resources
void thread_exit(void *value_ptr)
{
	createdThreads[runningThread] = 0;
	TCB_table[runningThread].status = Exited;
	//TCB_table[runningThread].exit_status = value_ptr; // exit status should be set during start routine

	if(TCB_table[runningThread].stack != NULL)
		free(TCB_table[runningThread].stack);

	active_threads--;
	scheduler();

}


// Initializes signals and timers for scheduling interrupts
// - Sets signal handler as the scheduling function
void initialize_subsystem()
{
	printf("initialize_subsystem\n");
	// Add main to RCB table
	TCB_table[0].id = 0;
	TCB_table[0].status = Ready;
	TCB_table[0].stack = NULL;
	setjmp(TCB_table[0].registers);
	active_threads++;

	struct sigaction sig;
	struct itimerval timer;

	memset(&sig, 0, sizeof(sig));
	sig.sa_sigaction = scheduler;
	sig.sa_flags = SA_NODEFER;
	sigaction(SIGALRM, &sig, NULL);

	// - Set to go off after 10usec time then every QUANTUM (ms) afterwards
	timer.it_value.tv_usec = 10;
	timer.it_value.tv_sec = 0;
	timer.it_interval.tv_usec = QUANTUM * 1000;
	timer.it_interval.tv_sec = 0;	
	setitimer(ITIMER_VIRTUAL, &timer, NULL);
}

int thread_create( int * thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg) 
{

	createdThreads[0] = 1;
	int i;
	for(i = 0; i<=MAX_NUM_THREADS; i++) {

		if(i == MAX_NUM_THREADS) {
			printf("ERROR: Maximum threads already created.\n");
			return -1;
		}
		if(createdThreads[i] == 0) {
			break;
		}
	}
	runningThread = i;
	*thread = i;
	TCB_table[i].id = i;
	TCB_table[i].stack = malloc(sizeof(int)*STACK_SIZE_INT);

	TCB_table[i].stack[STACK_SIZE_INT-2] = (unsigned int) thread_exit;
	TCB_table[i].stack[STACK_SIZE_INT-1] = (unsigned long)arg;
	
	// Registers:
	TCB_table[i].sp = ptr_mangle((int) (TCB_table[i].stack + STACK_SIZE_INT - 2));
	TCB_table[i].pc = ptr_mangle((int) start_routine);

	setjmp(TCB_table[i].registers);

	active_threads++;
	createdThreads[i] = 1;

	TCB_table[i].status = Created;

	printf("Thread %d created\n", thread_self());

	 
	if(active_threads == 1)
		initialize_subsystem();

	return 0;
}



int main(int argc, char **argv) {
  int threads[THREAD_NUM];
  int i, j, k;
  int limit = 100000000;
  unsigned long int cnt = 10000;

  for(i = 0; i<THREAD_NUM; i++) {
    thread_create(&threads[i], NULL, count, (void *)((i+1)*cnt));
  }

  for(j = 0; j<limit; j++) {
  	k++;
  }
  printf("k is %d\n", k);
  return 0;
}





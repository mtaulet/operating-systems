/*
*	Marta Taulet
*	EC440 Project 2
*	threads.c
*/


#include <pthread.h>

enum Status {
	Ready,
	Running,
	Exited
};


struct Thread {			// thread control block (TCB)

	pthread_t id;
	enum Status status;
	unsigned long * stack;
	int registers [REG_NUM];  // Maybe there is a better way to do this
	jmp_buf registers;


};


// int pthread_create(pthread_t *thread, const pthread_attr_t  *attr, void *(*start_routine) (void *), void *arg);

// void pthread_exit(void *value_ptr);

// void schedule();

pthread_t pthread_self(void);
/* Returns the thread ID of the calling thread. */
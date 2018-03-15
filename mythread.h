<<<<<<< HEAD
#include <stdio.h>
#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>
#include <ucontext.h>
#include <unistd.h>

#include "interrupt.h"

#define N 10
#define FREE 0
#define INIT 1
#define WAITING 2
#define IDLE 3

#define STACKSIZE 10000
#define QUANTUM_TICKS 40

#define LOW_PRIORITY 0
#define HIGH_PRIORITY 1
#define SYSTEM 2
/* Structure containing thread state  */
typedef struct tcb
{
	int state; /* the state of the current block: FREE or INIT */
	int tid; /* thread id*/
	int priority; /* thread priority*/
	int ticks;
	void(*function)(int);  /* the code of the thread */
	ucontext_t run_env; /* Context of the running environment*/
}
TCB;

int mythread_create(void(*fun_addr)(), int priority); /* Creates a new thread with one argument */
void mythread_setpriority(int priority); /* Sets the thread priority */
int mythread_getpriority(); /* Returns the priority of calling thread*/
void mythread_exit(); /* Frees the thread structure and exits the thread */
int mythread_gettid(); /* Returns the thread id */
int read_network(); /* */
=======
#include <stdio.h>
#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>
#include <ucontext.h>
#include <unistd.h>

#include "interrupt.h"

#define N 10
#define FREE 0
#define INIT 1
#define WAITING 2
#define IDLE 3

#define STACKSIZE 10000
#define QUANTUM_TICKS 40

#define LOW_PRIORITY 0
#define HIGH_PRIORITY 1
#define SYSTEM 2
/* Structure containing thread state  */
typedef struct tcb{
  int state; /* the state of the current block: FREE or INIT */
  int tid; /* thread id*/
  int priority; /* thread priority*/
  int ticks;
  void (*function)(int);  /* the code of the thread */
  ucontext_t run_env; /* Context of the running environment*/
}TCB;

int mythread_create (void (*fun_addr)(), int priority); /* Creates a new thread with one argument */
void mythread_setpriority(int priority); /* Sets the thread priority */
int mythread_getpriority(); /* Returns the priority of calling thread*/
void mythread_exit(); /* Frees the thread structure and exits the thread */
int mythread_gettid(); /* Returns the thread id */
int read_network(); /* */
>>>>>>> parent of 298597b... fixed

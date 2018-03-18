#include <stdio.h>
#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>
#include <ucontext.h>
#include <unistd.h>

#include "mythread.h"
#include "interrupt.h"

#include "queue.h"

TCB* scheduler ();
void activator ();
void timer_interrupt (int sig);
void network_interrupt (int sig);

/* Array of state thread control blocks: the process allows a maximum of N threads */
static TCB t_state[N]; 

/* Current running thread */
static TCB* running;
static int current = 0;

/* Variable indicating if the library is initialized (init == 1) or not (init == 0) */
static int init = 0;

/* Creación de cola */
struct queue *thread_q;

/* Thread control block for the idle thread */
static TCB idle;
static void idle_function (){
  while (1);
}

/* Initialize the thread library */
void init_mythreadlib ()
{
	int i;  
	/* Create context for the idle thread */
	if (getcontext (&idle.run_env) == -1)
	{
		perror ("*** ERROR: getcontext in init_thread_lib");
		exit (-1);
	}
	idle.state = IDLE;
	idle.priority = SYSTEM;
	idle.function = idle_function;
	idle.run_env.uc_stack.ss_sp = (void *)(malloc (STACKSIZE));
	idle.tid = -1;
	if (idle.run_env.uc_stack.ss_sp == NULL)
	{
		printf ("*** ERROR: thread failed to get stack space\n");
		exit (-1);
	}
	idle.run_env.uc_stack.ss_size = STACKSIZE;
	idle.run_env.uc_stack.ss_flags = 0;
	idle.ticks = QUANTUM_TICKS;
	makecontext (&idle.run_env, idle_function, 1);
	t_state[0].state = INIT;
	t_state[0].priority = LOW_PRIORITY;
	t_state[0].ticks = QUANTUM_TICKS;
	if (getcontext (&t_state[0].run_env) == -1)
	{
		perror ("*** ERROR: getcontext in init_thread_lib");
		exit (5);
	}	

	for (i=1; i<N; i++)
	{
		t_state[i].state = FREE;
	}
	
	t_state[0].tid = 0;
	running = &t_state[0];
	/*inicializamos la cola*/
	thread_q = queue_new ();
	/* Initialize network and clock interrupts */
	init_network_interrupt ();
	init_interrupt ();
}


/* Create and intialize a new thread with body fun_addr and one integer argument */
int mythread_create (void (*fun_addr)(), int priority)
{
	int i;
	if (!init)
	{
		init_mythreadlib();
		init=1;
	}
	for (i=0; i<N; i++)
	{
		if (t_state[i].state == FREE)
		{
			break;
		}
	}
	if (i == N)
	{
		return (-1);
	}
	if (getcontext (&t_state[i].run_env) == -1)
	{
		perror ("*** ERROR: getcontext in my_thread_create");
		exit (-1);
	}
	t_state[i].state = INIT;
	t_state[i].priority = priority;
	t_state[i].function = fun_addr;
	t_state[i].run_env.uc_stack.ss_sp = (void *)(malloc (STACKSIZE));
	if (t_state[i].run_env.uc_stack.ss_sp == NULL)
	{
		printf ("*** ERROR: thread failed to get stack space\n");
		exit (-1);
	}
	t_state[i].tid = i;
	t_state[i].run_env.uc_stack.ss_size = STACKSIZE;
	t_state[i].run_env.uc_stack.ss_flags = 0;
	makecontext (&t_state[i].run_env, fun_addr, 1);
	/*Se añade rodaja al hilo */
	t_state[i].ticks = QUANTUM_TICKS;
	/*Se añade el hilo a la cola */
	TCB * t = & t_state [i];
	disable_interrupt();
	enqueue (thread_q , t ) ;
	enable_interrupt();
	return i;
} /****** End my_thread_create() ******/

/* Read network syscall */
int read_network ()
{
   return 1;
}

/* Network interrupt  */
void network_interrupt (int sig)
{

} 


/* Free terminated thread and exits */
void mythread_exit ()
{
	int tid = mythread_gettid ();	
	printf ("*** THREAD %d FINISHED\n", tid);	
	t_state[tid].state = FREE;
	free (t_state[tid].run_env.uc_stack.ss_sp); 
	TCB* next = scheduler ();
	activator (next);
}

/* Sets the priority of the calling thread */
void mythread_setpriority (int priority)
{
	int tid = mythread_gettid ();	
	t_state[tid].priority = priority;
}

/* Returns the priority of the calling thread */
int mythread_getpriority (int priority)
{
	int tid = mythread_gettid ();
	return t_state[tid].priority;
}


/* Get the current thread id.*/
int mythread_gettid ()
{
	if (!init)
	{
		init_mythreadlib ();
		init=1;
	}
	return current;
}


/* Round Robin*/
TCB* scheduler ()
{
	/*Si el hilo que vamos a cambiar aún no ha terminado:*/
	if (running->state != FREE)
	{
		/*Volvemos a introducirlo la rodaja y lo metemos de nuevo a la cola*/
		running->ticks = QUANTUM_TICKS;
		disable_interrupt (); //Con disable_interrupt y enable_interrupt protegemos los accesos a la cola
		if (enqueue (thread_q , running )== NULL)
		{
			perror ("Fallo al introducir hilo de la cola");
			exit (-1);
		}
		enable_interrupt ();
	}
	/*Sacamos el siguiente hilo de la cola*/
	disable_interrupt ();
	TCB* next = dequeue (thread_q);
	enable_interrupt ();
	/*Si la cola no tiene más hilos terminamos el planificador*/
	if (next == NULL)
	{
		printf ("*** FINISH\n");
		exit (1);
	}
	return next;
}

/* Timer interrupt */
void timer_interrupt (int sig)
{
	/*A cada interrupción del reloj bajamos el número de ticks del hilo actual */
	running->ticks -= 1;
	/*Cuando termine una rodaja llamamos al planificador y activador para pasar al siguiente hilo que toque*/
	if (running->ticks == 0)
	{
		TCB* next = scheduler ();
		activator (next);
	}	
} 

/* Activator */
void activator (TCB* next)
{
/*Guardamos el hilo antiguo en la variable "last"*/
	TCB* last = running;
	running = next;
	/*Iniciamos el siguiente hilo actualizando running y current*/
	running->state = INIT;
	current = running->tid;
	/*Si el hilo anterior ha terminado, utilizaremos setcontext para el siguiente*/
	if (last->state == FREE)
	{
		printf ("*** THREAD %d TERMINATED : SETCONTEXT OF %d\n",last->tid,running->tid);
		setcontext (&running->run_env);
	}
	/*Si el hilo anterior no ha terminado, cambiamos el contexto mediante swapcontext*/
	else
	{
		printf ("*** SWAPCONTEXT FROM %d TO %d\n",last->tid,running->tid);
		swapcontext (&last->run_env,&running->run_env);
	}
}

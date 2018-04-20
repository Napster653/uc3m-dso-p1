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

/* Creación de colas */
struct queue *high_q; //Cola de alta prioridad
struct queue *low_q; //Cola de baja prioridad
struct queue *wait_q;

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
	/*inicializamos las colas*/
	high_q = queue_new ();
	low_q = queue_new ();
	wait_q = queue_new ();
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
	/*Añadimos el hilo a la cola correspondiente según su prioridad*/
	if(priority == HIGH_PRIORITY)
	{
		/*En el caso de que se este ejecutando un hilo de baja prioridad, el de alta debe sustituirle*/
		if(running->priority == LOW_PRIORITY)
		{
			TCB * last = running;
			running = t;
			running->state = INIT;
			current = t->tid;
			printf ("*** THREAD %d PREEMTED : SETCONTEXT OF %d\n",last->tid,running->tid);
			swapcontext (&last->run_env,&running->run_env);  
		}
		else
		{
			if(enqueue (high_q , t ) == NULL)
			{
				perror("Fallo al introducir hilo de la cola");
				exit(-1);
			}
		}
	}
	if(priority == LOW_PRIORITY)
	{
		if(enqueue (low_q , t ) == NULL)
		{
			perror("Fallo al introducir hilo de la cola");
			exit(-1);
		}
	}
	enable_interrupt();
	return i;
} /****** End my_thread_create() ******/

/* Read network syscall */
int read_network ()
{
  disable_interrupt(); 
 	printf ("*** THREAD %d READ FROM NETWORK\n",running->tid);
  running->state = WAITING;
  //if(running->priority == HIGH_PRIORITY) dequeue(high_q);
  //if(running->priority == LOW_PRIORITY) dequeue(high_q);
  if(enqueue (wait_q , running ) == NULL)
  {
    perror("Fallo al introducir hilo de la cola");
		exit(-1);
  }
  /*Si la cola de esperas es la única con algo lleno, pasaremos a ejecutar el idle thread*/
  if(queue_empty(low_q) == 1 && queue_empty(high_q) == 1 && current != -1)
  {
    TCB* last = running;
    running = &idle;
    current = idle.tid;
    swapcontext (&last->run_env,&running->run_env);
    return 1;
  }
  /*Procedemos a cambiar el contexto (voluntario)*/
  TCB* next = scheduler();
 	TCB* last = running;
	running = next;
	/*Iniciamos el siguiente hilo actualizando running y current*/
	running->state = INIT;
	current = running->tid;
  printf ("*** SWAPCONTEXT FROM %d TO %d\n",last->tid,running->tid);
	swapcontext (&last->run_env,&running->run_env);
 	enable_interrupt();
	return 1;
}

/* Network interrupt  */
void network_interrupt (int sig)
{
  /*Si la cola esta vacia descartamos el paquete:*/
  if(queue_empty(wait_q) == 1)
  {
    return;
  }
  /*Recibimos paquete, sacamos el hilo de la cola de espera y lo pasamos a la cola de listos, de alta prioridad o baja según corresponda*/
  TCB* ready = dequeue(wait_q);
  printf("*** THREAD %d READY\n", ready->tid);
  if(ready->priority == HIGH_PRIORITY)
  {
    if(enqueue (high_q , ready ) == NULL)
    {
      perror("Fallo al introducir hilo de la cola");
		  exit(-1);
    }
  }
  else
  {
    if(enqueue (low_q , ready ) == NULL)
    {
      perror("Fallo al introducir hilo de la cola");
		  exit(-1);
    }
  }

} 


/* Free terminated thread and exits */
void mythread_exit ()
{
	int tid = mythread_gettid ();	
	printf ("*** THREAD %d FINISHED\n", tid);	
	t_state[tid].state = FREE;
	free (t_state[tid].run_env.uc_stack.ss_sp); 
	/*Si la cola alta esta vacia y la pequeña también terminamos el planificador*/
	if(queue_empty(high_q) == 1 && queue_empty(low_q) == 1 && queue_empty(wait_q) == 1)
	{
		printf ("*** FINISH\n");
		exit(1);
	}
  if(queue_empty(low_q) == 1 && queue_empty(high_q) == 1 && current != -1)
  { 
    TCB* last = running;
    running = &idle;
    current = idle.tid;
    swapcontext (&last->run_env,&running->run_env);
    return;
  }
  disable_interrupt(); 
	TCB* next = scheduler ();
	activator (next);
	enable_interrupt();
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
	TCB* next;
	/*Si el proceso actual ha terminado o esta esperando:*/
	if(running->state == FREE || running->state == WAITING)
	{
		/*Si no hay hilos de prioridad alta, el actual a ejecutar será de prioridad baja*/
		if(queue_empty(high_q) == 1)
		{
			next = dequeue(low_q);
		}
		/*Si hay hilos de prioridad alta, sacaremos el siguiente de ahí*/
		else
		{
			next = dequeue(high_q);
		}
		return next;
	}

	/*En este caso nos encontramos en un hilo de prioridad baja que aún no ha terminado, así que restablecemos su rodaja y de vuelta a la cola*/
	running->ticks=QUANTUM_TICKS;
	if(enqueue (low_q , running )== NULL)
	{
		perror("Fallo al introducir hilo de la cola");
		exit(-1);
	}
	/*Sacamos el siguiente hilo de baja prioridad*/
	next = dequeue (low_q);
	return next;
}

/* Timer interrupt */
void timer_interrupt (int sig)
{
  /*Si se esta ejecutando el idle thread y hay hilos listos, los ponemos a ejecutar*/
  if(current == -1 && (queue_empty(high_q) != 1 || queue_empty(low_q) != 1))
  {
 	  disable_interrupt(); 
    TCB* next;
		if(queue_empty(high_q) != 1)
    {
      next = dequeue(high_q);
	    running = next;
      current = running->tid;
      printf("*** THREAD READY : SET CONTEXT TO %d\n",running->tid);
      setcontext (&running->run_env);
    }
    else
    {
      next = dequeue(low_q);
	    running = next;
      current = running->tid;
      printf("*** THREAD READY : SET CONTEXT TO %d\n",running->tid);
      setcontext (&running->run_env);
    }
    return;
		enable_interrupt();
  }
	/*Si el hilo es de alta prioridad no hacemos nada a cada paso del reloj*/
	if(running->priority == HIGH_PRIORITY || running == NULL || current == -1)
	{
		return;
	}
	/*Si el hilo es de baja prioridad, bajamos los ticks*/
	running->ticks -= 1;
	if(running->ticks == 0)
	{
		disable_interrupt(); 
		TCB* next = scheduler ();
		activator (next);
		enable_interrupt();
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
	if(last->state == FREE)
	{
		printf ("*** THREAD %d TERMINATED : SETCONTEXT OF %d\n",last->tid,running->tid);
		setcontext (&running->run_env);
	}
	/*Si el hilo anterior no ha terminado, cambiamos el contexto mediante swapcontext*/
	else
	{
		if(last->tid != running->tid)
		{
			printf ("*** SWAPCONTEXT FROM %d TO %d\n",last->tid,running->tid);
			swapcontext (&last->run_env,&running->run_env);
		}
	}
}

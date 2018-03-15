#include <stdio.h>
#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>
#include <ucontext.h>
#include <unistd.h>


#include <time.h>



#define TICK_TIME 5000
#define PACK_TIME 1
#define STARVATION 200

void timer_interrupt ();
void init_interrupt();
void disable_interrupt();
void enable_interrupt();

void network_interrupt ();
void init_network_interrupt();
void disable_network_interrupt();
void enable_network_interrupt();

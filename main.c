<<<<<<< HEAD
#include <stdio.h>
#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>
#include <ucontext.h>
#include <unistd.h>
#include "mythread.h"

void fun1(int global_index)
{
	int a=0, b=0;
	read_network();
	for(a=0; a<10; ++a)
	{
		// printf ("Thread %d with priority %d\t from fun2 a = %d\tb = %d\n", mythread_gettid(), mythread_getpriority(), a, b);
		for(b=0; b<25000000; ++b);
	}

	for(a=0; a<10; ++a)
	{
		// printf ("Thread %d with priority %d\t from fun2 a = %d\tb = %d\n", mythread_gettid(), mythread_getpriority(), a, b);
		for(b=0; b<25000000; ++b);
	}
	mythread_exit();
	return;
}

void fun2(int global_index)
{
	int a=0, b=0;
	read_network();
	for(a=0; a<10; ++a)
	{
		// printf ("Thread %d with priority %d\t from fun2 a = %d\tb = %d\n", mythread_gettid(), mythread_getpriority(), a, b);
		for(b=0; b<18000000; ++b);
	}
	for(a=0; a<10; ++a)
	{
		// printf ("Thread %d with priority %d\t from fun2 a = %d\tb = %d\n", mythread_gettid(), mythread_getpriority(), a, b);
		for(b=0; b<18000000; ++b);
	}
	mythread_exit();
	return;
}

void fun3(int global_index)
{
	int a=0, b=0;
	for(a=0; a<10; ++a)
	{
		// printf ("Thread %d with priority %d\t from fun2 a = %d\tb = %d\n", mythread_gettid(), mythread_getpriority(), a, b);
		for(b=0; b<40000000; ++b);
	}
	for(a=0; a<10; ++a)
	{
		// printf ("Thread %d with priority %d\t from fun2 a = %d\tb = %d\n", mythread_gettid(), mythread_getpriority(), a, b);
		for(b=0; b<40000000; ++b);
	}
	mythread_exit();
	return;
}



int main(int argc, char *argv[])
{
	int i,j,k,l,m,a,b=0;

	mythread_setpriority(HIGH_PRIORITY);

	read_network();
	if((i = mythread_create(fun1,LOW_PRIORITY)) == -1)
	{
		printf("thread failed to initialize\n");
		exit(-1);
	}
	read_network();
	if((j = mythread_create(fun2,LOW_PRIORITY)) == -1)
	{
		printf("thread failed to initialize\n");
		exit(-1);
	}
	if((k = mythread_create(fun3,LOW_PRIORITY)) == -1)
	{
		printf("thread failed to initialize\n");
		exit(-1);
	}
	if((l = mythread_create(fun1,HIGH_PRIORITY)) == -1)
	{
		printf("thread failed to initialize\n");
		exit(-1);
	}
	if((m = mythread_create(fun2,HIGH_PRIORITY)) == -1)
	{
		printf("thread failed to initialize\n");
		exit(-1);
	}

	for(a=0; a<10; ++a)
	{
		// printf ("Thread %d with priority %d\t from fun2 a = %d\tb = %d\n", mythread_gettid(), mythread_getpriority(), a, b);
		for(b=0; b<30000000; ++b);
	}

	if((a =  mythread_create(fun1,HIGH_PRIORITY)) == -1)
	{
		printf("thread failed to initialize\n");
		exit(-1);
	}
	if((b =  mythread_create(fun1,HIGH_PRIORITY)) == -1)
	{
		printf("thread failed to initialize\n");
		exit(-1);
	}
	mythread_exit();
	printf("This program should never come here\n");

	return 0;
}
=======
#include <stdio.h>
#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>
#include <ucontext.h>
#include <unistd.h>

#include "mythread.h"


void fun1 (int global_index)
{
  int a=0, b=0;
read_network();
  for (a=0; a<10; ++a) { 
//    printf ("Thread %d with priority %d\t from fun2 a = %d\tb = %d\n", mythread_gettid(), mythread_getpriority(), a, b);
    for (b=0; b<25000000; ++b);
  }

  for (a=0; a<10; ++a) { 
//    printf ("Thread %d with priority %d\t from fun2 a = %d\tb = %d\n", mythread_gettid(), mythread_getpriority(), a, b);
    for (b=0; b<25000000; ++b);
  }
  mythread_exit(); 
  return;
}


void fun2 (int global_index)
{
  int a=0, b=0;
  read_network();
  for (a=0; a<10; ++a) {
  //  printf ("Thread %d with priority %d\t from fun2 a = %d\tb = %d\n", mythread_gettid(), mythread_getpriority(), a, b);
    for (b=0; b<18000000; ++b);
  }
  for (a=0; a<10; ++a) {
  //  printf ("Thread %d with priority %d\t from fun2 a = %d\tb = %d\n", mythread_gettid(), mythread_getpriority(), a, b);
    for (b=0; b<18000000; ++b);
  }
  mythread_exit();
  return;
}

void fun3 (int global_index)
{
  int a=0, b=0;
  for (a=0; a<10; ++a) {
    //printf ("Thread %d with priority %d\t from fun2 a = %d\tb = %d\n", mythread_gettid(), mythread_getpriority(), a, b);
    for (b=0; b<40000000; ++b);
  }
  for (a=0; a<10; ++a) {
    //printf ("Thread %d with priority %d\t from fun2 a = %d\tb = %d\n", mythread_gettid(), mythread_getpriority(), a, b);
    for (b=0; b<40000000; ++b);
  }
  mythread_exit();
  return;
}



int main(int argc, char *argv[])
{
  int i,j,k,l,m,a,b=0;

  mythread_setpriority(HIGH_PRIORITY);
  read_network();
  if((i = mythread_create(fun1,LOW_PRIORITY)) == -1){
    printf("thread failed to initialize\n");
    exit(-1);
  }
 read_network();
  if((j = mythread_create(fun2,LOW_PRIORITY)) == -1){
    printf("thread failed to initialize\n");
    exit(-1);
  }
  if((k = mythread_create(fun3,LOW_PRIORITY)) == -1){
    printf("thread failed to initialize\n");
    exit(-1);
  }  
  if((l = mythread_create(fun1,HIGH_PRIORITY)) == -1){
    printf("thread failed to initialize\n");
    exit(-1);
  }

  if((m = mythread_create(fun2,HIGH_PRIORITY)) == -1){
    printf("thread failed to initialize\n");
    exit(-1);
  }
      
     
  for (a=0; a<10; ++a) {
  //    printf ("Thread %d with priority %d\t from fun2 a = %d\tb = %d\n", mythread_gettid(), mythread_getpriority(), a, b);
     for (b=0; b<30000000; ++b);
  }	
 
  if((a =  mythread_create(fun1,HIGH_PRIORITY)) == -1){
    printf("thread failed to initialize\n");
    exit(-1);
  }
  if((b =  mythread_create(fun1,HIGH_PRIORITY)) == -1){
    printf("thread failed to initialize\n");
    exit(-1);
  }
  mythread_exit();	
  
  printf("This program should never come here\n");
  
  return 0;
} /****** End main() ******/


>>>>>>> parent of 298597b... fixed

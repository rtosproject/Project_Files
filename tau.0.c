#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <time.h>
#include <signal.h>
#include <semaphore.h>

#define forever for(;1;)
#define zero( x ) memset( &x, 0, sizeof( x ) )

void handler( int signum ) {
  printf( "tick\n" ); // not technically safe in asynchronous func...
}

int main() {
  printf( "tau\n" );
  

  // register the handler
  struct sigaction sa;
    zero( sa );
    sa.sa_handler = handler; // void (*sa_handler)(int);
  sigaction( SIGINT, &sa, NULL );
  
  
  // create timer
  clockid_t cid = CLOCK_MONOTONIC;
  struct sigevent se;
    zero( se );
    se.sigev_notify = SIGEV_SIGNAL;
    se.sigev_signo = SIGINT;
  timer_t tid;
  if( timer_create( cid, &se, &tid ) ) {
    printf( "couldn't create timer\n" );
    return EXIT_FAILURE;
  }
  // & arm it
  struct itimerspec its;
    zero( its );
    its.it_value.tv_nsec = 2; // effectively immediate
    its.it_interval.tv_sec = 2; // every 2 seconds
  timer_settime( tid, 0, &its, NULL );
  
  
  // just busy wait to see if this works...
  forever;
  
  // create semaphore
  // call down on semaphore
  

  return 0; // never reached
}



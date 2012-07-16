#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <time.h>
#include <signal.h>
#include <semaphore.h>

#include <errno.h>

#define forever for(;1;)
#define zero( x ) memset( &x, 0, sizeof( x ) )

/*
  note: ctrl + z
  kills this process, where ctrl + c doesn't...
*/


sem_t sem;

void handler( int signum ) {
  /*
  int i;
  sem_getvalue( &sem, &i );
  printf( "handler-sem: %i\n", i ); // not technically safe in asynchronous func...
  */
  sem_post( &sem );
}

int main() {
  printf( "tau\n" );
  

  // register the handler
  struct sigaction sa;
    zero( sa );
    sa.sa_handler = handler; // void (*sa_handler)(int);
  sigaction( SIGINT, &sa, NULL );
  
  
  // create semaphore
  // call down on semaphore
  // sem_t sem;
  if( sem_init( &sem, 0, 0 ) ) {
    printf( "error initializing sem\n" );
    return EXIT_FAILURE;
  }
  /* sem_post( &sem ); // up
   * sem_wait( &sem ); // down
   * sem_destroy( &sem );
   */
  
  
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
    // its.it_interval.tv_sec = 2; // every 2 seconds
    its.it_interval.tv_nsec = 100000000;
  timer_settime( tid, 0, &its, NULL );
  
  
  // just busy wait to see if this works...
  // forever; // sit 'n spin
  
  
  int i = 0;
  forever {
    printf( "tick: %i\n", i++ );
    while( sem_wait( &sem ) ); // hack
  /*
    sem_getvalue( &sem, &i );
    printf( "tick-sem: %i\n", i );
    if( sem_wait( &sem ) ) {
      printf( "error waiting\n" );
      switch( errno ) {
        case EINTR:
          printf( "EINTR\n" );
          break;
        case EINVAL:
          printf( "EINVAL\n" );
          break;
        default:
          printf( "wtf!?...\n" );
      }
      return EXIT_FAILURE;
    }
    
    turns out the timer throwing an interrupt is causing the sem_wait to fail
    man 7 signal
      Interruption of System Calls and Library Functions by Stop Signals
      ( near the end )
      supposedly this is on linux 2.6.21 but mine is 2.6.38 ( uname -r )
    to fix this, I'll /hack/ just put it in a loop
      0 is returned on success
      -1 on error -> causes loop
  */
  }
  // better than "forever;" because it doesn't use the cpu
  // requires sem_post be called in handler
  //   and that sem be global to be accessed in handler
  // also allows handler to be small ( one call to sem_post )

  return 0; // never reached
}



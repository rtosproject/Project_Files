#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <time.h>
#include <signal.h>
#include <semaphore.h>
#include <math.h>

#include <errno.h>

#define forever for(;1;)
#define ZERO( x ) memset( &x, 0, sizeof( x ) )

/*
  note: ctrl + z
  kills this process, where ctrl + c doesn't...
*/


sem_t sem;

int timer_lt( itimer *ta, itimer *tb );

void handler( int signum ) {
  sem_post( &sem );
}

int main( int argc, char *argv[] ) {
  printf( "tau\n" );
  
  if( argc != 3 ) { // arbitrary number now, not sure how many args needed
    printf( "usage: tau <period(secs)> <cpu_util(secs)> \n" );
    return EXIT_FAILURE;
  }
  
  
  // register the handler
  struct sigaction sa;
    ZERO( sa );
    sa.sa_handler = handler; // void (*sa_handler)(int);
  sigaction( SIGINT, &sa, NULL );
  
  
  // create & initialize semaphore
  if( sem_init( &sem, 0, 0 ) ) {
    printf( "error initializing sem\n" );
    return EXIT_FAILURE;
  }
  
  
  // create timer
  clockid_t cid = CLOCK_MONOTONIC;
  struct sigevent se;
    ZERO( se );
    se.sigev_notify = SIGEV_SIGNAL;
    se.sigev_signo = SIGINT;
  timer_t tid;
  if( timer_create( cid, &se, &tid ) ) {
    printf( "couldn't create timer\n" );
    return EXIT_FAILURE;
  }
  // & arm it
  struct itimerspec its;
  float freq;
    ZERO( its );
    its.it_value.tv_nsec = 2; // effectively immediate
    sscanf( argv[ 1 ], "%f", &freq );
    its.it_interval.tv_sec = floor( freq );
    its.it_interval.tv_nsec = ( freq - its.it_interval.tv_sec ) * 100000000;
  timer_settime( tid, 0, &its, NULL );
  
  
  struct timespec ts_util;
  float cpu_load;
    sscanf( argv[ 2 ], "%f", &cpu_load );
    ts_util.tv_sec = floor( cpu_load );
    ts_util.tv_nsec = ( cpu_load - ts_util.tv_sec ) * 100000000;
  
  
  struct timespec ts_load;
    ZERO( ts_load );
  int i = 0;
  forever {
    sem_getvalue( &sem, &i );
    if( i ) {
      printf( "%s missed a deadline\n", argv[ 0 ] );
      if( i > 4 ) {
        printf( "processes snowballing;\n\t%s exiting\n", argv[ 0 ] );
        return EXIT_FAILURE;
      }
    }
    while( sem_wait( &sem ) ); // hack
  
    // use timer of cputime and busywait until constant ( arg: cpu utilization )
    while( timer_lt( &ts_load, &ts_util ) )
      clock_gettime( CLOCK_PROCESS_CPUTIME_ID, &ts_load );
    ZERO( ts_load );
    clock_settime( CLOCK_PROCESS_CPUTIME_ID, &ts_load );

    printf( "%s: tick\n", argv[ 0 ] );
  }
  
  
  return EXIT_SUCCESS; // never reached
}


int timer_lt( itimer *ta, itimer *tb ) {
  if( ta->tv_sec == tb->tv_sec ) {
    if( ta->tv_nsec < tb->tv_nsec )
      return true;
    else
      return false;
  }
  if( ta->tv_sec < tb->tv_sec )
    return true;
  else
    return false;
}








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

int main( int argc, char *argv[] ) {
  printf( "beta\n" );

  if( argc != 4 ) { // to change when adding priority, nice, algorithm, etc.
    printf( "usage: beta <led#> <freq(nanosecs)> <loadcount>\n" );
    return EXIT_FAILURE;
  }

  // register the handler
  struct sigaction sa;
    zero( sa );
    sa.sa_handler = handler; // void (*sa_handler)(int);
  sigaction( SIGINT, &sa, NULL );
  
  
  // sem_t sem;
  // sem_init( &sem, 0, 0 );
  if( sem_init( &sem, 0, 0 ) ) {
    printf( "error initializing sem\n" );
    return EXIT_FAILURE;
  }
  // sem_post( &sem ); // up
  // sem_wait( &sem ); // down
  // sem_getvalue( &sem, &int );
  // sem_destroy( &sem );
  
  
  // open led file
  char ss[ 0xff ];
  int state = 1; // led_state
  sprintf( ss, "/sys/class/leds/beaglebone::usr%s/brightness", argv[ 1 ] );
  FILE *led;
  led = fopen( ss, "w" );
  if( !led ) {
    printf( "couldn't open %s", ss );
    return EXIT_SUCCESS;
  }
  
  
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
    // its.it_interval.tv_sec = 1; // every second
    // its.it_interval.tv_nsec = 100000000;
    int freq = atoi( argv[ 2 ] );
    its.it_interval.tv_sec = floor( freq );
    its.it_interval.tv_nsec =  ( freq - its.it_interval.tv_sec ) * 100000000;
  timer_settime( tid, 0, &its, NULL );
  
  
  int load, i = 0;
  load = atoi( argv[ 3 ] );
  forever {
    sem_getvalue( &sem, &i );
    if( i > 1 ) {
      printf( "sem_value: %i\n", i );
      if( i > 16 ) {
        printf( "cpu overloaded: exiting\n" );
        return EXIT_SUCCESS;
      }
    }
    while( sem_wait( &sem ) ); // hack

    // printf( "tick, state:%i\n", state );
    for( i = 0; i < load; i++ ); // 'load' loop
    if( state )
      fwrite( "0", sizeof( char ), strlen( "0" ), led );
    else
      fwrite( "1", sizeof( char ), strlen( "1" ), led );
    fflush( led );
    state = state ^ 1;
  }
  
  
  // never reached
  fclose( led );
  return 0;
}



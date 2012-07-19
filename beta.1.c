#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <time.h>
#include <signal.h>
#include <semaphore.h>

#include <errno.h>

#define forever for(;1;)
#define ZERO( x ) memset( &x, 0, sizeof( x ) )

#define true 1
#define false ( !true )

/*
  note: ctrl + z
  kills this process, where ctrl + c doesn't...
*/


sem_t sem;

int timer_lt( struct timespec *, struct timespec * );
void timer_print( struct timespec * );

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
    printf( "usage: beta <led#> <freq(secs)> <cpu_util(sec)>\n" );
    return EXIT_FAILURE;
  }

  // register the handler
  struct sigaction sa;
    ZERO( sa );
    sa.sa_handler = handler; // void (*sa_handler)(int);
  sigaction( SIGINT, &sa, NULL );
  
  //   initialize the semaphor
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
    ZERO( se );
    se.sigev_notify = SIGEV_SIGNAL;
    se.sigev_signo = SIGINT;
  timer_t tid;
  if( timer_create( cid, &se, &tid ) ) {
    printf( "couldn't create timer\n" );
    return EXIT_FAILURE;
  }
  
  
  // initialize cpu bounds
  struct timespec ts_util, ts_util_mark; // utilization
  float cpu_load;
    sscanf( argv[ 3 ], "%f", &cpu_load );
    ts_util.tv_sec = floor( cpu_load );
    ts_util.tv_nsec = ( cpu_load - ts_util.tv_sec ) * 1000000000;
    ts_util_mark = ts_util;
  
  
  // arm timer
  struct itimerspec its;
  float freq;
    ZERO( its );
    its.it_value.tv_nsec = 2; // effectively immediate
    sscanf( argv[ 2 ], "%f", &freq );
    its.it_interval.tv_sec = floor( freq );
    its.it_interval.tv_nsec =  ( freq - its.it_interval.tv_sec ) * 1000000000;
  timer_settime( tid, 0, &its, NULL );
  
  
  struct timespec ts_load;
    ZERO( ts_load );
  int i, c = 0;
  char *sbit;
  
  
  forever {
  
    
    sem_getvalue( &sem, &i );
    if( i > 0 ) {
      printf( "%s sem: %i\n", argv[ 0 ], i );
      if( i > 16 ) {
        printf( "cpu overloaded: %s exiting\n", argv[ 0 ] );
        return EXIT_SUCCESS;
      }
    }
    
    
    while( sem_wait( &sem ) ); // hack


    while( timer_lt( &ts_load, &ts_util_mark ) ) {
      if( clock_gettime( CLOCK_PROCESS_CPUTIME_ID, &ts_load ) )
        perror( "clock_gettime() failed" );
      // timer_print( &ts_load );
    }
    c = 0;
    ZERO( ts_load );
    // if( clock_settime( CLOCK_PROCESS_CPUTIME_ID, &ts_load ) )
    //   perror( "clock_settime() failed" );
    ts_util_mark.tv_sec += ts_util.tv_sec;
    ts_util_mark.tv_nsec += ts_util.tv_nsec;
    if( ts_util_mark.tv_nsec >= 1000000000 ) {
      ts_util_mark.tv_nsec -= 1000000000;
      ts_util_mark.tv_sec++;
    }
    
    
    if( state )
      sbit = "0";
    else
      sbit = "1";
    fwrite( sbit, sizeof( *sbit ), strlen( sbit ), led );
    fflush( led );
    state = state ^ 1;
  }
  
  
  // never reached
  fclose( led );
  return 0;
}


int timer_lt( struct timespec *ta, struct timespec *tb ) {
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

void timer_print( struct timespec *ts ) {
  printf( "timer { sec:%i, nsec:%i }\n", ts->tv_sec, ts->tv_nsec );
}





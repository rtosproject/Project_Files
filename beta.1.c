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
  if( sem_init( &sem, 0, 0 ) )
    perror( "error initializing semaphore" );
  
  
  // open led file
  char ss[ 0xff ];
  int state = 1; // led_state
  sprintf( ss, "/sys/class/leds/beaglebone::usr%s/brightness", argv[ 1 ] );
  FILE *led;
  led = fopen( ss, "w" );
  if( !led )
    perror( "couldn't open led file" );
    // printf( "couldn't open %s", ss );
  
  
  // create timer
  clockid_t cid = CLOCK_MONOTONIC;
  struct sigevent se;
    ZERO( se );
    se.sigev_notify = SIGEV_SIGNAL;
    se.sigev_signo = SIGINT;
  timer_t tid;
  if( timer_create( cid, &se, &tid ) )
    perror( "couldn't create timer\n" );
  
  
  // initialize cpu bounds
  struct timespec ts_util, ts_util_mark; // cpu utilization
  float cpu_load;
    sscanf( argv[ 3 ], "%f", &cpu_load );
    ts_util.tv_sec = floor( cpu_load );
    ts_util.tv_nsec = ( cpu_load - ts_util.tv_sec ) * 1000000000;
    ts_util_mark = ts_util;
  struct timespec ts_load;
    ZERO( ts_load );
  
  
  // arm timer
  struct itimerspec its;
  float freq;
    ZERO( its );
    sscanf( argv[ 2 ], "%f", &freq );
    its.it_interval.tv_sec = floor( freq );
    its.it_interval.tv_nsec =  ( freq - its.it_interval.tv_sec ) * 1000000000;
    its.it_value = its.it_interval; // initial delay
  timer_settime( tid, 0, &its, NULL );
  
  
  int i;
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


    while( timer_lt( &ts_load, &ts_util_mark ) )
      if( clock_gettime( CLOCK_PROCESS_CPUTIME_ID, &ts_load ) ) // necessary
        perror( "clock_gettime() failed" );
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





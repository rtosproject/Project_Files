/*

  Brian Kirkpatrick
  7/30/12

  This scheduler is intended to work by creating a timer for each process to
  run. The timers throw interrupts. Registered handlers catch and handler the
  interrupts, signalling the main scheduler appropriately.

  The main scheduler should be notified whenever a process begins (timer) or
  ends (exit_function). This allows the scheduler to keep track of who should
  be running.

  semaphores are used to signal and maintain valid state with concurrency.

  real-time priorities are used to make the correct process run when each
  should.

  shared memory is used to communicate info to the scheduler.

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h> // getpagesize

#include "flist.h"

/*
  $( getconf PAGESIZE ) == 4096
  this is far more than necessary than what's needed to parse args
*/
#define PAGE sysconf( _SC_PAGESIZE )
// character buffer size for parsing/holding arguments
#define BUF ( 0x100 )
// limit on number of processes per group can run at once.
#define LIM 6
#define die( s ) { perror( s ); exit( 1 ); }

#define forever for(;1;)
#define ZERO( x ) memset( &x, 0, sizeof( x ) )


sem_t sched_sleep, mem_lock;

struct memmap {
  pid_t pid;
  int index;
};

struct proc_data {
  timer_t tid;
  struct itimerspec tau; // http://pubs.opengroup.org/onlinepubs/009696699/basedefs/time.h.html
  struct params_t { 
    char (*a)[ BUF ]; // pointer to arrays
    // char *a[ BUF ]; // array of pointers
    int num, cap;
  } params;
  flist ids;
};

void handler( int signum ) {
  sem_post( &sched_sleep );
}

int main() {

  // parse input file
  FILE *f = fopen( "scheduler_input", "r" );
  if( !f )
    die( "couldn't open scheduler_input" );
  int n; // number of process groups
  char buf[ BUF ];
  proc_data *procs;

  fscanf( f, " %i processes ", &n );
  procs = malloc( n * sizeof( struct args ) );
  
  for( c = 0; c < n; c++ ) {
    // alias
    params_t *p;
    p = &input->params;
      
    // initialize params
    p->num = 0;
    p->cap = 4;
    p->a = malloc( sizeof( *p->a ) * p->cap );
    
    // get period
    fgets( buf, BUF, f );
    set_timespec( buf, &procs[ c ].tau.it_interval );
    
    // parse command & args
    do {
      if( p->num >= p->cap ) { // extend params if needed
        p->cap *= 2;
        p->a = realloc( p->a, sizeof( *p->a ) * p->cap );
      }
      fgets( p->a[ p->num ], BUF, f );
      int blank_line;
      blank_line = !strcmp( p->a[ p->num ], "\n" );
      if( !blank_line )
        p->num++;
    } while( !blank_line && !foef( f ) );
    
    // append ending null to array for use as param to exec
    if( p->num >= p->cap ) { // extend params if needed
      p->cap++;
      p->a = realloc( p->a, sizeof( *p->a ) * p->cap );
    }
    p->a[ p->num ] = NULL;
    
    // initialize finite list for pids of this group
    flist_init( &procs[ c ].ids, LIM );
  }
  fclose( f );
  // ... forgot how much i hate parsing files ...
  // also, may want to confirm that everything was read in correctly...


  // set priority to realtime, high
  int policy = SCHED_FIFO; // SCHED_RR
  struct sched_param sparam;
    ZERO( sparam );
    param.sched_priority = 2; // low is high ?
  sched_setscheduler( 0 /* this process */, SCHED_FIFO, &param );


  // initialize 2 named semaphores
  sem_init( &sched_sleep, 0, 0 );
  sem_init( &mem_lock, 0, 1 );


  // open shared memory
  struct memmap *mem;
  f = tmpfile();
  ftruncate( f, sizeof( memmap ) );
  // void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
  mem = (memmap*) mmap( NULL, PAGE, PROT_READ | PROT_WRITE, MAP_SHARED, f, 0 );
  assert( mem );


  // register the handler
  struct sigaction sa;
    ZERO( sa );
    sa.sa_handler = handler; // void (*sa_handler)( int );
  sigaction( SIGALRM, &sa, NULL );
  
  
  // setup & arm timers 
  struct sigevent se;
    ZERO( se );
    se.sigev_notify = SIGEV_SIGNAL;
    se.sigev_signo = SIGALRM;
  struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 1; // immediate ( critical instant )
  for( c = 0; c < n; c++ ) {
    if( timer_create( CLOCK_MONOTONIC, &se, &procs[ c ].tid ) )
      die( "couldn't create timer\n" );
    procs[ c ].tau.it_value = ts;
    timer_settime( procs[ c ].tid, 0, &procs[ c ].tau, NULL );
      // for a running deadline
    clock_gettime( CLOCK_MONOTONIC, &procs[ c ].tau.it_value );
  }
  

  forever {

    while( sem_wait( &sched_sleep ) );

    // read mem, clear pid, realease semaphore
    pid_t fin;
    int index;
    fin = mem->pid;
    index = mem->index;
    mem->pid = 0;
    sem_post( &mem_lock );

    
    clock_gettime( CLOCK_MONOTONIC, &ts );
    if( fin ) { // process finished
      
      // notify world of process finishing
      printf( "{ command: '%s', pid: %i, event: '%s', sec: %i, nsec: %i }\n",
        procs[ index ]., fin, "finish", ts.tv_sec, ts.tv_nsec );
      
      // decrement count array
      procs[ c ].running--;
      
    }
    else { // else ( check processes to launch )
      
      // find index // better way to do this?
      for( c = 0; c < n && timer_lt( &ts, &procs[ c ].tau.it_value ); c++ );
        // while( now < deadline );
      assert( c < n );
      
      // fork, lower prio & exec, then pthread_create & waitpid
      pid_t cid = fork();
      if( cid ) { // parent
      
        // notify world of process beginning
        printf( "{ command: '%s', pid: %i, event: '%s', sec: %i, nsec: %i }\n",
          buf, fin, "begin", ts.tv_sec, ts.tv_nsec );
        
        // increment appropriate count & print error if needed
        if( procs[ c ].running++ > 1 ) {
          fprintf( stderr, "%s missed a deadline. %i copies now running.\n",
            procs[ c ].args[ 0 ], procs[ c ].running );
            
      }
      else { // child
        
        // set priority
        
        // execv[p]
        //   int execv(const char *path, char *const argv[]);
        execv( 
        assert( 0 );
      }
    
    }

  }

}


void set_timespec( char *t, struct timespec *ts ) {
  float tau;
  sscanf( t, "%f", &tau );
  ts->tv_sec = floor( tau );
  ts->tv_nsec = ( cpu_load - ts_util.tv_sec ) * 1000000000;
}

int timer_lt( struct timespec *ta, struct timespec *tb ) {
  if( ta->tv_sec == tb->tv_sec ) {
    if( ta->tv_nsec < tb->tv_nsec )
      return true;
    else
      return false;
  }
  else if( ta->tv_sec < tb->tv_sec )
    return true;
  else
    return false;
}



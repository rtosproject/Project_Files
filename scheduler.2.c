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

#define PAGE getpagesize()
#define BUF ( PAGE/2 )
#define die( s ) { perror( s ); exit( 1 ); }

sem_t sched_sleep, mem_lock;

struct memmap {
  pid_t pid;
  char command[ BUF ];
};

struct proc_data {
  char command[ BUF ];
  timer_t tid;
  struct itimerspec tau; // http://pubs.opengroup.org/onlinepubs/009696699/basedefs/time.h.html
  struct params_t { 
    char (*a)[ BUF ]; // pointer to arrays
    // char *a[ BUF ]; // array of pointers
    int num, cap; // of params
  } params;
  int running;
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
  proc_data *procs, *input;

  fscanf( f, " %i processes ", &n );
  procs = malloc( n * sizeof( struct args ) );
  // input = procs;
  
  for( c = 0; c < n; c++ ) {
    // alias for "input->params."
    params_t *p;
    p = &input->params;
      
    // initialize params
    p->num = 0;
    p->cap = 4;
    p->a = malloc( sizeof( *p->a ) * p->cap );
    
    // get period
    fgets( buf, BUF, f );
    set_timespec( buf, &procs[ c ].tau.it_interval );
    procs[ c ].tau.it_value = procs[ c ].tau.it_interval;
    
    // get command
    fgets( procs[ c ].command, BUF, f );
    
    // parse args
    do {
      if( p->num >= p->cap ) { // extend params if needed
        p->cap *= 2;
        p->str = realloc( p->str, sizeof( *p->a ) * p->cap * 2 );
      }
      fgets( p->a[ p->num ], BUF, f );
      int blank_line;
      blank_line = !strcmp( p->a[ p->num ], "\n" );
      if( !blank_line )
        p->num++;
    } while( !blank_line && !foef( f ) );
    
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


  // setup timers ... & arm timers ( later )
  struct sigevent se;
    ZERO( se );
    se.sigev_notify = SIGEV_SIGNAL;
    se.sigev_signo = SIGALRM;
  for( c = 0; c < n; c++ )
    if( timer_create( CLOCK_MONOTONIC, &se, &procs[ c ].tid ) )
      die( "couldn't create timer\n" );


  // register the handler
  struct sigaction sa;
    ZERO( sa );
    sa.sa_handler = handler; // void (*sa_handler)( int );
  sigaction( SIGALRM, &sa, NULL );


  // open shared memory
  struct memmap *mem;
  f = tmpfile();
  ftruncate( f, sizeof( memmap ) );
  // void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
  mem = (memmap*) mmap( NULL, PAGE, PROT_READ | PROT_WRITE, MAP_SHARED, f, 0 );
  assert( mem );


  // ... arm timers
  for( c = 0; c < n; c++ )
    timer_settime( procs[ c ].tid, 0, &procs[ c ].tau, NULL );
  

  forever {

    while( sem_wait( &sched_sleep ) );

    if( mem->pid ) // process finished
    {
      // clear pid, read name, realease semaphore
      pid_t fin;
      fin = mem->pid;
      strcpy( buf, mem->command );
      sem_post( &mem_lock );
      
      // notify world of process finishing
      printf( "pid %i finished at ?\n", fin );
        // need to add actual time here...

      
      // find index in names array
      // decrement count array
        // better way to do this?
      for( c = 0; strcmp( buf, procs[ c ].command ); c++ );
      procs[ c ].running--;

    }

    // else ( check processes to launch )
    {

      // realease semaphore

      // launch next process & re-insert
      // fork, lower prio & exec, then pthread_create & waitpid

      // set priority

      // increment appropriate count & print error if needed

      // output creation

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
  if( ta->tv_sec < tb->tv_sec )
    return true;
  else
    return false;
}



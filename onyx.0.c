#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <string.h>

#define ZERO( x ) memset( &x, 0, sizeof( x ) )

/*
  still need to add checks for return values of schedul-changing functions
  * i'm leery of this working...
 *
  also, there may be an error including the macros for the normal scheduling categories.
  * just comment those lines out if so.
*/

int main( int argc, char *argv[] ) {

	printf( "creating real-time process\n\n" );

	if( argc != 3 ) {
		printf( "usage: onyx <method> <priority>\n" );
		printf( "real-time categories:\n" );
		printf( "FIFO: %i\n\tmin prio: %i\n\tmax prio: %i\n",
				SCHED_FIFO, sched_get_priority_min( SCHED_FIFO ), sched_get_priority_max( SCHED_FIFO ) );
		printf( "RR: %i\n\tmin prio: %i\n\tmax prio: %i\n",
				SCHED_RR, sched_get_priority_min( SCHED_RR ), sched_get_priority_max( SCHED_RR ) );
/*		printf( "'normal' categories:\nSCHED_OTHER: %i, SCHED_IDLE: %i, SCHED_BATCH: %i\n",
				SCHED_OTHER, SCHED_IDLE, SCHED_BATCH );*/
				// priorities don't matter for normals ( I don't think... reference? )
		return EXIT_FAILURE;
	}

	int method, priority;
	method = atoi( argv[ 1 ] );
	priority = atoi( argv[ 2 ] );

	printf( "method: %i\n", method );
	printf( "priority: %i\n", priority );

	/* pid_t pid, int policy, const struct sched_param *param */

	struct sched_param param;
	ZERO( param ); // now a macro
	// memset( &param, 0, sizeof( param ) ); // clear to zero
	param.sched_priority = priority; // set priority
	sched_setscheduler( 0 /* this process */, method, &param );

	printf( "zxcvbnm,./\n" );
	return EXIT_SUCCESS;
}


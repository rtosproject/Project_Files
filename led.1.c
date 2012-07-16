#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
  this accepts one integer as input and lights the four user leds based on the low bits
  this is how qnx wrote to the leds
*/

int main( int argc, char *argv[] ) {
	if( argc != 2 ) {
		printf( " usage: <prog> <value(bits)>\n" );
		return EXIT_FAILURE;
	}

	int c, i = atoi( argv[ 1 ] );

	char *sp, sbuf[ 0xff ];

	for( c = 0; c < 4; c++ ) {
		sprintf( sbuf, "/sys/class/leds/beaglebone::usr%i/brightness", c );

		FILE *f = fopen( sbuf, "w" );
		if( !f ) {
			printf( "couldn't open %s\n", sbuf );
			// return EXIT_FAILURE;
			continue;
		}

		if( i & ( 1 << c ) )
			sp = "1";
		else
			sp = "0";

		fwrite( sp, sizeof( *sp ), strlen( sp ), f );

		fclose( f );
	}

	return EXIT_SUCCESS;
}



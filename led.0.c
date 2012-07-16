#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main( int argc, char *argv[] ) {
	if( argc != 3 ) {
		printf( " usage: <prog> <led#> <value>\n" );
		return EXIT_FAILURE;
	}

	char sbuf[ 0xff ];
	sprintf( sbuf, "/sys/class/leds/beaglebone::usr%s/brightness", argv[ 1 ] );
	FILE *f = fopen( sbuf, "w" );
	if( !f ) {
		printf( "couldn't open %s\n", sbuf );
		return EXIT_FAILURE;
	}

	fwrite( argv[ 2 ], sizeof( argv[ 2 ][0] ), strlen( argv[ 2 ] ), f );
	fclose( f );

	return EXIT_SUCCESS;
}



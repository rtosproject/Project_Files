


/*
  array.h
  Brian Kirkpatrick
*/

#ifndef b2k_array_h
#define b2k_array_h


/* included libraries */

#include <stdlib.h>


/* library parameters

#define array_DATA
typedef int* array_data;

#define array_DYNAMIC

*/


/* parameter defaults */

#ifndef array_DATA

#define array_DATA
typedef data_store array_data;

//#define array_DYNAMIC

#endif


/* library types */

typedef struct {
  array_data *a;
  int s; // size
  int c; // capacity
} array;


/* function declarations */

inline void array_init( array *a, int c );
inline void array_free( array *a );
inline void array_push( array *a, array_data data );
inline array_data array_pop( array *a );
inline int array_notempty( array *a );
inline int array_size( array *a );
inline int array_capacity( array *a );
inline int array_full( array *a );
inline void array_extend( array *a );


/* function instantiations */

inline void array_init( array *a, int c ) {
  a->a = (array_data*) malloc( c * sizeof( array_data ) );
  a->s = 0;
  a->c = c;
};


inline void array_free( array *a ) {
#ifdef array_DYNAMIC
  while( a->s > 0 )
    free( a->a[ --a->s ] );
#endif /* array_DYNAMIC */
  free( a->a );
};


inline void array_push( array *a, array_data data ) {
  if( a->s == a->c ) // full
    array_extend( a );
  a->a[ a->s++ ] = data;
};


inline array_data array_pop( array *a ) {
  return a->a[ --a->s ];
};


inline int array_notempty( array *a ) {
  return a->s;
};


inline int array_size( array *a ) {
  return a->s;
};


inline int array_capacity( array *a ) {
  return a->c;
};


inline int array_full( array *a ) {
  return a->s == a->c;
};


inline void array_extend( array *a ) {
  a->a = (array_data*) realloc( a->a, a->c * 2 * sizeof( array_data ) );
  a->c *= 2;
};


#endif /* array_h */

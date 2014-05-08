#ifndef PLOT_H
#define PLOT_H

#include <gsl/gsl_vector.h>
#include "circuit_hash.h"

/*
 * Call this before using the plot methods
 */
void plot_init();


/*
 * Return the size  of an array of vectors.
 * endv < startv
 */ 
int plot_find_size( double startv , double endv , double inc );

/*
 * returns an array of gsl_vectors.Starting values = 0.0
 * returns : NULL at failure
 */
gsl_vector** plot_create_vector( int array_size , int vector_size );

/*
 * writes an array of vectors to a file 
 */
void plot_to_file( hashtable_t* hashtable, gsl_vector** array , int array_size, char* filename);

/*
 * prints an array of vectors to a stderr 
 */
void plot_to_screen( hashtable_t* hashtable, gsl_vector** array , int array_size);

/*
 * Create files named after their representative node name and print the results
 */
void plot_by_node_name(hashtable_t* hashtable , gsl_vector** array , int array_size);

/*
 * Copies vector b at array[index]
 */
int plot_set_vector_index(gsl_vector** array , gsl_vector* b , int index);

/*
 * Deallocates all vectors
 */
void plot_free_array(gsl_vector** array , int array_size);

/*
 * Adds a node name for plotting
 * Returns 1 when ok
 *         0 when failed
 */
int plot_add_node(char* node_name);

/*
 * Deallocate all memory used by the plot module
 */
void plot_destroy();
#endif
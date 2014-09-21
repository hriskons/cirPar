#ifndef ITER_SOLVE_SPARSE_H
#define ITER_SOLVE_SPARSE_H
#include "linear_helper.h"

#include "sparse_interface.h"

/*
 * Set max iterations, error tolerance
 */
void iter_set_options( int iterations , double itol );

/*
 * Solve using the cg method
 * Returns NULL on error
 */
gsl_vector* sparse_solve_cg(sparse_matrix* A , gsl_vector* b , gsl_vector* x0);

gsl_vector* sparse_solve_bicg(sparse_matrix* A , gsl_vector* b , gsl_vector* x0);

void  safe_gsl_vector_calloc(gsl_vector** v, const size_t n);

#endif

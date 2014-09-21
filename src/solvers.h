/*
 * solvers.h
 *
 *  Created on: Sep 18, 2014
 *      Author: christos
 */

#ifndef SOLVERS_H_
#define SOLVERS_H_

#include "csparse.h"
#include "sparse_interface.h"
#include "helper_functions.h"
#include <math.h>
#include <string.h>
#include <gsl/gsl_vector_double.h>

#define ITER_NUM 15000
#define ITER_TOL 1e-5


void conjugate_gradient_sparse(cs *A, double *b, int n, double *x, double itol);

int sparse_solve_LU(sparse_matrix* matrix, sparse_vector* b, sparse_vector* x, int n);

void solve(sparse_matrix* matrix, sparse_vector* vector, gsl_vector* x,int vector_size);

#endif /* SOLVERS_H_ */

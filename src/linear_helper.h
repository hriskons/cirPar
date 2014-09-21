#ifndef LINEAR_HELPER_H
#define LINEAR_HELPER_H

#include "sparse_interface.h"

#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>

#define TRANSP 1
#define NON_TRANSP 2

double lh_norm( gsl_vector* v);

/*
 * Diagonal matrix-vector multiplication
 * Writes result to res
 */
void lh_diag_mul(gsl_vector* res, gsl_vector* v , gsl_vector* A);

/*
 * Returns a vector of the inversed diagonal elements of matrix m
 */
gsl_vector* lh_get_inv_diag(gsl_matrix* m);

/*
 * Returns a vector of the inversed diagonal elements of sparse matrix m
 */
gsl_vector* lh_get_inv_diag_sparse(sparse_matrix* A);

/*
	This function computes the matrix-vector product and sum y = \alpha op(A) x + \beta y,
	where op(A) = A, A^T, A^H for TransA = CblasNoTrans, CblasTrans, CblasConjTrans.
	We set 
		alpha = 1
		beta = 0
	so we have 
		y = op(A) * x
*/

void lh_matrix_vector_mul( gsl_vector* x, gsl_matrix* A,gsl_vector* y,int transp);

/* This is a custom made function that does the multiplication between a sparse array and a vector
 * 
 * @param A: This is the sparse array
 * @param x: This is the vector to be multiplied with the vector
 * @param y: This is the vector result
 * @param transp: This is a boolean variable that defines the nature of the sparse array and whether
 * 				  we need the normal or the transposed form
 * @return : The result will be in the y vector that will be given as a pointer
 */
void lh_matrix_vector_mul_sparse( gsl_vector* x, sparse_matrix* A,gsl_vector* y,int transp);

/*
	This function computes the matrix-vector product and sum y = \alpha op(A) x + \beta y,
	where op(A) = A, A^T, A^H for TransA = CblasNoTrans, CblasTrans, CblasConjTrans.

*/
void lh_matrix_vector_mul_and_sum( gsl_vector* x, gsl_matrix* A,gsl_vector* y,int transp,double alpha,double beta);

void lh_matrix_vector_mul_and_sum_sparse( gsl_vector* x, sparse_matrix* A,gsl_vector* y,int transp,double alpha,double beta);

/*
 * Vector - scalar multiplication
 */
void lh_scalar_vector_mul(gsl_vector* res, double s , gsl_vector* v);

/*
 * dot product
 */
double lh_dot_product(gsl_vector* v1 , gsl_vector* v2);

/*
* function that takes as parameter a gsl vector and returns a normal vector(pointer)
* the user has to free the allocated memory.
*/

void lh_gslVector_to_pointerVector(gsl_vector *vec,sparse_vector* v);

/*
* function that takes as parameter a pointer vector and returns the gsl vector
*/

void lh_pointerVector_to_gslVector(double *vec, gsl_vector *gslVec);
#endif

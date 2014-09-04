#ifndef CIRCUIT_SIM_SPARSE_H
#define CIRCUIT_SIM_SPARSE_H

#include "node_list.h"
#include "sparse_interface.h"

/*
 * creates a sparse mna matrix  and rhs b vector
 * Returns NULL when failed
 */
sparse_matrix* create_mna_sparse(LIST *list, sparse_vector** b, int* vector_len );

/*
 * dc_sweep for sparse matrices
 */
void dc_sweep_sparse();


/* S and N might need allocation*/
int sparse_LU_decomp(sparse_matrix* matrix, css* S, csn* N );

/**
 * Sparse LU solver
 *
 */
int sparse_solve_LU(sparse_matrix* matrix, sparse_vector* b, sparse_vector* x, int n);

/*S and N might need allocation*/
int sparse_cholesky_decomp(sparse_matrix** matrix, css* S, csn* N);

/**
 * Sparse cholesky solver
 *
 */
int sparse_solve_cholesky(sparse_matrix* matrix, sparse_vector* b, sparse_vector* x, int n);

/**
 * Perform dc sweep using LU solver
 *
 * Returns : 1 on success
 *			 0 on failure
 */
int sparse_dc_sweep(LIST *list , sparse_matrix* matrix , sparse_vector* rhs);


void add_resistance_element(sparse_matrix* matrix, sparse_vector* vector, LIST_NODE* curr,int matrix_row);

void add_current_source(sparse_matrix* matrix, sparse_vector* vector, LIST_NODE* curr,int matrix_row);

void add_voltage_source(sparse_matrix* matrix, sparse_vector* vector, LIST_NODE* curr,int matrix_row);

void add_inductunce_element(sparse_matrix* matrix, sparse_vector* vector, LIST_NODE* curr,int matrix_row);


#endif

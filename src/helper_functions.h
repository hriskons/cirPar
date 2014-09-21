/*
 * helper_functions.h
 *
 *  Created on: Sep 18, 2014
 *      Author: christos
 */

#ifndef HELPER_FUNCTIONS_H_
#define HELPER_FUNCTIONS_H_

#include <gsl/gsl_blas.h>
#include "sparse_interface.h"
#include "graph_metis.h"

#define TRUE 	1
#define FALSE 	0
#define DEBUG 1
#define debug_print(fmt, ...) \
            do { if (DEBUG) fprintf(stderr, fmt, __VA_ARGS__); } while (0)


typedef struct {
	int** ptr;
	sparse_vector* value;
	sparse_matrix* graph;
	int size;
	int id;

}cluster_t;

void print_gsl_vector_to_file(gsl_vector* vector,const char* filename);

void print_vector_to_file(sparse_vector* vector,int vector_size,const char* filename);

void sparse_to_gsl_vector(double *doubl_vec,gsl_vector* gsl_v,int size_n);

void* safe_malloc(const int size);

double cs_find(sparse_matrix* A,int column,int row);

int* count_elements_in_each_cluster(partition_t par_table);

int get_max(int* vec,int size);



#endif /* HELPER_FUNCTIONS_H_ */

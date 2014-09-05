#ifndef WRAPED_FUNCTIONS_H
#define WRAPED_FUNCTIONS_H

#include "metis.h"
#include "struct.h"
#include "graph_metis.h"
#include "sparse_interface.h"

idx_t* imalloc(int noOfElements, char * msg);

real_t* rmalloc(int noOfElements, char * msg);

void* bmalloc(int bytes, char * msg);

idx_t* irealloc(idx_t* oldMemory,int noOfElementsNew);

void print_vector(idx_t *vector, int length);

void print_input_data(graph_t *graph, params_t *params);

int csEntry(sparse_matrix *graph_matrix, int plus_node, int minus_node, double x);

#endif

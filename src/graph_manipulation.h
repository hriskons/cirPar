/*
 * graph_manipulation.h
 *
 *  Created on: Sep 18, 2014
 *      Author: christos
 */

#ifndef GRAPH_MANIPULATION_H_
#define GRAPH_MANIPULATION_H_

#include "sparse_interface.h"
#include "graph_metis.h"
#include "helper_functions.h"
#include "csparse.h"

sparse_matrix* graph_t_to_sparse_matrix(graph_t* graph);

void set_edge_weights(sparse_matrix* original_matrix,sparse_matrix* output_matrix);

int check_correctness(sparse_matrix* original_matrix,sparse_matrix* output_matrix);

void set_weights_between_clusters(sparse_matrix* matrix,partition_t par_graph);

void set_weights_between_clusters(sparse_matrix* matrix,partition_t par_graph);


#endif /* GRAPH_MANIPULATION_H_ */

/*
 * graph_manipulation.c
 *
 *  Created on: Sep 18, 2014
 *      Author: christos
 */
#include "graph_manipulation.h"

sparse_matrix* graph_t_to_sparse_matrix(graph_t* graph)
{
	sparse_matrix* matrix;
	matrix = (sparse_matrix*)cs_malloc(1,sizeof(sparse_matrix));


	return matrix;
}


int count_num_of_edges(sparse_matrix* A,int nodeId)
{
	return A->p[nodeId + 1] - A->p[nodeId];
}

/* It checks if there is a node on the place (column,row) */
int cs_get_node_id(sparse_matrix* A,int column,int row)
{
	int p;

	for(p = A->p[column]; p < A->p[column + 1]; p++)
	{
		if(row == A->i[p])
			return p;
	}
	return -1;
}

int change_diagonal(sparse_matrix* A,int col,double value){

	int nodeId;

	if((nodeId = cs_get_node_id(A,col,col)) != -1){

		A->x[nodeId] -= value;
		return TRUE;
	}

	return FALSE;

}

void split_the_weight(sparse_matrix* matrix,int col,int row)
{
	int p,j;

	double value_to_split = matrix->x[row];
	int return_value;
	int counter = 0;
	/* The first thing that we have to do is remove the weight from the main diagonal
	 * of the edge that is being removed. This amount will be added on other places
	 * of the diagonal where the weight will be transfered. Of course this applies
	 * only on the matrix that has a diagonal that consists of the sum of each row in
	 * its place.
	 */
	return_value = change_diagonal(matrix,col,value_to_split);

	value_to_split /= count_num_of_edges(matrix,col);


	/* This is the second stage where we increase the value of each edge that is connected
	 * with the edge that we removed.
	 */
	for(p = matrix->p[col]; p < matrix->p[col + 1]; p++){
		matrix->x[p] += value_to_split;
		if(return_value)
			change_diagonal(matrix,matrix->i[p],value_to_split);
	}

	/* This is the symmetric opperation on what we did above. Because the graph
	 * consists from double edges when is comes to represent a connection between
	 * two different nodes, we have to add the subtracted weight twice.
	 */
	for(j = 0; j < matrix->n; j++){
		for(p = matrix->p[j]; p < matrix->p[j + 1]; p++){
			if(col == matrix->i[p]){
				matrix->x[p] -= value_to_split;
				counter++;
				if(return_value)
					change_diagonal(matrix,matrix->i[p],value_to_split);
			}
		}
	}

	assert(matrix->p[col + 1] - matrix->p[col] == counter);

}

void set_weights_between_clusters(sparse_matrix* matrix,partition_t par_graph)
{
	int numOfCols;
	int* colPtr;
	int* rowIndc;
	int j,p;
	double *values,value_to_split;

	numOfCols = matrix->n;
	colPtr = matrix->p;
	rowIndc = matrix->i;
	values = matrix->x;

	for (j = 0; j < numOfCols; j++) {
		for (p = colPtr[j]; p < colPtr[j + 1]; p++) {
			if( j < rowIndc[p] && 													/* 	Because between two different vertices there are two edges,
																						we want to make sure that we divide the edge cost only once
																						and on the first time that we reach this edge */
			    par_graph.partion_table[j] != par_graph.partion_table[matrix->i[p]]) /* 	This check makes sure that we only divide the weight of
			   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	   		edges that are between different clusters */
			{
				value_to_split = values[p]; 										/* This is the value of the edge that connects the two clusters */

				/* For the solution that we have designed what we have to do is the following:
				 * 		We have to find all the edges that are connected on the two nodes that the initial edge was connected to.
				 * 		As soon as we find the edges we are going to split among them the weight of the edge that we are removing
				 */
				//debug_print("%d edges found from the node %d\n",count_num_of_edges(matrix,j),j);

				value_to_split /= count_num_of_edges(matrix,j);

				split_the_weight(matrix,j,p);
			}

				//printf("%d with %d -> %d and %d with edge weight %f\n",j,rowIndc[p],par_graph.partion_table[j],par_graph.partion_table[matrix->i[p]],values[p]);

			//	printf("%d with %d -> %d and %d with edge weight %f\n",rowIndc[p],colPtr[j],par_graph.partion_table[p],par_graph.partion_table[colPtr[j]],values[p]);

		}
	}
	printf("We are done with the modification of the mna matrix after the partitioning\n");
}


int check_correctness(sparse_matrix* original_matrix,sparse_matrix* output_matrix)
{
	int j,p;
	double value;
	for (j = 0; j < 10; j++) {
		for (p = original_matrix->p[j]; p < original_matrix->p[j + 1]; p++) {
			value = cs_find(output_matrix,j,original_matrix->i[p]);
			printf("%f %f\n",value,original_matrix->x[p]);

			if(!(abs(value - original_matrix->x[p]) < 1)){
				return FALSE;
			}
		}
	}
	return TRUE;

}

void set_edge_weights(sparse_matrix* original_matrix,sparse_matrix* output_matrix)
{
	/* Count how many non-zero elements exist in the array excluding the elements of the
	 * diagonal that represent the sum of each row
	 */
	int counter = 0;
	int j,p;

	for (j = 0; j < original_matrix->n; j++) {
		for (p = original_matrix->p[j]; p < original_matrix->p[j + 1]; p++) {
			if (j != original_matrix->i[p]) {
				counter++;
			}
		}
	}
	/* At this point we have the number of non zero elements without the ones that are
	 * in the main diagonal of the sparse array.
	 */


	output_matrix->x = (double *)malloc(counter * sizeof(double));
	if(output_matrix->x == NULL){
		perror("Allocation of double array of the sparse matrix failed\n");
		exit(0);
	}

	/* we are going to copy the non-zero elements of the original matrix on a new one
	 * that represents the graph in order to be able to change the weights of some of
	 * the edges.
	 */
	int r = 0;
	for (j = 0; j < original_matrix->n; j++) {
		for (p = original_matrix->p[j]; p < original_matrix->p[j + 1]; p++) {
			if (j != original_matrix->i[p]) {
				output_matrix->x[r] = original_matrix->x[p];
				r++;
			}
		}
	}
	assert(r == counter);

}





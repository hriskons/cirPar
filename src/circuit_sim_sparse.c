#include <stdlib.h>
#include <string.h>
#include "circuit_sim_sparse.h"
#include "plot.h"

#define DEFAULT_NZ 15

sparse_matrix* create_mna_sparse(LIST *list, sparse_vector** b, int* vector_len){

	int i;
	int rows;
	int columns;
	sparse_vector* vector = NULL;
	sparse_matrix* matrix = NULL;
	LIST_NODE* curr;

	int num_nodes = ht_get_num_nodes(list->hashtable);
	int* nodeids = (int*)malloc(sizeof(int) * num_nodes);
	if(!nodeids)
		return NULL;

	for( i = 0; i < num_nodes; i++)
		nodeids[i] = 0;

	int m2_elements_found = 0;       // # of elements in group 2

	/* allocate matrix and vector */
	rows    = list->hashtable->num_nodes + list->m2;
 	columns = list->hashtable->num_nodes + list->m2;

 	matrix =  cs_spalloc( rows , columns , DEFAULT_NZ , 1 , 1 );
 	if(!matrix)
 		return NULL;

 	vector = (sparse_vector*) malloc( sizeof(sparse_vector) * rows);
 	if( !vector){
 		cs_spfree(matrix);
 		return NULL;
 	} 		

 	for( curr = list->head ; curr; curr = curr->next){

 		/*
 		 * RESISTANCE ELEMENT
 		 */

 		if( curr->type == NODE_RESISTANCE_TYPE ){

  			double conductance = 1 / curr->node.resistance.value ;
 			long plus_node  = curr->node.resistance.node1 - 1;
 			long minus_node = curr->node.resistance.node2  - 1;
 			//printf("plus_node: %d minus_node: %d\n",plus_node,minus_node);
 			/* <+> is ground */
 		 	if( plus_node == -1 ){

 				//double value = gsl_matrix_get(tmp_matrix , minus_node , minus_node);
 				//value += conductance ; 
 				//gsl_matrix_set( tmp_matrix , minus_node , minus_node ,  value );
 				//printf("Adding to matrix element (%d,%d) value:%f\n\n",minus_node,minus_node,value);
 				if( !cs_entry(matrix, minus_node , minus_node , conductance) ){
 					fprintf(stderr, "Error while inserting element in sparse matrix\n");
 					free(vector);
 					cs_spfree(matrix);
 					return NULL;
 				}
 			}
 			/* <-> is ground */
 			else if ( minus_node == -1  ){

 				if( !cs_entry(matrix, plus_node , plus_node , conductance) ){
 					fprintf(stderr, "Error while inserting element in sparse matrix\n");
 					free(vector);
 					cs_spfree(matrix);
 					return NULL;
 				}
 			}
 			else {

 				/* <+> <+> */
 				if( !cs_entry(matrix, plus_node , plus_node , conductance) ){
 					fprintf(stderr, "Error while inserting element in sparse matrix\n");
 					free(vector);
 					cs_spfree(matrix);
 					return NULL;
 				}

 				/* <+> <-> */
 				if( !cs_entry(matrix, plus_node , minus_node , -conductance) ){
 					fprintf(stderr, "Error while inserting element in sparse matrix\n");
 					free(vector);
 					cs_spfree(matrix);
 					return NULL;
 				}

 				/* <-> <+> */
 				if( !cs_entry(matrix, minus_node , plus_node , -conductance) ){
 					fprintf(stderr, "Error while inserting element in sparse matrix\n");
 					free(vector);
 					cs_spfree(matrix);
 					return NULL;
 				}

 				/* <-> <-> */
 				if( !cs_entry(matrix, minus_node , minus_node , conductance) ){
 					fprintf(stderr, "Error while inserting element in sparse matrix\n");
 					free(vector);
 					cs_spfree(matrix);
 					return NULL;
 				}
 			}
		}
		/* 
 		 * CURRENT SOURCE
 		 */
 		else if( curr->type == NODE_SOURCE_I_TYPE ){

 			/* change only the vector */
 			double current = curr->node.source_i.value;
 			double value;

 			if( curr->node.source_i.node1 != 0 ){
 				/* ste <+> */
 				value  = vector[curr->node.source_i.node1 - 1 ];
 				value -= current;
 				vector[curr->node.source_i.node1 -1 ] =  value;
 			}

 			if( curr->node.source_i.node2 != 0 ){
 				/* <-> */
 				value  = vector[curr->node.source_i.node2 - 1 ];
 				value += current;
 				vector[curr->node.source_i.node2 -1 ] =  value;
 			}
 		}
 	 	/*
 		 * VOLTAGE SOURCE
 		 */
 		else if ( curr->type == NODE_SOURCE_V_TYPE  ){
 			m2_elements_found++;
 			int matrix_row = list->hashtable->num_nodes + m2_elements_found - 1;
 			curr->node.source_v.mna_row = matrix_row;

 			double value;
 			
 			/* set vector value */
 			value = vector[matrix_row];
 			value += curr->node.source_v.value;
 			vector[ matrix_row ] = value;

 			long plus_node  = curr->node.source_v.node1 - 1;
 			long minus_node = curr->node.source_v.node2 - 1;

 			//printf("Voltage %s plus = %d minus = %d matrix_row = %d\n", curr->node.source_v.name,plus_node,minus_node,matrix_row );

 			/* <+> */
 			if( plus_node != -1 ){
 				//if( nodeids[plus_node] == 0 ){
 					//printf("mphke plus node...\n");
 					if( !cs_entry(matrix, matrix_row , plus_node , 1.0 ) ){
 		 				fprintf(stderr, "Error while inserting element in sparse matrix\n");
 						free(vector);
 						cs_spfree(matrix);
 						return NULL;	
 					}
 					
 					if( !cs_entry(matrix, plus_node , matrix_row , 1.0 ) ){
 		 				fprintf(stderr, "Error while inserting element in sparse matrix\n");
 						free(vector);
 						cs_spfree(matrix);
 						return NULL;	
 					}
 					//nodeids[plus_node] = 1;
 				//}			
 			}

 			/* <-> */
 			if( minus_node != -1 ){
 				
 				//if( nodeids[minus_node] == 0 ){
 					//printf("mphke minus node...\n");
 					if( !cs_entry(matrix, matrix_row , minus_node , -1.0 ) ){
 		 				fprintf(stderr, "Error while inserting element in sparse matrix\n");
 						free(vector);
 						cs_spfree(matrix);
 						return NULL;	
 					}
 					if( !cs_entry(matrix, minus_node , matrix_row , -1.0 ) ){
 		 				fprintf(stderr, "Error while inserting element in sparse matrix\n");
 						free(vector);
 						cs_spfree(matrix);
 						return NULL;	
 					}
 					//nodeids[minus_node] = 1;
 				
 				//}
 			}	 
 		}

 		/*
 		 * Inductance
 		 */
 		else if ( curr->type == NODE_INDUCTANCE_TYPE  ){
 			m2_elements_found++;
 			int matrix_row = list->hashtable->num_nodes  + m2_elements_found - 1 ;
			

 			/* Change the matrix */
 			long plus_node  = curr->node.inductance.node1 - 1;
 			long minus_node = curr->node.inductance.node2 - 1;
 			// printf("Inductance %s plus = %d minus = %d\n matrix_row = %d\n", curr->node.inductance.name,plus_node,minus_node,matrix_row);

 			/* <+> */
 			if( plus_node != -1 ){
 				//if( nodeids[plus_node] == 0 ){
 					//printf("mphke inductance sto plus...\n");
 					if( !cs_entry(matrix, matrix_row , plus_node , 1.0) ){
 		 				fprintf(stderr, "Error while inserting element in sparse matrix\n");
 						free(vector);
 						cs_spfree(matrix);
 						return NULL;	
 					}

 					if( !cs_entry(matrix, plus_node , matrix_row , 1.0) ){
 		 				fprintf(stderr, "Error while inserting element in sparse matrix\n");
 						free(vector);
 						cs_spfree(matrix);
 						return NULL;	
 					}
 					//nodeids[plus_node] = 1;
 				//} 				
 			}
 			/* <-> */
 			if( minus_node != -1 ){
 				//if( nodeids[minus_node] == 0 ){
 					//printf("mpahke Inductance minus_node...\n");
 					if( !cs_entry(matrix, matrix_row , minus_node , -1.0 ) ){
 		 				fprintf(stderr, "Error while inserting element in sparse matrix\n");
 						free(vector);
 						cs_spfree(matrix);
 						return NULL;	
 					}
 					if( !cs_entry(matrix, minus_node , matrix_row , -1.0 ) ){
 		 				fprintf(stderr, "Error while inserting element in sparse matrix\n");
 						free(vector);
 						cs_spfree(matrix);
 						return NULL;	
 					}
 					//nodeids[minus_node] = 1;
 				//}
 			} 			
 		}
 	} 	

	matrix = cs_compress(matrix);
 	
 	/* remove duplicates */
 	if( !cs_dupl(matrix) ){
 		fprintf(stderr, "Sparse matrix: duplicates not removed \n");
 		cs_spfree(matrix);
 		free(vector);
 		return NULL;
 	}
	*vector_len = matrix->n;
 	*b = vector;

 	//cs_print(matrix,"sparse_matrix.txt",0);
 	return matrix;
}

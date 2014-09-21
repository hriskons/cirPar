#include <stdlib.h>
#include <string.h>
#include "circuit_sim_sparse.h"
#include "plot.h"
#include "wraped_functions.h"


#define DEBUG 0
#define debug_print(fmt, ...) \
            do { if (DEBUG) fprintf(stderr, fmt, __VA_ARGS__); } while (0)

sparse_matrix* create_mna_sparse(LIST *list, sparse_vector** b, int* vector_len){

	int i;
	int rows;
	int columns;
	sparse_vector* vector = NULL;
	sparse_matrix* matrix = NULL;
	LIST_NODE* curr;

	sparse_matrix* temp;

	int num_nodes = ht_get_num_nodes(list->hashtable);
	int* nodeids = (int*)malloc(sizeof(int) * num_nodes);
	if(!nodeids)
		return NULL;

	for( i = 0; i < num_nodes; i++)
		nodeids[i] = 0;

	int m2_elements_found = 0;       // # of elements in group 2

	/* allocate matrix and vector */
	rows    = list->hashtable->num_nodes + list->m2 - 1;
 	columns = list->hashtable->num_nodes + list->m2 - 1;

 	printf("non zero elements: %d\n",list->non_zero_elements);
 	matrix =  cs_spalloc( rows , columns , list->non_zero_elements , 1 , 1 );
 	if(!matrix)
 		return NULL;

 	vector = (sparse_vector*) malloc( sizeof(sparse_vector) * rows);
 	if( !vector){
 		cs_spfree(matrix);
 		return NULL;
 	} 		

 	for( curr = list->head ; curr; curr = curr->next){

 		switch (curr->type){
 			case NODE_RESISTANCE_TYPE:  { 	add_resistance_element(matrix , NULL, curr , -1); break; 	}  			/* RESISTANCE ELEMENT	 */
 			case NODE_SOURCE_I_TYPE: 	{ 	add_current_source(matrix, vector, curr, -1); break;		}			/* CURRENT SOURCE  		 */
 			case NODE_SOURCE_V_TYPE:	{
 	 										int matrix_row = list->hashtable->num_nodes + m2_elements_found - 1;
 	 										add_voltage_source( matrix,  vector,  curr, matrix_row );
 	 										m2_elements_found++;
 	 										break;
 	 									}																			/* VOLTAGE SOURCE		 */
 			case NODE_INDUCTANCE_TYPE:  {
											int matrix_row = list->hashtable->num_nodes  + m2_elements_found - 1;
											add_inductunce_element( matrix, vector, curr, matrix_row);
											m2_elements_found++;
											break;
 	 									}																			/* INDUCTUNCE			 */
 		}
 	} 	

	temp = cs_compress(matrix);
	cs_spfree(matrix);
	matrix = temp;
 	cs_print(matrix,"output_matrix_with_duplicates",0);
 	/* remove duplicates */
 	if( !cs_dupl(matrix) ){
 		fprintf(stderr, "Sparse matrix: duplicates not removed \n");
 		cs_spfree(matrix);
 		free(vector);
 		return NULL;
 	}
	*vector_len = rows;
 	*b = vector;

 	//cs_print(matrix,"sparse_matrix.txt",0);
 	return matrix;
}


void add_resistance_element(sparse_matrix* matrix, sparse_vector* vector, LIST_NODE* curr,int matrix_row)
{
	double conductance = 1 / curr->node.resistance.value ;
	long plus_node  = curr->node.resistance.node1 - 1;
	long minus_node = curr->node.resistance.node2 - 1;
	debug_print("plus_node: %lu minus_node: %lu\n",plus_node,minus_node);

	/* <+> is ground */
	if( plus_node == -1 ){
		csEntry(matrix, minus_node , minus_node , conductance);
	}
	/* <-> is ground */
	else if ( minus_node == -1  ){
		csEntry(matrix, plus_node , plus_node , conductance);
	}
	else {
		/* <+> <+> */
		csEntry(matrix, plus_node , plus_node , conductance);
		/* <-> <-> */
		csEntry(matrix, minus_node , minus_node , conductance);
		/* <+> <-> */
		csEntry(matrix, plus_node , minus_node , -conductance);
		/* <-> <+> */
		csEntry(matrix, minus_node , plus_node , -conductance);

			}
}

void add_current_source(sparse_matrix* matrix, sparse_vector* vector, LIST_NODE* curr,int matrix_row)
{

	/* change only the vector */
	double current = curr->node.source_i.value;
	double value;

	if( curr->node.source_i.node1 != 0 ){
		/* ste <+> */
		value  = vector[curr->node.source_i.node1 - 1 ];
		value -= current;
		vector[curr->node.source_i.node1 -1] =  value;
	}

	if( curr->node.source_i.node2 != 0 ){
		/* <-> */
		value  = vector[curr->node.source_i.node2 - 1 ];
		value += current;
		vector[curr->node.source_i.node2 -1] =  value;
	}
}

void add_voltage_source(sparse_matrix* matrix, sparse_vector* vector, LIST_NODE* curr,int matrix_row)
{
	curr->node.source_v.mna_row = matrix_row;

	double value;

	/* set vector value */
	value = vector[matrix_row];
	value += curr->node.source_v.value;
	vector[ matrix_row ] = value;

	long plus_node  = curr->node.source_v.node1 - 1;
	long minus_node = curr->node.source_v.node2 - 1;

	debug_print("Voltage %s plus = %d minus = %d matrix_row = %d\n", curr->node.source_v.name,plus_node,minus_node,matrix_row );

	/* <+> */
	if( plus_node != -1 ){
		csEntry(matrix, matrix_row , plus_node , 1.0 );
		csEntry(matrix, plus_node , matrix_row , 1.0 );
	}

	/* <-> */
	if( minus_node != -1 ){
		csEntry(matrix, matrix_row , minus_node , -1.0 );
		csEntry(matrix, minus_node , matrix_row , -1.0 );
	}

}

void add_inductunce_element(sparse_matrix* matrix, sparse_vector* vector, LIST_NODE* curr,int matrix_row)
{

	/* Change the matrix */
	long plus_node  = curr->node.inductance.node1 - 1;
	long minus_node = curr->node.inductance.node2 - 1;
	debug_print("Inductance %s plus = %d minus = %d\n matrix_row = %d\n", curr->node.inductance.name,plus_node,minus_node,matrix_row);

	/* <+> */
	if( plus_node != -1 ){
		csEntry(matrix, matrix_row , plus_node , 1.0);
		csEntry(matrix, plus_node , matrix_row , 1.0);
	}
	/* <-> */
	if( minus_node != -1 ){
		csEntry(matrix, matrix_row , minus_node , -1.0 );
		csEntry(matrix, minus_node , matrix_row , -1.0 );
	}
}


#include <stdlib.h>
#include <string.h>
#include "circuit_sim_sparse.h"
#include "plot.h"
#include "iter_solve_sparse.h"
#include "linear_helper.h"

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


int sparse_LU_decomp(sparse_matrix* matrix, css* S, csn* N ){

	if( !matrix)
		return 0;

	S=cs_sqr(2,matrix,0);
	N=cs_lu(matrix,S,1);
	
	if(!S || !N)
		return 0;

	cs_spfree(matrix);

	return 1;
}

int sparse_solve_LU(sparse_matrix* matrix, sparse_vector* b, sparse_vector* x, int n){

	if( n < 1 )
		return 0;

	memcpy(x,b ,n * sizeof(double));

	return cs_lusol(2 , matrix , x , 1.0 );

}

int sparse_solve_cholesky(sparse_matrix* matrix, sparse_vector* b, sparse_vector* x, int n){
		
		if( n < 1)
			return 0;
	memcpy(x,b,n*sizeof(double));

	return cs_cholsol(1 , matrix , x);
}

int sparse_dc_sweep(LIST *list , sparse_matrix* matrix , sparse_vector* rhs){

	int i;

	int array_size;
	int vector_len;
	int vector_row;

	gsl_vector** plot_array = NULL;
	gsl_vector* plotting_vector = NULL;
	gsl_vector* iter_gsl_vector = NULL;
	gsl_vector* gsl_x_and_startup = NULL;
	sparse_vector* b = NULL; 
	sparse_vector* x = NULL;



	vector_len = matrix->n;
	plotting_vector = gsl_vector_calloc( vector_len);
	if( !plotting_vector){
		fprintf(stderr, "Not enough memory for plotting_vector...\n" );
		return 0;
	}

	gsl_x_and_startup = gsl_vector_calloc(vector_len);
	if(!gsl_x_and_startup){
		fprintf(stderr, "Not enough memory\n");
		return 0;
	}

	iter_gsl_vector = gsl_vector_calloc(vector_len);
	if(!iter_gsl_vector){
		fprintf(stderr, "Not enough memory...\n");
		return 0;
	}

	b = (sparse_vector*) malloc( sizeof(sparse_vector) * vector_len );
	if(!b){
		gsl_vector_free(plotting_vector);
		fprintf(stderr, "Not enough memory for sparse dc sweep using LU solver...\n" );	
		return 0;
	}

	x = (sparse_vector*) malloc( sizeof(sparse_vector)* vector_len);
	if(!x){
		free(b);
		fprintf(stderr, "Not enough memory for dc sweep sparse LU solution...\n" );
		return 0;
	}

	array_size = plot_find_size( list->dc_sweep.start_v, list->dc_sweep.end_v , list->dc_sweep.inc);
	plot_array = plot_create_vector( array_size , matrix->n);
	if( !plot_array ){
		free(b);
		free(x);
		fprintf(stderr, "No memory for plotting results...\n");
		return 0;
	}

	memcpy(b, rhs, sizeof(double) * vector_len);
	/*
	 * Set initial value for input source (voltage or current)
	 */
	if ( list->dc_sweep.node->type == NODE_SOURCE_V_TYPE ){
		vector_row = list->dc_sweep.node->node.source_v.mna_row;

		b[vector_row] = list->dc_sweep.start_v;
	}
	else if ( list->dc_sweep.node->type == NODE_SOURCE_I_TYPE){

		int node1 = list->dc_sweep.node->node.source_v.node1;
		int node2 = list->dc_sweep.node->node.source_v.node2;

		if(node1 != 0){
			b[node1 - 1] = list->dc_sweep.start_v; 
		}

		if(node2 != 0){
			b[node2 - 1 ] = list->dc_sweep.start_v;
		}
	}

	// find all solutions and plot
	for( i = 0 ; i < array_size ; i++ ){

		// solve sparse system using LU
		int  flag;
		if( list->solving_method == METHOD_LU_SPARSE ){ 
			flag = sparse_solve_LU(matrix, b, x, vector_len);

		}
		else if( list->solving_method == METHOD_CHOLESKY_SPARSE){
			flag =  sparse_solve_cholesky(matrix, b, x, vector_len);
		}
		else if( list->solving_method == METHOD_CG_SPARSE){
			lh_pointerVector_to_gslVector(b, iter_gsl_vector);
			sparse_solve_cg(matrix, iter_gsl_vector, gsl_x_and_startup);
			flag = 1;
		}
		else if( list->solving_method == METHOD_BICG_SPARSE){
			lh_pointerVector_to_gslVector(b, iter_gsl_vector);
			sparse_solve_bicg(matrix, iter_gsl_vector, gsl_x_and_startup);
			flag = 1;
		}
		
		if( !flag )
			fprintf(stderr, "DC SWEEP SPARSE ERROR: Solution not found\n");

		// update b vector
		if ( list->dc_sweep.node->type == NODE_SOURCE_V_TYPE ){
			b[vector_row] += list->dc_sweep.inc;
		}
		else if ( list->dc_sweep.node->type == NODE_SOURCE_I_TYPE){

			int node1 = list->dc_sweep.node->node.source_v.node1;
			int node2 = list->dc_sweep.node->node.source_v.node2;

			if(node1 != 0){
				b[node1 - 1] -= list->dc_sweep.inc; 
			}

			if(node2 != 0){
				b[node2 - 1 ] += list->dc_sweep.inc;
			}
		}

		// store solution found for plotting
		lh_pointerVector_to_gslVector(x, plotting_vector);
		if( list->solving_method == METHOD_CG_SPARSE || list->solving_method == METHOD_BICG_SPARSE ){
			plot_set_vector_index(plot_array, gsl_x_and_startup, i);

			int k;
			fprintf(stderr, "Priting solution: Iter = %d\n", i);
			for( k = 0 ; k < vector_len; k++)
				fprintf(stderr, "node %d: %f\n", k, gsl_vector_get(gsl_x_and_startup,k));
		}
		else{ 
			plot_set_vector_index(plot_array ,plotting_vector,i);
				
			int k;
			fprintf(stderr, "Priting solution: Iter = %d\n", i);
			for( k = 0 ; k < vector_len; k++)
				fprintf(stderr, "node %d: %f\n", k, gsl_vector_get(plotting_vector, k));
		}
		int k;
		for( k = 0 ; k < gsl_x_and_startup->size; k++)
			gsl_vector_set(gsl_x_and_startup, k, 0);

	} 

	if( list->plot == PLOT_ON )
		plot_by_node_name(list->hashtable , plot_array , array_size);

	gsl_vector_free(gsl_x_and_startup);
	gsl_vector_free(plotting_vector);
	return 1;
}

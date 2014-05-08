#include <stdlib.h>
#include <string.h>

#include "circuit_sim.h"
#include "decomposition.h"
#include "plot.h"
#include "iter_solve.h"

/*
 * Create the matrix and vector for the circuit elements
 */
int create_mna(LIST *list , gsl_matrix **matrix , gsl_vector** vector, int transient, gsl_matrix **c_matrix){

	LIST_NODE* curr;
	gsl_matrix* tmp_matrix;
	gsl_vector* tmp_vector;

	gsl_matrix* tmp_c_matrix;



	//int group1 = list->len - list->m2;
	//int group2 = list->m2;
	int m2_elements_found = 0;

	int rows;
	int columns;

	if( !matrix || !vector || !list)
		return 0;

	if(transient && !c_matrix)
		return 0;

	/* allocate matrix and vector */
	rows    = list->hashtable->num_nodes + list->m2;
 	columns = list->hashtable->num_nodes + list->m2;

 	printf("Creating matrix: rows = %d columns = %d\n",rows,columns);

 	tmp_matrix = gsl_matrix_calloc(rows , columns);
 	if( !tmp_matrix )
 		return 0;

	tmp_c_matrix = gsl_matrix_calloc(rows , columns);
 	if( !tmp_c_matrix )
 		return 0;

 	tmp_vector = gsl_vector_calloc( rows);
 	if( !tmp_vector )
 		return 0;	
 	
	/* compute mna */
 	for( curr = list->head ; curr ; curr = curr->next){

 		/*
 		 * RESISTANCE ELEMENT
 		 */

 		if( curr->type == NODE_RESISTANCE_TYPE ){

 			double conductance = 1 / curr->node.resistance.value ;
 			int plus_node  = curr->node.resistance.node1 - 1 ;
 			int minus_node = curr->node.resistance.node2  - 1;

 			/* <+> is ground */
 			if( plus_node == -1 ){

 				double value = gsl_matrix_get(tmp_matrix , minus_node , minus_node);
 				value += conductance ; 
 				gsl_matrix_set( tmp_matrix , minus_node , minus_node ,  value );
 				//printf("Adding to matrix element (%d,%d) value:%f\n\n",minus_node,minus_node,value);
 			}
 			else if( minus_node == -1  ){
 				/* <-> is ground */
 				double value = gsl_matrix_get(tmp_matrix , plus_node , plus_node);
 				value += conductance; 
 				gsl_matrix_set( tmp_matrix , plus_node ,plus_node , value );
				//printf("Adding to matrix element (%d,%d) value:%f\n\n",plus_node,plus_node,value);
 			}
 			else {

 				/* set <+> <+> matrix element */
 				double value;
 				

 				value = gsl_matrix_get(tmp_matrix , plus_node , plus_node);
 				value += conductance ; 
 				gsl_matrix_set(tmp_matrix , plus_node , plus_node , value);
				//printf("Adding to matrix element (%d,%d) value:%f\n",plus_node,plus_node,value);

 				/* set <+> <-> */
 				value = gsl_matrix_get(tmp_matrix , plus_node , minus_node);
 				value -= conductance ; 
 				gsl_matrix_set(tmp_matrix , plus_node , minus_node , value);
				//printf("Adding to matrix element (%d,%d) value:%f\n",plus_node,minus_node,value);

 				/* set <-> <+> */
 				value = gsl_matrix_get(tmp_matrix , minus_node , plus_node);
 				value -= conductance ; 
 				gsl_matrix_set(tmp_matrix , minus_node , plus_node , value); 				
				//printf("Adding to matrix element (%d,%d) value:%f\n",minus_node,plus_node,value);

 				/* set <-> <-> */
 				value = gsl_matrix_get(tmp_matrix , minus_node , minus_node);
 				value += conductance ; 
 				gsl_matrix_set(tmp_matrix , minus_node , minus_node , value);
				//printf("Adding to matrix element (%d,%d) value:%f\n\n",minus_node,minus_node,value);
 			}
 		}

 		/*
 		 * CAPACITY ELEMENT
 		 */
 		else if( curr->type == NODE_CAPACITY_TYPE  && transient){

 			double capacity = curr->node.capacity.value ;
 			int plus_node  = curr->node.capacity.node1 - 1 ;
 			int minus_node = curr->node.capacity.node2  - 1;

 			/* <+> is ground */
 			if( plus_node == -1 ){

 				double value = gsl_matrix_get(tmp_c_matrix , minus_node , minus_node);
 				value += capacity ; 
 				gsl_matrix_set( tmp_c_matrix , minus_node , minus_node ,  value );
 				//printf("Adding to matrix element (%d,%d) value:%f\n\n",minus_node,minus_node,value);
 			}
 			else if( minus_node == -1  ){
 				/* <-> is ground */
 				double value = gsl_matrix_get(tmp_c_matrix , plus_node , plus_node);
 				value += capacity; 
 				gsl_matrix_set( tmp_c_matrix , plus_node ,plus_node , value );
				//printf("Adding to matrix element (%d,%d) value:%f\n\n",plus_node,plus_node,value);
 			}
 			else {

 				/* set <+> <+> matrix element */
 				double value;
 				

 				value = gsl_matrix_get(tmp_c_matrix , plus_node , plus_node);
 				value += capacity ; 
 				gsl_matrix_set(tmp_c_matrix , plus_node , plus_node , value);
				//printf("Adding to matrix element (%d,%d) value:%f\n",plus_node,plus_node,value);

 				/* set <+> <-> */
 				value = gsl_matrix_get(tmp_c_matrix , plus_node , minus_node);
 				value -= capacity ; 
 				gsl_matrix_set(tmp_c_matrix , plus_node , minus_node , value);
				//printf("Adding to matrix element (%d,%d) value:%f\n",plus_node,minus_node,value);

 				/* set <-> <+> */
 				value = gsl_matrix_get(tmp_c_matrix , minus_node , plus_node);
 				value -= capacity ; 
 				gsl_matrix_set(tmp_c_matrix , minus_node , plus_node , value); 				
				//printf("Adding to matrix element (%d,%d) value:%f\n",minus_node,plus_node,value);

 				/* set <-> <-> */
 				value = gsl_matrix_get(tmp_c_matrix , minus_node , minus_node);
 				value += capacity ; 
 				gsl_matrix_set(tmp_c_matrix , minus_node , minus_node , value);
				//printf("Adding to matrix element (%d,%d) value:%f\n\n",minus_node,minus_node,value);
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
 				value  = gsl_vector_get(tmp_vector , curr->node.source_i.node1 - 1  );
 				value -= current;
 				gsl_vector_set(tmp_vector , curr->node.source_i.node1 -1  , value );
 			}

 			if( curr->node.source_i.node2 != 0 ){
 				/* <-> */
 				value  = gsl_vector_get(tmp_vector , curr->node.source_i.node2 - 1 );
 				value += current;
 				gsl_vector_set(tmp_vector , curr->node.source_i.node2 - 1 , value);
 			}
 		}
 		/*
 		 * VOLTAGE SOURCE
 		 */
 		else if ( curr->type == NODE_SOURCE_V_TYPE  ){
 			m2_elements_found++;
 			int matrix_row = list->hashtable->num_nodes  + m2_elements_found - 1 ;
 			curr->node.source_v.mna_row = matrix_row;
 			
 			double value;

 			double c_value;
 			/* set vector value */
 			value = gsl_vector_get(tmp_vector , matrix_row  );
 			value += curr->node.source_v.value;
 			c_value = value;
 			gsl_vector_set(tmp_vector, matrix_row , value);

 			/* Change the matrix */
 			int plus_node  = curr->node.source_v.node1 - 1 ;
 			int minus_node = curr->node.source_v.node2 - 1;

 			/* <+> */
 			if( plus_node != -1 ){

 				value = gsl_matrix_get(tmp_matrix , matrix_row , plus_node);
 				//value++;
 				gsl_matrix_set(tmp_matrix , matrix_row , plus_node , 1);
 				//printf("VOLTAGE SOURCE : (%d,%d) +1\n",matrix_row,plus_node);

 				value = gsl_matrix_get(tmp_matrix , plus_node , matrix_row);
 				//value++;
 				gsl_matrix_set(tmp_matrix , plus_node , matrix_row , 1); 
 				//printf("VOLTAGE SOURCE : (%d,%d) + 1 \n",plus_node,matrix_row);				
 			
 			} 
 			/* <->*/
 			if( minus_node != -1 ){
 				//value = gsl_matrix_get(tmp_matrix , matrix_row , minus_node);
 				//value++;
 				gsl_matrix_set(tmp_matrix , matrix_row , minus_node , -1);

 				//value = gsl_matrix_get(tmp_matrix , minus_node , matrix_row);
 				//value--;
 				gsl_matrix_set(tmp_matrix , minus_node , matrix_row , -1);
 			}
 		}
 		/*
 		 * Inductance
 		 */
 		else if ( curr->type == NODE_INDUCTANCE_TYPE  ){
 			m2_elements_found++;
 			int matrix_row = list->hashtable->num_nodes  + m2_elements_found - 1 ;
			double value;

			double c_value = curr->node.inductance.value;
 		
 			/* Change the matrix */
 			int plus_node  = curr->node.inductance.node1 - 1 ;
 			int minus_node = curr->node.inductance.node2 - 1;

 			/* <+> */
 			if( plus_node != -1 ){

 				value = gsl_matrix_get(tmp_matrix , matrix_row , plus_node);
 				//value++;
 				gsl_matrix_set(tmp_matrix , matrix_row , plus_node , 1);
 				//printf("VOLTAGE SOURCE : (%d,%d) +1\n",matrix_row,plus_node);

 				value = gsl_matrix_get(tmp_matrix , plus_node , matrix_row);
 				//value++;
 				gsl_matrix_set(tmp_matrix , plus_node , matrix_row , 1); 
 				//printf("VOLTAGE SOURCE : (%d,%d) + 1 \n",plus_node,matrix_row);				
 			
 			} 
 			/* <->*/
 			if( minus_node != -1 ){
 				//value = gsl_matrix_get(tmp_matrix , matrix_row , minus_node);
 				//value++;
 				gsl_matrix_set(tmp_matrix , matrix_row , minus_node , -1);

 				//value = gsl_matrix_get(tmp_matrix , minus_node , matrix_row);
 				//value--;
 				gsl_matrix_set(tmp_matrix , minus_node , matrix_row , -1);
 			}

 			if(transient)
 			{
 				//value = gsl_matrix_get(tmp_matrix , matrix_row , minus_node);
 				c_value = c_value * (-1);
 				gsl_matrix_set(tmp_matrix , matrix_row , matrix_row , c_value);
 			}
 		}
 	}

 	*matrix = tmp_matrix;
 	*c_matrix = tmp_c_matrix;
 	*vector = tmp_vector;
 	/* return */
 	return 1;
}


double dc_sweep_increment(gsl_vector *vector,DC_SWEEP_T dc_sweep_node)
{
	char node_type = dc_sweep_node.node->type;
	int vector_row;
	static int first_time = 0;

	if (  node_type == NODE_SOURCE_V_TYPE )
	{
		vector_row = dc_sweep_node.node->node.source_v.mna_row;
		/* Error check
			printf("V element for the dc sweep in row: %d\n",vector_row);
		 */
		double value = 0;
		/* set vector value */
		value = gsl_vector_get(vector, vector_row );
		value += dc_sweep_node.inc;
		if(!first_time){
			value = dc_sweep_node.start_v;
			first_time = 1;
		}
		gsl_vector_set(vector, vector_row , value);
		return abs(value);
	}
	else if( node_type == NODE_SOURCE_I_TYPE )
	{
		/* change only the vector */
		double current = dc_sweep_node.inc;
		double value;
		int node1 = dc_sweep_node.node->node.source_v.node1;
		int node2 = dc_sweep_node.node->node.source_v.node2;
		
		if( node1 != 0 ){
			/* ste <+> */
			value  = gsl_vector_get(vector , node1 - 1  );
			value -= current;
			if(!first_time){
				value = dc_sweep_node.start_v;
			}
			gsl_vector_set(vector , node1 - 1  , value );
		}

		if( node2 != 0 ){
			/* <-> */
			value  = gsl_vector_get(vector , node2 - 1 );
			value += current;
			if(!first_time){
				value = dc_sweep_node.start_v;
			}
			gsl_vector_set(vector , node2 - 1 , value);
		}
		if(!first_time)
			first_time = 1;
		
		return abs(value);
	}
	else
	{
		perror("Error found: Couldnt find a valid I or V source to do a DC sweep\n");
		return -1;
	}
	return 0;
}

void dc_sweep(LIST list, gsl_matrix* matrix, gsl_vector* vector, gsl_vector* x,gsl_permutation* permutation,int decomposition_choice)
{
	int k;
	int array_size;
	gsl_vector ** plot_array;
	gsl_vector* zero_vector;

	zero_vector = gsl_vector_calloc(vector->size);
	if(!zero_vector){
		printf("NO memory !\n");
		exit(1);
	}

	array_size = plot_find_size( list.dc_sweep.start_v, list.dc_sweep.end_v , list.dc_sweep.inc);
	//printf("Array Size for the plot: %d\n",array_size);

	plot_array = plot_create_vector( array_size , x->size);
	if(plot_array == NULL)
	{
		perror("Error while allocating the ploting array\n");
		exit(0);
	}
	
	//printf("%f %f\n",list.dc_sweep.start_v,list.dc_sweep.end_v);
	
 	for (k = 0; k < array_size ; k++)
 	{
 		dc_sweep_increment(vector,list.dc_sweep);

 		if( decomposition_choice == METHOD_CG ){
 			
 			gsl_vector_memcpy(x,zero_vector);

 			iter_solve_cg(matrix,vector,x);

 		}
 		else if ( decomposition_choice == METHOD_BICG){

 			gsl_vector_memcpy(x,zero_vector);
 			iter_solve_bicg(matrix,vector,x);

 		}
 		else{ 
 			solve(matrix,vector,x,permutation,decomposition_choice);
	 	}

 		plot_set_vector_index(plot_array ,x ,k);
 	}
 	
 	if(list.plot == PLOT_ON)
 	{
 		//plot_to_file(list.hashtable,plot_array,array_size,"results_plot_file.txt");
 		plot_by_node_name(list.hashtable , plot_array , array_size);
 	}
 	
 	gsl_vector_free(zero_vector);
}

#include <stdio.h>
#include <string.h>


#include "circuit_sim.h"
#include "node_list.h"

#include "parse.h"
#include "circuit_hash.h"
#include "sparse_interface.h"
#include "circuit_sim_sparse.h"

int main( int argc , char* argv[]){
	int flag;
	
	if( argc < 2 ){
		printf("Usage: %s <netlist>\n",argv[0]);
		return -1;
	}

	printf("Starting simulation....\n");
 	LIST list;
 	init_list(&list);
 	

 	//FILE *input_file=fopen(argv[1], "r");
 	//int flag = check_netlist_file(input_file,&list);
 	flag = parse_netlist(argv[1] , &list);
 	if( !flag ){
 		printf("ERROR BUILDING NETLIST\n");
 		free_list(&list);
 		return -1;
 	}

 	//printf("Solving Method = %s\n",solving_method_names[list.solving_method-1]);
	//  sparse simulation
	int vector_size;
	char method;
	sparse_matrix* matrix;
	sparse_vector* vector;
	sparse_vector* x;


	method = list.solving_method;
	matrix = (sparse_matrix*)create_mna_sparse( &list , &vector , &vector_size);
	if( !matrix ){
		fprintf(stderr, "Error creating MNA matrix \n");
		exit(1);
	}

	/* Print the matrix in a file */
	cs_print(matrix, "output_sparse_matrix", 0);

	/* clean up before exit */
	cs_spfree(matrix);

	//free(vector);
	//free(x);
/*
 * Clean up
 */
	free_list(&list);

	return 0;
}

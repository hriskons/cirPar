#include <stdio.h>
#include <string.h>

#include "node_list.h"
#include "parse.h"
#include "circuit_hash.h"
#include "sparse_interface.h"
#include "circuit_sim_sparse.h"
#include "graph_metis.h"


void parse_options(int argc, char* argv[],LIST *list,partition_t* par_graph)
{
	int flag;


	if( argc < 3 ){
		fprintf(stderr,"Usage: %s <netlist> <number of partitions>\n",argv[0]);
		exit(-1);
	}

	/* Set the number of clusters that you want metis to produce */
	par_graph->noOfParts = atoi(argv[2]);
	if(par_graph->noOfParts <= 0){
		fprintf(stderr,"Number of partitions must be > zero");
		exit(-1);
	}

	flag = parse_netlist(argv[1] , list);
	if( !flag ){
		fprintf(stderr,"PARSING NETLIST FAILED\n");
		free_list(list);
		exit(-1);
	}


}

int main( int argc , char* argv[]){


	/* Partition table */
	partition_t par_graph;


	printf("Starting simulation....\n");
 	LIST list;
 	init_list(&list);
 	

	parse_options(argc,argv,&list,&par_graph);

	int vector_size;
	sparse_matrix* matrix;
	sparse_vector* vector;
	//sparse_vector* x;


	//method = list.solving_method;
	matrix = (sparse_matrix*)create_mna_sparse( &list , &vector , &vector_size);
	if( !matrix ){
		fprintf(stderr, "Error creating MNA matrix \n");
		exit(1);
	}

	/* Print the matrix in a file */
	cs_print(matrix, "output_sparse_matrix", 0);

	graph_partition(&list,&par_graph);

	/* clean up before exit */
	cs_spfree(matrix);
	free(vector);
	free_list(&list);


	return 0;
}

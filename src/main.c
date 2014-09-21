#include <stdio.h>
#include <string.h>

#include "node_list.h"
#include "parse.h"
#include "circuit_hash.h"
#include "sparse_interface.h"
#include "circuit_sim_sparse.h"
#include "graph_metis.h"
#include "iter_solve_sparse.h"
#include <math.h>
#include <gsl/gsl_blas.h>
#include "helper_functions.h"
#include "solvers.h"
#include "graph_manipulation.h"


#define DEBUG 1
#define debug_print(fmt, ...) \
            do { if (DEBUG) fprintf(stderr, fmt, __VA_ARGS__); } while (0)

#define SOLVE 0
#define GRAPH 1

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

int  assign_values_of_cluster(cluster_t cluster,sparse_matrix* matrix,partition_t par_table,int num_cluster)
{
	int counter = 0;
	int j;

	for(j = 0; j < matrix->n; j++)
		if(par_table.partion_table[j] == num_cluster){
			cluster.ptr[counter] = &(matrix->p[j]);
			counter++;
		}

	cluster.value = matrix->x;
	cluster.size = counter;
	cluster.id = num_cluster;
	cluster.value = matrix->x;

	return counter;


}

int main( int argc , char* argv[]){


	/* Partition table */
	partition_t par_graph;
	sparse_matrix* graph_matrix;
	gsl_vector* x;

	printf("Starting simulation....\n");
 	LIST list;
 	init_list(&list);
 	

	parse_options(argc,argv,&list,&par_graph);
	//print_list(list);
	int vector_size;
	sparse_matrix* matrix;
	sparse_vector* vector;
	cluster_t cluster;

	int* num_of_elements_in_cluster;
	//sparse_vector* x;


	//method = list.solving_method;
	matrix = (sparse_matrix*)create_mna_sparse( &list , &vector , &vector_size);
	if( !matrix ){
		fprintf(stderr, "Error creating MNA matrix \n");
		exit(1);
	}

	/* Print the matrix in a file */
	//cs_print(matrix, "output_sparse_matrix", 0);
#if SOLVE
	solve(matrix,vector,x,vector_size);
#endif

#if GRAPH
	graph_matrix = graph_partition(&list,&par_graph);
	if(graph_matrix == NULL)
		exit(0);

	cs_print(graph_matrix,"graph_matrix",0);
	set_edge_weights(matrix,graph_matrix);

	set_weights_between_clusters(graph_matrix,par_graph);

	//set_weights_between_clusters(matrix,par_graph);

	cs_print_with_partition_table(graph_matrix,par_graph, "output_sparse_graph_matrix");
#endif
	num_of_elements_in_cluster = count_elements_in_each_cluster(par_graph);

	for(int i = 0; i < par_graph.noOfParts; i++)
		printf("Cluster:%d with %d elements\n",i,num_of_elements_in_cluster[i]);

	int max_elements = get_max(num_of_elements_in_cluster,par_graph.noOfParts);

	cluster.size = max_elements;
	cluster.ptr = (int **)safe_malloc(cluster.size * sizeof(int *));
	cluster.graph = graph_matrix;

	int cluster_number_assigning = 0;
	int values_assigned = assign_values_of_cluster(cluster,graph_matrix,par_graph,cluster_number_assigning);

	if( values_assigned != num_of_elements_in_cluster[cluster_number_assigning]){
		fprintf(stderr,"There were not assigned as many elements as expected on cluster pointers\n");
		fprintf(stderr,"Elements expected %d and elements assigned %d\n",num_of_elements_in_cluster[cluster_number_assigning],values_assigned);

	}
	char* filename = malloc(20);
	sprintf(filename,"output_cluster_no_%d",cluster_number_assigning);
	cs_print_from_pointers(cluster,filename);
	//print_gsl_vector_to_file(x,"solution_cg");

	/* clean up before exit */
	cs_spfree(matrix);
	free(vector);
	free_list(&list);


	return 0;
}





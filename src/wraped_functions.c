#include "wraped_functions.h"

idx_t* imalloc(int noOfElements, char * msg)
{
	idx_t* memory;

	memory = (idx_t*)gk_malloc(sizeof(idx_t) * noOfElements,msg);
	return memory;
}

real_t* rmalloc(int noOfElements, char * msg)
{
	real_t* memory;

	memory = (real_t*)gk_malloc(sizeof(real_t) * noOfElements,msg);
	return memory;
}

void* bmalloc(int bytes, char * msg)
{
	void* memory;

	memory = gk_malloc(bytes, msg);

	return memory;
}

idx_t* irealloc(idx_t* oldMemory,int noOfElementsNew)
{
	void* newMemory;

	newMemory = realloc(oldMemory,noOfElementsNew * sizeof(idx_t));
	if(newMemory == NULL)
	{
		fprintf(stderr,"Failed to allocate memory\n");
		exit(0);
	}
	return newMemory;
}



void print_vector(idx_t *vector, int length)
{
	int i;

	for (i = 0; i < length; i++)
		printf("%d ",vector[i]);
}

void print_input_data(graph_t *graph, params_t *params)
{
	int i,j;

	printf("\nAdjncy Vector: ");
	print_vector(graph->xadj,graph->nvtxs + 1);
	printf("\n");

	printf("Xadj Vector: ");
	print_vector(graph->adjncy,graph->nedges);
	printf("\n");

	if(graph->vwgt == NULL)
		printf("Vwgt == NULL\n");
	else{
		printf("vwgt Vector: ");
		print_vector(graph->vwgt,graph->nvtxs);
		printf("\n");
	}

	if(graph->vsize == NULL)
		printf("Vsize == NULL\n");
	else{
		printf("vsize Vector: ");
		print_vector(graph->vsize,graph->nvtxs);
		printf("\n");
	}

	printf("Nparts: %d\n",params->nparts);

	printf("tpwgts Vector: ");
	for (i = 0; i < params->nparts; i++) {
		for ( j = 0; j < graph->ncon; j++)
			printf("%f ",params->tpwgts[ i * graph->ncon + j ]);
	}
	printf("\n");

}

/* A wraped version of cs_entry from csparse library that offers error checking */
int csEntry(sparse_matrix *graph_matrix, int plus_node, int minus_node, double x)
{
	if( !cs_entry(graph_matrix, plus_node , minus_node , x) ){
		fprintf(stderr, "Error while inserting element in sparse matrix\n");
		cs_spfree(graph_matrix);
		exit(0);
	}
	return 1;
}

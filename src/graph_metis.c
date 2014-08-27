/*
 * graph_metis.c
 *
 *  Created on: May 9, 2014
 *      Author: christos
 */
#include "graph_metis.h"
#include "sparse_interface.h"
#include "csparse.h"
#include "GKlib.h"
#include "circuit_hash.h"

#define INIT_VERTICES 10
#define INIT_EDGES 10
#define DEFAULT_NZ 10
#define WITHOUT_ZERO 1

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

/*************************************************************************/
/*! This function creates and initializes a graph_t data structure */
/*************************************************************************/
graph_t *GK_CreateGraph(void)
{
  graph_t *graph;

  graph = (graph_t *)gk_malloc(sizeof(graph_t), "CreateGraph: graph");

  InitGraph(graph);

  return graph;
}


/*************************************************************************/
/*! This function creates and initializes a graph_t data structure */
/*************************************************************************/
graph_t *CreateGraph(void)
{
  graph_t *graph;

  graph = (graph_t *)malloc(sizeof(graph_t));
  if(graph == NULL){
	  fprintf(stderr,"Failed to allocate memory for graph\n");
	  exit(0);
  }

  InitGraph(graph);

  return graph;
}


/*************************************************************************/
/*! This function initializes a graph_t data structure */
/*************************************************************************/
void InitGraph(graph_t *graph)
{
  memset((void *)graph, 0, sizeof(graph_t));

  /* graph size constants */
  graph->nvtxs     = -1;
  graph->nedges    = -1;
  graph->ncon      = -1;
  graph->mincut    = -1;
  graph->minvol    = -1;
  graph->nbnd      = -1;

  /* memory for the graph structure */
  graph->xadj      = NULL;
  graph->vwgt      = NULL;
  graph->vsize     = NULL;
  graph->adjncy    = NULL;
  graph->adjwgt    = NULL;
  graph->label     = NULL;
  graph->cmap      = NULL;
  graph->tvwgt     = NULL;
  graph->invtvwgt  = NULL;

  /* by default these are set to true, but the can be explicitly changed afterwards */
  graph->free_xadj   = 1;
  graph->free_vwgt   = 1;
  graph->free_vsize  = 1;
  graph->free_adjncy = 1;
  graph->free_adjwgt = 1;


  /* memory for the partition/refinement structure */
  graph->where     = NULL;
  graph->pwgts     = NULL;
  graph->id        = NULL;
  graph->ed        = NULL;
  graph->bndptr    = NULL;
  graph->bndind    = NULL;
  graph->nrinfo    = NULL;
  graph->ckrinfo   = NULL;
  graph->vkrinfo   = NULL;

  /* linked-list structure */
  graph->coarser   = NULL;
  graph->finer     = NULL;
}

void METIS_init_params(params_t *params , int ncon)
{
	int i,j;

	/* Specifies the partitioning method. Possible values are: */
	//params->ptype = METIS_PTYPE_RB; 			/* Multilevel recursive bisectioning. */
	params->ptype = METIS_PTYPE_KWAY;			/* Multilevel k-way partitioning. */

	/*	Specifies the type of objective. Possible values are:*/
	params->objtype = METIS_OBJTYPE_CUT; 		/*Edge-cut minimization.*/
	//params->objtype = METIS_OBJTYPE_VOL; 		/*Total communication volume minimization.*/

	/*	Specifies the matching scheme to be used during coarsening. Possible values are:*/
	//params->ctype = METIS_CTYPE_RM; 			/*	Random matching.*/
	params->ctype = METIS_CTYPE_SHEM;			/*	Sorted heavy-edge matching.*/

	/*	Determines the algorithm used during initial partitioning. Possible values are:*/
	params->iptype = METIS_IPTYPE_GROW; 		/*	Grows a bisection using a greedy strategy.*/
	//params->iptype = METIS_IPTYPE_RANDOM;		/*	Computes a bisection at random followed by a refinement.*/
	//params->iptype = METIS_IPTYPE_EDGE; 		/*	Derives a separator from an edge cut.*/
	//params->iptype = METIS_IPTYPE_NODE; 		/*	Grow a bisection using a greedy node-based strategy.*/

	/*	Determines the algorithm used for refinement. Possible values are:*/
	//params->rtype = METIS_RTYPE_FM; 			/*	FM-based cut refinement.*/
	params->rtype = METIS_RTYPE_GREEDY;		/*	Greedy-based cut and volume refinement.*/
	//params->rtype = METIS_RTYPE_SEP2SIDED; 	/*	Two-sided node FM refinement.*/
	//params->rtype = METIS_RTYPE_SEP1SIDED; 	/*	One-sided node FM refinement.*/

	/* 	Specifies the number of different partitionings that it will compute. The final partitioning is the one that
		achieves the best edgecut or communication volume. Default is 1. */
	params->ncuts = 1;

	/* 	Specifies the number of different separators that it will compute at each level of nested dissection. The final
		separator that is used is the smallest one. Default is 1.  */
	params->nseps = 1;

	/*Used to indicate which numbering scheme is used for the adjacency structure of a graph or the element-
	node structure of a mesh. The possible values are:*/
	params->numflag = 0; 						/*	C-style numbering is assumed that starts from 0.*/
	//params->numflag = 1; 						/*	Fortran-style numbering is assumed that starts from 1.*/

/*	Specifies the number of iterations for the refinement algorithms at each stage of the uncoarsening process.
	Default is 10.*/
	params->niter = 10;

	/* Specifies the seed for the random number generator.*/
	params->seed = -1;

	/*Specifies that the partitioning routines should try to minimize the maximum degree of the subdomain graph,
	i.e., the graph in which each partition is a node, and edges connect subdomains with a shared interface.
	*/
	//params->minconn = 0; 	/*Does not explicitly minimize the maximum connectivity.*/
	params->minconn = 1; 	/*Explicitly minimize the maximum connectivity.*/

	/*	Specifies that the coarsening will not perform any 2–hop matchings when the standard matching approach
	fails to sufficiently coarsen the graph. The 2–hop matching is very effective for graphs with power-law
	degree distributions*/
	params->no2hop = 0; 					/* Performs a 2–hop matching.*/
	//params->no2hop = 1; 					/* Does not perform a 2–hop matching.*/

	/*	Specifies that the partitioning routines should try to produce partitions that are contiguous. Note that if the
	input graph is not connected this option is ignored.*/
	params->contig = 0; 					/*	Does not force contiguous partitions.*/
	//params->contig = 1;					/*	Forces contiguous partitions.*/

	/*Specifies that the graph should be compressed by combining together vertices that have identical adjacency
	lists.*/
	params->compress = 0; 					/*	Does not try to compress the graph.*/
	//params->compress = 1; 				/*	Tries to compress the graph.*/

	/*	Specifies if the connected components of the graph should first be identified and ordered separately.*/
	params->ccorder = 0; 					/*	Does not identify the connected components.*/
	//params->ccorder = 1; 					/*	Identifies the connected components.*/

	params->pfactor = 0;

	params->nparts = 2; 					/* The number of parts to partition the graph. */

	switch (params->pfactor) {
		case (METIS_PTYPE_RB) : { 	params->ufactor = 1; break;}
		case (METIS_PTYPE_KWAY):{ 	params->ufactor = 1.030; }


	}

	params->tpwgts = rmalloc(params->nparts * ncon ,"ReadTPwgts: tpwgts");

	for (i = 0; i < params->nparts; i++) {
	  for ( j = 0; j < ncon; j++)
		params->tpwgts[ i * ncon + j ] = 1.0/params->nparts;
	}
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

void graph_partition(LIST *list , partition_t* partition_info)
{
	graph_t* graph;
	params_t *params;
	idx_t options[METIS_NOPTIONS], status;
	idx_t objval , *part;
	int j;

	graph = ReadGraph(list);

	params = bmalloc(sizeof(*params), "Allocating memory for params");

	METIS_SetDefaultOptions(options);
	METIS_init_params(params, graph->ncon);
	options[METIS_OPTION_OBJTYPE] = params->objtype;
	options[METIS_OPTION_CTYPE]   = params->ctype;
	options[METIS_OPTION_IPTYPE]  = params->iptype;
	options[METIS_OPTION_RTYPE]   = params->rtype;
	options[METIS_OPTION_NO2HOP]  = params->no2hop;
	options[METIS_OPTION_MINCONN] = params->minconn;
	options[METIS_OPTION_CONTIG]  = params->contig;
	options[METIS_OPTION_SEED]    = params->seed;
	options[METIS_OPTION_NITER]   = params->niter;
	options[METIS_OPTION_NCUTS]   = params->ncuts;
	options[METIS_OPTION_UFACTOR] = params->ufactor;
	options[METIS_OPTION_DBGLVL]  = params->dbglvl;

	//print_input_data(graph,params);

	params->tpwgts = NULL;
	params->ubvec = NULL;
	partition_info->partion_table = imalloc(graph->nvtxs, "Allocate memory for part");

	switch (params->ptype) {
	    case METIS_PTYPE_RB:
	      status = METIS_PartGraphRecursive(&graph->nvtxs, &graph->ncon, graph->xadj,
	                   graph->adjncy, graph->vwgt, graph->vsize, graph->adjwgt,
	                   &params->nparts, params->tpwgts, params->ubvec, options,
	                   &objval, partition_info->partion_table);
	      break;

	    case METIS_PTYPE_KWAY:
	      status = METIS_PartGraphKway(&graph->nvtxs, &graph->ncon, graph->xadj,
	                   graph->adjncy, graph->vwgt, graph->vsize, graph->adjwgt,
	                   &params->nparts, params->tpwgts, params->ubvec, options,
	                   &objval, partition_info->partion_table);
	      break;

	}
	if (status != METIS_OK) {
		fprintf(stderr,"\n***Metis returned with an error.***\n");
	}
	printf("Objval: %d\n",objval);
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

void double_to_int( double *x ,int *adjwgt ,int size)
{
	int i;

	for(i = 0; i < size ; i++)
	{
		adjwgt[i] = ((int)(x[i] + 1) >> 1);
	}
}

void int_to_idx(int *vector_int , idx_t *vector_idx,int size)
{
	int i;

	for(i = 0; i < size; i++)
	{
		vector_idx[i] = vector_int[i];
	}
}

/*
idx_t unhash_nodes(hashtable_t* hashtable, idx_t hashed_node)
{
	int key;
	char ret[15];

	if(!ht_get_value_to_key(hashtable,hashed_node,ret))
	{
		fprintf(stderr,"Hashed node wasn't able to be located\n");
		fprintf(stderr,"Node that failed: %d\n",hashed_node);
		exit(-1);
	}

	//printf("Node %s was translated to %d\n",key,hashed_node[0]);
	return atoi(ret);
	//printf("Node %d\n",ret);
}
*/
graph_t *ReadGraph(LIST *list)
{
	int i;
	int rows , columns;
	LIST_NODE* curr;
	graph_t *graph;
	sparse_matrix *graph_matrix;


	/* Initialization of the graph */
	graph = GK_CreateGraph();

	int num_nodes = ht_get_num_nodes(list->hashtable);
	int* nodeids = (int*)malloc(sizeof(int) * num_nodes);
	if(!nodeids)
		return NULL;

	for( i = 0; i < num_nodes; i++)
		nodeids[i] = 0;

	/* allocate matrix */
	rows    = list->hashtable->num_nodes + list->m2;
	columns = list->hashtable->num_nodes + list->m2;


	graph_matrix = cs_spalloc( rows , columns , DEFAULT_NZ , 1 , 1 );
 	if(!graph_matrix)
 		return NULL;

	for( i = 0, curr = list->head ; curr; curr = curr->next, i++){
	    /*
		 * RESISTANCE ELEMENT
		 */
		if( curr->type == NODE_RESISTANCE_TYPE ){
			idx_t plus_node = curr->node.resistance.node1;
			idx_t minus_node = curr->node.resistance.node2;

			if(plus_node == 0)
			{
				csEntry(graph_matrix, 0 , minus_node , 1.0);

				csEntry(graph_matrix, minus_node , 0 , 1.0);
			}
			else if(minus_node == 0)
			{
				csEntry(graph_matrix, 0 , plus_node , 1.0);

				csEntry(graph_matrix, plus_node , 0 , 1.0);
			}
			if( plus_node != 0 && minus_node != 0)
			{
				/* <+> <-> */
				csEntry(graph_matrix, plus_node , minus_node , 1.0);

				/* <-> <+> */
				csEntry(graph_matrix, minus_node , plus_node , 1.0);
			}
		}
	}
	/* just checking the number of non_zero elements */

	graph->nvtxs = rows;
	graph->nedges = graph_matrix->nz;
	graph->ncon = 1;

	printf("No of vertices: %d\n",graph->nvtxs);
	printf("No of edges: %d\n",graph->nedges);
	printf("No of conditions: %d\n",graph->ncon);

	/* convert the sparse matrix into a compressed column form */
	graph_matrix = 	cs_compress(graph_matrix);

	cs_print(graph_matrix,"sparse_matrix",0);

	graph->xadj   = imalloc(graph->nvtxs + 1, "Allocate memory for graph->xadj");
	graph->adjncy = imalloc(graph->nedges,  "Allocate memory for graph->adjncy");
	graph->adjwgt = imalloc(graph->nedges,  "Allocate memory for graph->adjwgt");
	graph->vwgt   = imalloc(graph->ncon * graph->nvtxs, "ReadGraph: vwgt");
	graph->vsize  = imalloc(graph->nvtxs,  "ReadGraph: vsize");


	int_to_idx(graph_matrix->p,graph->xadj, graph->nvtxs + 1);
	int_to_idx(graph_matrix->i , graph->adjncy, graph->nedges);
	double_to_int( graph_matrix->x , graph->adjwgt , graph->nedges);
	int_to_idx(graph->adjwgt,graph->vwgt, graph->nvtxs);
	int_to_idx(graph->adjwgt,graph->vsize, graph->nvtxs);


	cs_print(graph_matrix,"sparse_matrix",0);



	return graph;
}






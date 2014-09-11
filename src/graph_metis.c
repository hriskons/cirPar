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
#include "wraped_functions.h"
#include "graph_functions.h"


#define DEBUG 1




sparse_matrix* graph_partition(LIST *list , partition_t* partition_info)
{
	graph_t* graph;
	params_t *params;
	idx_t options[METIS_NOPTIONS], status;
	idx_t objval;
	sparse_matrix* matrix = (sparse_matrix*) malloc(sizeof(sparse_matrix));

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
		return NULL;
	}
    partition_info->size = graph->nvtxs;
    partition_info->noOfParts = params->nparts;
    WritePartition(params->filename, partition_info->partion_table, graph->nvtxs, params->nparts);

	printf("Objval: %d\n",objval);

	matrix->nz = -1;
	matrix->n = graph->nvtxs;
	matrix->p = graph->xadj;
	matrix->i = graph->adjncy;



	return matrix;
}






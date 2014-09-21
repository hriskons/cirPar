/*
 * graph_metis.h
 *
 *  Created on: May 9, 2014
 *      Author: christos
 */

#ifndef GRAPH_METIS_H_
#define GRAPH_METIS_H_

#include <stdlib.h>
#include "node_list.h"
#include "struct.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "metis.h"
#include "GKlib.h"
#include "sparse_interface.h"

#define INIT_VERTICES 10
#define INIT_EDGES 10
#define DEFAULT_NZ 10
#define WITHOUT_ZERO 1


typedef struct {
	idx_t noOfParts;
	idx_t size;
	idx_t* partion_table;

} partition_t;


/*************************************************************************/
/*! This data structure stores the various command line arguments */
/*************************************************************************/
typedef struct {
  idx_t ptype;
  idx_t objtype;
  idx_t ctype;
  idx_t iptype;
  idx_t rtype;

  idx_t no2hop;
  idx_t minconn;
  idx_t contig;

  idx_t nooutput;

  idx_t balance;
  idx_t ncuts;
  idx_t niter;

  idx_t gtype;
  idx_t ncommon;

  idx_t seed;
  idx_t dbglvl;

  idx_t nparts;

  idx_t nseps;
  idx_t ufactor;
  idx_t pfactor;
  idx_t compress;
  idx_t ccorder;

  idx_t wgtflag;
  idx_t numflag;
  real_t *tpwgts;
  real_t *ubvec;

  size_t maxmemory;

  char* filename;
} params_t;





graph_t *ReadGraph(LIST *list);

void InitGraph(graph_t *graph);

void double_to_int( double *x ,int *adjwgt ,int size);

void int_to_idx(int *vector_int , idx_t *vector_idx,int size);

sparse_matrix* graph_partition(LIST *list , partition_t* partition_table);


#endif /* GRAPH_METIS_H_ */

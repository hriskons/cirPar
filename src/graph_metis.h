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

typedef struct {
	idx_t noOfParts;
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
} params_t;




void graph_partition(LIST *list , partition_t* partition_table);

graph_t *ReadGraph(LIST *list);

void InitGraph(graph_t *graph);




#endif /* GRAPH_METIS_H_ */

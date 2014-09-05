#ifndef GRAPH_FUNCTIONS_H
#define GRAPH_FUNCTIONS_H

#define MAXLINE			1280000

#include "metis.h"
#include "struct.h"
#include "graph_metis.h"
#include "sparse_interface.h"

void WritePartition(char *fname, idx_t *part, idx_t n, idx_t nparts);


/*************************************************************************/
/*! This function creates and initializes a graph_t data structure */
/*************************************************************************/
graph_t *GK_CreateGraph(void);


/*************************************************************************/
/*! This function creates and initializes a graph_t data structure */
/*************************************************************************/
graph_t *CreateGraph(void);


/*************************************************************************/
/*! This function initializes a graph_t data structure */
/*************************************************************************/
void InitGraph(graph_t *graph);

void METIS_init_params(params_t *params , int ncon);

graph_t *ReadGraph(LIST *list);



#endif

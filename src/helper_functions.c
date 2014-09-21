/*
 * helper_functions.c
 *
 *  Created on: Sep 18, 2014
 *      Author: christos
 */
#include "helper_functions.h"
#include "graph_metis.h"

/***********************************************************************************************
 *
 * ************************************ PRINT FUNCTIONS ****************************************
 *
 ***********************************************************************************************/
void print_gsl_vector_to_file(gsl_vector* vector,const char* filename){
	int i;
	FILE* outputFile;

	outputFile = fopen(filename,"w");
	if(outputFile == NULL){
		fprintf(stderr,"Failed to open file: %s\n",filename);
		return;
	}

	for(i = 0; i < vector->size; i++)
		fprintf(outputFile, "%e\n", gsl_vector_get(vector,i));

}

void print_vector_to_file(sparse_vector* vector,int vector_size,const char* filename){
	int i;
	FILE* outputFile;

	outputFile = fopen(filename,"w");
	if(outputFile == NULL){
		fprintf(stderr,"Failed to open file: %s\n",filename);
		return;
	}

	for(i = 0; i < vector_size; i++)
		fprintf(outputFile, "%e\n", vector[i]);

}

void sparse_to_gsl_vector(double *doubl_vec,gsl_vector* gsl_v,int size_n)
{
	int i;
	for (i = 0; i < size_n; i++)
	{
		gsl_vector_set(gsl_v,i,doubl_vec[i]);
	}
}

void* safe_malloc(const int size)
{
	void *temp;
	temp = malloc(size);
	if(temp == NULL){
		fprintf(stderr,"Malloc allocation failed\n");
		exit(-1);
	}

	return temp;
}
/***********************************************************************************************
 *
 * ************************************* CS FUNCTIONS ******************************************
 *
 ***********************************************************************************************/

int cs_print_with_partition_table(sparse_matrix* A, partition_t par_table,char* outputFilename)
{
	int p, j, m, n, nzmax, *Ap, *Ai,*B;
	double *Ax;
	FILE *outputFilePtr;

	outputFilePtr = fopen(outputFilename, "w");
	if (outputFilePtr == NULL) {

		fprintf(stderr, "Could not open output file %s for writing\n", outputFilename);
		exit(EXIT_FAILURE);
	}

	if (!A) {
		fprintf(outputFilePtr, "(null)\n");
		return (0);
	}
	m = A->m;
	n = A->n;
	Ap = A->p;
	Ai = A->i;
	Ax = A->x;
	nzmax = A->nzmax;
	B = par_table.partion_table;

	fprintf(outputFilePtr, "%d-by-%d, nzmax: %d nnz: %d\n", m, n, nzmax, Ap[n]);
	for (j = 0; j < n; j++) {
		fprintf(outputFilePtr, "    col %d : locations %d to %d %d\n", j, Ap[j], Ap[j + 1] - 1,B[j]);
		for (p = Ap[j]; p < Ap[j + 1]; p++) {
			fprintf(outputFilePtr, "      %d : %g\n", Ai[p], Ax ? Ax[p] : 1);
		}
	}

	fflush(outputFilePtr);
	fclose(outputFilePtr);
	return (1);
}

int cs_print_from_pointers(cluster_t cluster,char* outputFilename)
{
	FILE *outputFilePtr;
	int n,j,p;
	int** ptr, *Ap;
	int *Ai;
	double* Ax;

	outputFilePtr = fopen(outputFilename, "w");
	if (outputFilePtr == NULL) {

		fprintf(stderr, "Could not open output file %s for writing\n", outputFilename);
		exit(EXIT_FAILURE);
	}

	n = cluster.size;
	ptr = cluster.ptr;
	Ai = cluster.graph->i;
	Ax = cluster.graph->x;

	fprintf(outputFilePtr, "Cluster id: %d Number of elements: %d\n",cluster.id,cluster.size);
	for (j = 0; j < n; j++) {
		Ap = ptr[j];
		fprintf(outputFilePtr, "    col %d : locations %d to %d\n", j, Ap[j], Ap[j + 1] - 1);
		for (p = Ap[j]; p < Ap[j + 1]; p++) {
			fprintf(outputFilePtr, "      %d : %g\n", Ai[p],Ax[p]);
		}
	}

	fflush(outputFilePtr);
	fclose(outputFilePtr);
	return (1);
}

double cs_find(sparse_matrix* A,int column,int row)
{
	int p;
	printf("cs_find\n");
	for (p = A->p[column]; p < A->p[column + 1]; p++){
		printf("\t%d %d\n",A->i[p],row);

		if(row == A->i[p])
			return A->x[p];
	}

	return -1.0; /* This return value is not completely correct because there is a
				chance that it will be the same as a valid one even though it is not. */

}

int* count_elements_in_each_cluster(partition_t par_table)
{
	int i;
	int* num_of_elements = (int *)safe_malloc(par_table.noOfParts * sizeof(int));
	memset(num_of_elements, 0 , par_table.noOfParts * sizeof(int));

	for(i = 0; i < par_table.size; i++)
		num_of_elements[par_table.partion_table[i]]++;

	return num_of_elements;
}


int get_max(int* vec,int size)
{
	int max = vec[0];

	for(int i = 1; i < size; i++)
		if(max < vec[i])
			max = vec[i];
	return max;

}






#include <stdio.h>
#include <string.h>
#include "plot.h"


#define MAX_STRINGS 100

char *node_names[MAX_STRINGS];
int num_node_names;


/*
 * Call this before using the plot methods
 */
void plot_init(){
	int i;

	num_node_names = 0 ; 

	for( i = 0 ; i < MAX_STRINGS ; i++ )
		node_names[i] = NULL;

}


/*
 * Return the size  of an array of vectors
 */ 
int plot_find_size( double startv , double endv , double inc ){

	double iter;
	unsigned int num_elements = 0;
	for( iter = startv ; iter <= endv ; iter = iter + inc )
		num_elements++;


	return num_elements;
}

/*
 * Adds a node name for plotting
 * Returns 1 when ok
 *         0 when failed
 */
int plot_add_node(char* node_name){

	if( num_node_names > MAX_STRINGS )
		return 0;

	node_names[num_node_names] = strdup(node_name);
	if( !node_names[num_node_names] )
		return 0;

	num_node_names++;
	return 1;

}

/*
 * Gets an array of gsl_vectors.Starting values = 0.0
 * returns : NULL at failure
 */
gsl_vector** plot_create_vector( int array_size  ,  int vector_size ){

	int i;
	gsl_vector **vector_array = NULL;
	if( array_size < 1 )
		return NULL;
	array_size++;
	vector_array = ( gsl_vector**) malloc( sizeof(gsl_vector*) * array_size );
	if( !vector_array )
		return NULL;

	for( i = 0 ; i < array_size ; i++){
		
		vector_array[i] = gsl_vector_calloc( vector_size );
		if( !(vector_array[i]) ){
			printf("Out of memory...\n");
			exit(1);
		}
	}

	return vector_array;
}

/*
 * writes an array of vectors to a file 
 */
void plot_to_file( hashtable_t* hashtable, gsl_vector** array , int array_size, char* filename){

	int i,j;
	//int vector_size;
	int node_id;

	//char final_file_name[100];

	FILE* file;
	file = fopen( filename , "w");
	if( !file ){
		printf("Plot Failed\n");
		return;
	}

	fprintf(file, "##### SOLUTIONS ######\n");

	for( i = 0 ; i < array_size ; i++ ){
		fprintf(file, "Solution: %d\n",(i+1) );
		//vector_size = array[i]->size;
				
		for( j = 0 ; j < num_node_names ; j++){
			ht_get(hashtable , node_names[j] , &node_id);
			fprintf(file, "\t%s : %f\n", node_names[j] , gsl_vector_get( array[i] , node_id - 1 ) );

		}
		fprintf(file, "\n");
	}
	fclose(file);

	return ;
}



/*
 * Copies vector b at array[index]
 */
int plot_set_vector_index(gsl_vector** array , gsl_vector* b , int index){
	return gsl_vector_memcpy( array[index] , b);

}


/*
 * Deallocates all vectors
 */
void plot_free_array(gsl_vector** array , int array_size){

	int i;
	for( i = 0 ; i < array_size ; i ++ ){
		gsl_vector_free(array[i]);
	}

	free( array);

}

/*
 * prints an array of vectors to a stderr 
 */
void plot_to_screen( hashtable_t* hashtable, gsl_vector** array , int array_size){


	int i,j;
	int node_id;

	printf("##### SOLUTIONS ######\n");

	for( i = 0 ; i < array_size ; i++ ){
		printf("Solution: %d\n",(i+1) );
		//vector_size = array[i]->size;
				
		for( j = 0 ; j < num_node_names ; j++){
			ht_get(hashtable , node_names[j] , &node_id);
			printf("\t%s : %f\n", node_names[j] , gsl_vector_get( array[i] , node_id - 1 ) );

		}
		
	}
}

/*
 * Create files named after their representative node name and print the results
 */
void plot_by_node_name(hashtable_t* hashtable , gsl_vector** array , int array_size){

	char final_file_name[100];
	char filename_prefix[]    = "node_";
	char filename_extension[] = ".txt";

	int i,j;
	int node_id;
	int legit_node;

	for( i = 0 ; i < num_node_names ; i++ ){
		legit_node = ht_get(hashtable , node_names[i] , &node_id);
		
		if( legit_node ){
			FILE* file;
			
			memset(final_file_name , 0 , 100);

			strcpy(final_file_name , filename_prefix);
			strcat(final_file_name , node_names[i]);
			strcat(final_file_name , filename_extension );

			file = fopen( final_file_name , "w" );
			
			if( !file )
				continue;		// do not print this node
			fprintf(file, "RESULTS:\n\n" );
			for( j = 0 ; j < array_size ; j++){
				fprintf(file, "%f\n", gsl_vector_get( array[j] , node_id - 1 ) );
			}
			fclose(file);
		}

	} 

}



/*
 * Deallocate all memory used by the plot module
 */
void plot_destroy(){

	int i;
	for(i = 0 ; i < num_node_names ; i++ ){
		free( node_names[i]);
		node_names[i] = NULL;
	}

	num_node_names = 0;
}


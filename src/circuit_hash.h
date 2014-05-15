#ifndef CIRCUIT_HASH_H
#define CIRCUIT_HASH_H

struct entry_s{

	char *key;
	int ikey;
	int value;

	struct entry_s *next; 
};

typedef struct entry_s entry_t;


struct hashtable_s{
	int size;
	int num_nodes;
	entry_t **table;
};

typedef struct hashtable_s hashtable_t;


/*
 * Init hash table
 *
 */
hashtable_t* ht_init( int size );

/*
 * Hash function
 */
unsigned int hash(hashtable_t *hashtable,char* key);

/*
 * Creates a new key - value pair ( circuit node name - int )
 */
entry_t* ht_create_pair(char* key,int value);


/*
 * add pair
 *
 * return:  > 0 success
 *		   == 0 no memory
 *          < 0 key already exists( no value replacement )
 */
int ht_insert_pair(hashtable_t *hashtable, char* key, int value );

/*
 * get value
 *
 * Returns: 1 = success. ret points to value of the matching key
 *			0 = key not found
 */
int ht_get(hashtable_t* hashtable , char* key , int *ret);

/*
 * Deallocate
 */
void ht_free(hashtable_t *hashtable);

/*
 * print all pairs
 */
void ht_print(hashtable_t* hashtable);

/*
 * get active table entries
 */
int ht_get_num_nodes(hashtable_t* hashtable);
#endif

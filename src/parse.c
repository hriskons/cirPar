/*
 * Small code for reading the netlist file line by line,
 * extracting string tokens from each line,
 * and recognizing the type of element described on each line.
 */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "plot.h"


#define RESISTANCE_NUM_PARSE_ELEMENTS  4
#define CAPACITY_NUM_PARSE_ELEMENTS    4
#define INDUCTANCE_NUM_PARSE_ELEMENTS  4
#define MOSFET_NUM_PARSE_ELEMENTS      6 


static int get_node_from_line( LIST* list,char* line , NODE* node , int* type);
void parse_node_names(char* token,GENERAL_NODE* node,LIST* list, long* node_count);
int read_transient_elements(char* token,GENERAL_NODE* node);
void read_mos_elements(char* token,GENERAL_NODE* node,LIST* list,long* node_count);
void read_bjt_elements(char* token,GENERAL_NODE* node,LIST* list,long* node_count);

void safe_strtok(char**token,char* line,char* input)
{

	*token = strtok(line,input);
	if( *token == NULL ){
		fprintf(stderr,"Exiting from strtok because it did not return a token\n");
		exit(0);
	}
}

/*
 * Parse netlist
 */
int parse_netlist(char* filename , LIST* list){

  char line[ MAX_LINE_SIZE + 1];
  NODE element_node;
  int element_type;
  int line_number;
  int res;

  if( !filename || !list  )
    return 0;

  FILE* file;
  file = fopen(filename,"r");
  if( !file ){
	  fprintf(stderr,"Failed to open the netlist.\n");
	  return 0;
  }
  list->non_zero_elements = 0;
  line_number = 1 ;
  /*Read until EOF */
  while( !feof(file)){

    if( fgets( line , MAX_LINE_SIZE , file) != NULL ) {									/* Get a single line */
		if( line[0] != '*'){															/* check for comment,else process */
			res = get_node_from_line( list, line , &element_node , &element_type);

			if( res == 1 ){																/* add node read and store at list */
				res = add_node_to_list(list , &element_node , element_type);
				if( !res ){
					printf("NO MEMORY\n");
					return 0;
				}
			}else if( res == 0 ){														/* Error while parsing line */
				fclose(file);
				fprintf(stderr,"Error while parsing.Line %d : %s\n",line_number , line);
				return 0;
			}
		}
    }
    line_number++;
  }

  if( list->has_reference == 1 ){
   return 1;
  }
  else{
    printf("No reference node (ground) specified\n");
    return 0;
  }
}

int parse_line(GENERAL_NODE* node,char* line,LIST* list,char type)
{
	char* token;
	static long node_count = 1;

	/* read name */
	safe_strtok(&token,line," ");
	strcpy( node->name , token);

	parse_node_names(token,node,list,&node_count);

	token = strtok(NULL," ");
	if( token == NULL || token[0] == '\n'){
		return node_count;
	}else{
		printf("Extra token: %d\n",token[0]);
		switch (type){
			case 'V':
			case 'v':
			case 'I':
			case 'i': { 	read_transient_elements(token,node);					break;	}
			case 'M':
			case 'm': {		read_mos_elements(token,node,list,&node_count);			break;	}
			case 'Q':
			case 'q': {		read_bjt_elements(token,node,list,&node_count);			break;	}
		}

		return node_count;
	}
}


int read_node(LIST* list,char* token,long* node_count)
{
	int flag;

	/* check for reference node (ground) */
	if( strcmp(token,"0") == 0 ){
		list->has_reference = 1;
		return 0;
	}

	/* this is not a reference node.Add string to hash table */
	flag = ht_insert_pair(list->hashtable, token , *node_count);
	if( flag == 1 ){	  		/* successfull insertion */
		(*node_count)++;    		// get ready for the next node
		return (*node_count) - 1;
	}else if( flag == 0 ){	 	/* NULL pointer or out of memory */

		fprintf(stderr,"Error at inserting pair to hash table..\n");
		exit(1);
	}else if( flag == -1 ){

		int n;

		if (!ht_get(list->hashtable,token,&n))
		{
		  fprintf(stderr,"Token failed: %s \n",token);
		  perror("Key was not found in the hash table\n The program will exit\n");
		  exit(0);
		}
		return n;
	}

	/* It should not reach this point */
	return -1;
}


void parse_node_names(char* token,GENERAL_NODE* node,LIST* list, long* node_count)
{

	/* Read <+> node */
	safe_strtok(&token,NULL," ");
	node->node1 = read_node(list,token,node_count);

	/* Read <-> node */
	safe_strtok(&token,NULL," ");
	node->node2 = read_node(list,token,node_count);

	/* read value node */
	safe_strtok(&token,NULL," ");
	node->value = atof(token);

}
int read_transient_elements(char* token,GENERAL_NODE* node)
{
	printf("Read transient elements %s\n",token);
    /*check for exponential transient spec*/
    if (strcmp(token,"EXP") == 0 || strcmp(token,"exp") == 0){

		safe_strtok(&token,NULL,"() \n");
		node->i1 = atof(token);

		safe_strtok(&token,NULL,"() \n");
		node->i2 = atof(token);

		safe_strtok(&token,NULL,"() \n");
		node->td1 = atof(token);

		safe_strtok(&token,NULL,"() \n");
		node->tc1 = atof(token);

		safe_strtok(&token,NULL,"() \n");
		node->td2 = atof(token);

		safe_strtok(&token,NULL,"() \n");
		node->tc2 = atof(token);

		node->pulse_type = PULSE_EXP;
    }
    /*check for SIN transient spec*/
    else if(strcmp(token,"SIN") == 0 || strcmp(token,"sin") == 0){

		safe_strtok(&token,NULL,"() \n");
		node->i1 = atof(token);

		safe_strtok(&token,NULL,"() \n");
		node->ia = atof(token);

		safe_strtok(&token,NULL,"() \n");
		node->fr = atof(token);

		safe_strtok(&token,NULL,"() \n");
		node->td = atof(token);

		safe_strtok(&token,NULL,"() \n");
		node->df = atof(token);

		safe_strtok(&token,NULL,"() \n");
		node->ph = atof(token);

		node->pulse_type = PULSE_SIN;
    }

    /*check for PULSE transient spec*/
    else if(strcmp(token,"PULSE") == 0 || strcmp(token,"PULSE") == 0){

		safe_strtok(&token,NULL,"() \n");
		node->i1 = atof(token);

		safe_strtok(&token,NULL,"() \n");
		node->i2 = atof(token);

		safe_strtok(&token,NULL,"() \n");
		node->td = atof(token);

		safe_strtok(&token,NULL,"() \n");
		node->tr = atof(token);

		safe_strtok(&token,NULL,"() \n");
		node->tf = atof(token);

		safe_strtok(&token,NULL,"() \n");
		node->pw = atof(token);

		safe_strtok(&token,NULL,"() \n");
		node->per = atof(token);

		node->pulse_type = PULSE_PULSE;

    }else if(strcmp(token,"PWL") == 0 || strcmp(token,"pwl")){
		PAIR_LIST* pair_list = create_pair_list();
		if(!pair_list){
			printf("Not enough memory for pair list...\n");
			return 0;
		}

		safe_strtok(&token,NULL,"() \n");

		while(token != NULL){
			double ti,ii;

			ti = atof(token);

			safe_strtok(&token,NULL,"() \n");
			ii = atof(token);
			add_to_pair_list(pair_list, ti, ii);

			safe_strtok(&token,NULL,"() \n");

		}
		node->pair_list = pair_list;

		node->pulse_type = PULSE_PWL;
	}

    node->is_ac = 1;
	return 1;
}


void read_mos_elements(char* token,GENERAL_NODE* node,LIST* list,long* node_count)
{
    /* read source */
    safe_strtok(&token,NULL," ");
    node->source = read_node(list,token,node_count);

    /* read body */
    safe_strtok(&token,NULL," ");
    node->body = read_node(list,token,node_count);

    /* read length */
    safe_strtok(&token,NULL," ");
    node->l = atof(token);

    /* read width */
    safe_strtok(&token,NULL," ");
    node->w = atof(token);
}

void read_bjt_elements(char* token,GENERAL_NODE* node,LIST* list,long* node_count)
{
    safe_strtok(&token,NULL," ");
    node->emitter = read_node(list,token,node_count);
}

void set_R_Node(LIST* list,GENERAL_NODE* node,RESISTANCE_T* resNode,int* type)
{
	strncpy(resNode->name,node->name,MAX_NAME_LENGTH);
	resNode->node1 = node->node1;
	resNode->node2 = node->node2;
	resNode->value = node->value;

	*type = NODE_RESISTANCE_TYPE;

	if(!resNode->node1)
		list->non_zero_elements++;
	else if(!resNode->node2)
		list->non_zero_elements++;
	else
		list->non_zero_elements += 4;


}

void set_C_Node(GENERAL_NODE* node,CAPACITY_T* resNode,int* type)
{
	strncpy(resNode->name,node->name,MAX_NAME_LENGTH);
	resNode->node1 = node->node1;
	resNode->node2 = node->node2;
	resNode->value = node->value;

	*type = NODE_CAPACITY_TYPE;

}

void set_L_Node(LIST* list,GENERAL_NODE* node,INDUCTANCE_T* resNode,int* type)
{
	strncpy(resNode->name,node->name,MAX_NAME_LENGTH);
	resNode->node1 = node->node1;
	resNode->node2 = node->node2;
	resNode->value = node->value;

	*type = NODE_INDUCTANCE_TYPE;

	if(!resNode->node1)
		list->non_zero_elements += 2;
	if(!resNode->node2)
		list->non_zero_elements += 2;
}

void set_M_Node(GENERAL_NODE* node,MOSFET_T* resNode,int* type)
{
	strncpy(resNode->name,node->name,MAX_NAME_LENGTH);
	resNode->drain 	= node->node1;
	resNode->gate 	= node->node2;

	resNode->source = node->source;
    resNode->body 	= node->body;
    resNode->l 		= node->l;
    resNode->w 		= node->w;

    *type = NODE_MOSFET_TYPE;
}

void set_Q_Node(GENERAL_NODE* node,BJT_T* resNode,int* type)
{
	strncpy(resNode->name,node->name,MAX_NAME_LENGTH);
	resNode->collector 	= node->node1;
	resNode->base 			= node->node2;
	resNode->emitter	= node->emitter;
}

void copy_transient_elements(void* resNode,GENERAL_NODE* node,int type)
{
	SOURCE_V_T* tempNode = (SOURCE_V_T*)resNode;

	if(type != NODE_SOURCE_V_TYPE)
		tempNode = (SOURCE_I_T*)resNode;

	tempNode->pulse_type 	= node->pulse_type;
	tempNode->is_ac 		= node->is_ac;

	/*values for transient spec, might need to add specific nodes for each type (EXP,SIN etc) to save memory*/
	tempNode->i1 			= node->i1;
	tempNode->i2			= node->i2;
	tempNode->td1			= node->td1;
	tempNode->td2			= node->td2;
	tempNode->tc1			= node->tc1;
	tempNode->tc2			= node->tc2;

	tempNode->ia			= node->ia;
	tempNode->fr			= node->fr;
	tempNode->td			= node->td;
	tempNode->df			= node->df;
	tempNode->ph			= node->ph;

	tempNode->tr			= node->tr;
	tempNode->tf			= node->tf;
	tempNode->pw			= node->pw;
	tempNode->per			= node->per;

	tempNode->pair_list  = node->pair_list;

}

void set_V_Node(LIST* list,GENERAL_NODE* node,SOURCE_V_T* resNode,int* type)
{
	strncpy(resNode->name,node->name,MAX_NAME_LENGTH);
	resNode->node1 = node->node1;
	resNode->node2 = node->node2;
	resNode->value = node->value;

	copy_transient_elements((void*)resNode,node,NODE_SOURCE_V_TYPE);

	*type = NODE_SOURCE_V_TYPE;

	if(!resNode->node1)
		list->non_zero_elements += 2;
	if(!resNode->node2)
		list->non_zero_elements += 2;

}

void set_I_Node(GENERAL_NODE* node,SOURCE_I_T* resNode,int* type)
{
	strncpy(resNode->name,node->name,MAX_NAME_LENGTH);
	resNode->node1 = node->node1;
	resNode->node2 = node->node2;
	resNode->value = node->value;

	copy_transient_elements((void*)resNode,node,NODE_SOURCE_I_TYPE);

	*type = NODE_SOURCE_I_TYPE;
}

/* 
 * Proccess a single line
 *
 * 1) Identify circuit element
 * 2) Check for errors
 * 3) Build circuit node
 *
 * Retuns: 1 when a node was identified correctly.Variables -node-  and -type- contain a circuit node
 *         0 when a parsing error occurs.Variables -node- and -type- values are not predicted
 */
static int get_node_from_line( LIST* list,char* line , NODE* node , int* type){
	char c;
	long node_count = 1;

	ht_insert_pair(list->hashtable, "0" , 0);

	if( line == NULL || node == NULL  || type == NULL )
	return 0;

	c = line[0];
	GENERAL_NODE* temp_node = (GENERAL_NODE*)malloc(sizeof(GENERAL_NODE));

	//printf("%s \n",line);
	if(c == '.')
		return 1;
	node_count = parse_line(temp_node,line,list,c);

	switch(c){
		case 'r':
    	case 'R': { 	set_R_Node(list,temp_node,&node->resistance,type);	break;	}
    	case 'c':
    	case 'C': { 	set_C_Node(temp_node,&node->capacity,type); 		break; 	}
    	case 'l':
    	case 'L': {		set_L_Node(list,temp_node,&node->inductance,type); 	break; 	}
    	case 'v':
		case 'V': {		set_V_Node(list,temp_node,&node->source_v,type);	break;	}
		case 'i':
		case 'I': {		set_I_Node(temp_node,&node->source_i,type);			break;	}
		case 'q':
		case 'Q': {		set_Q_Node(temp_node,&node->bjt,type);				break;	}
		case 'm':
		case 'M': {		set_M_Node(temp_node,&node->mosfet,type);			break;	}
		case '.': {		break;														}
	}
	return 1;
}

#if 0
    /*
     * Commands
     */
    case '.':{

      char* temp = line;
      token = strtok(temp," ");
        
      if( !token )
        return 0;

      /* solving method */
      if( strcmp(token,".OPTIONS") == 0 || strcmp(token,".options") == 0 ){

        token = strtok(NULL," \n");
        //printf("Token after .options :%s\n",token);
        if( !token ){
          printf("Error while parsing...\n");
          printf("Must define an option \n");
          printf("Line : %s\n", line );
          return 0;
        }
        if( strcmp(token,"SPARSE") == 0 || strcmp(token,"sparse") == 0 ){
          list->solving_method = METHOD_LU_SPARSE;
          list->sparse = 1;
          return 2;
        }
        //printf("TOKEN AFTER .OPTIONS :%s\n",token);
        else if( strcmp(token,"SPD") == 0 || strcmp(token,"spd") == 0 ){
          token = strtok(NULL," \n");
          if(!token){
            list->solving_method = METHOD_CHOLESKY;
            return 2;
          }

          /*------  .OPTIONS SPD ITER----------------*/
          if( strcmp(token,"ITER") == 0 || strcmp(token,"ITER") == 0){
            list->solving_method = METHOD_CG;
            return 2;
          }

          /*------------ .OPTIONS SPD SPARSE--------------*/
          else if (strcmp(token,"SPARSE") == 0 || strcmp(token,"sparse") == 0){
            /* cholesky solution with sparce arrays   */
            list->solving_method = METHOD_CHOLESKY_SPARSE;
            list->sparse=1;
          }
        }
        else if( strcmp(token,"ITER") == 0 || strcmp(token,"iter") == 0 ){
          token = strtok(NULL," \n");
          if( !token ){         
            list->solving_method = METHOD_BICG;
            return 2;
          }
            
          /*----------- .OPTIONS ITER SPARSE ---------------*/
          if( strcmp(token,"SPARSE") == 0 || strcmp(token,"sparse") == 0){
            /*   Bi-CG with sparce arrays  */
            list->solving_method = METHOD_BICG_SPARSE;
            list->sparse=1;
          }
          /*----------- .OPTIONS ITER SPD || ITER SPD SPARCE-------*/
          else if( strcmp(token, "SPD") == 0 || strcmp(token, "spd") == 0){
            token = strtok(NULL," \n");
            if( !token ){         
              list->solving_method = METHOD_CG;
              return 2;
            }
            else if ( strcmp(token,"SPARSE") == 0 || strcmp(token,"sparse") == 0){
              /*   CG solving method with sparce arrays   */
              list->solving_method = METHOD_CG_SPARSE;
              list->sparse=1;
            }
          }
          
        }
        else if( strcmp(token,"ITOL") == 0 || strcmp(token,"itol") == 0){
          /* tolerance */
          token = strtok(NULL," \n");
          if ( !token ){
            printf("Error while parsing...\n");
            printf("Line: %s\n",line);
            return 0;
          }


          if( strcmp(token,"=") ==  0 ){
            /* now read the tolerance value */
            token = strtok(NULL," \n");

            if( !token ){
              printf("Error while parsing..\n");
              printf("Line: %s\n",line);
              return 0;

            }
            else{
              double val;
              val = atof(token);
            
              list->itol = val;
              return 2;
            }  
          }
          else{
            printf("Error while parsing...\n");
            printf("Line: %s\n",line);
            return 0;
          }

        }
        else if(strcmp(token,"SPARCE") == 0 || strcmp(token,"sparce") == 0){
          /*LU with sparce arrays*/
          list->solving_method = METHOD_LU_SPARSE;
          list->sparse=1;
        }
        else if(strcmp(token,"METHOD=TR") == 0 || strcmp(token,"method=tr") == 0){
          /*Trapezoidal Transient Method*/
          list->transient_sim = METHOD_TR; 
        }
        else if(strcmp(token,"METHOD=BE") == 0 || strcmp(token,"method=be") == 0){
          /*Trapezoidal Transient Method*/
          list->transient_sim = METHOD_BE; 
        }


        
        else{
          printf("No token after .OPTIONS \n");
          return 0;
        }

        //return 2;
      }
      else if(strcmp(token,".TRAN") == 0 || strcmp(token,".tran") == 0){
        /*read the time step*/
        token = strtok(NULL," \n");
        if( !token ){
              printf("Error while parsing TRAN command, please define time step...\n");
              printf("Line: %s\n",line);
              return 0;
        }
        list->time_step = atof( token );
        

        /*read the finish time*/
        token = strtok(NULL," \n");
        if( !token ){
              printf("Error while parsing TRAN command, please define finish time...\n");
              printf("Line: %s\n",line);
              return 0;
        }
        list->fin_time = atof( token );
        

      }
      else if( strcmp(token,".DC") == 0 || strcmp(token,".dc") == 0 ){  // check for .DC

/*
        token = strtok(temp," ");
        if( !token ){
          printf("Error while parsing...\n");
          printf("Line : %s\n", line );
          return 0;
        }
*/
        token = strtok(NULL , " ");
        if( !token ){
        //  list->solving_method = METHOD_LU;

          //printf("Error while parsing...\n");
          //printf("Line : %s\n", line );
          return 2;
        }
        list->dc_sweep.name = strdup(token);
        //printf("DC: name = %s \n",list->dc_sweep.name);

        token = strtok(NULL , " ");
        if( !token ){
          printf("Error while parsing...\n");
          printf("Line : %s\n", line );
          return 0;
        }
        list->dc_sweep.start_v = atof( token );

        token = strtok(NULL , " ");
        if( !token ){
          printf("Error while parsing...\n");
          printf("Line : %s\n", line );
          return 0;
        }
        list->dc_sweep.end_v = atof(token);

        token = strtok(NULL , " ");
        if( !token ){
          printf("Error while parsing...\n");
          printf("Line : %s\n", line );
          return 0;
        }
        list->dc_sweep.inc = atof(token);


        // check if node already declared 
        list->dc_sweep.node = list_search_by_name(list , list->dc_sweep.name );
        if( !(list->dc_sweep.node)){
          printf(".DC Error: %s element not found\n",list->dc_sweep.name);
          exit(1);
        }
        list->dc_sweep.oldval = list->dc_sweep.node->node.source_v.value ; 

      }
      else if( strcmp(token,".PLOT") == 0 || strcmp(token,".plot") == 0 ){
    	  int plot_num=0;
    	  //reading the PLOT command keyword

    	  plot_init();
    	  list->plot = PLOT_ON;
          while(1){
        	  //reading the source type for plotting
        	  token = strtok(NULL," (\n");
        	  if( !token && plot_num == 0 ){
        		  printf("Error while parsing...\n");
        		  printf("Line : %s\n", line );
        		  return 0;
        	  }
        	  else if(!token && plot_num>0)
        		  break;
          

        	  //reading the node name
        	  token = strtok(NULL,")");
        	  printf("Going to plot results for node: %s \n", token);
        	  if( !token ){
        		  printf("Error while parsing...\n");
        		  printf("Line : %s\n", line );
        		  return 0;
        	  }
        	  plot_add_node(token);
        	  plot_num++;
          }
      }
    } 
  }
  //printf("Total node count: %ld \n",node_count);
  //printf("Hash_table nodes: %d\n",list->hashtable->num_nodes);

  return 2;
}
#endif

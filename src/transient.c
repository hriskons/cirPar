#include "transient.h"

#define PI (3.141592653589793)

double calc_trans_sin(double i1,double ia,double fr,double td,double df,double ph,double t);
double calc_trans_pwl(PAIR_LIST* list, double t);
double calc_trans_exp(double i1,double i2,double td1,double tc1,double td2,double tc2,double t);
double calc_trans_pulse(double i1,double i2,double td,double tr,double tf,double pw,double per,double t);


static void print_vector_gsl(gsl_vector* A)
{
	int i;

	printf("Going to print gsl_vector \n");
	for(i = 0; i < A->size; i++)
	{
		printf("\t%.4f\n",gsl_vector_get(A,i));

	}
}

static void double_vector_to_gsl_vector(gsl_vector* gsl_v,double *doubl_vec,int size_n)
{
	int i;
	for (i = 0; i < size_n; i++)
	{
		gsl_vector_set(gsl_v,i,doubl_vec[i]);
	}
}

static void print_matrix_gsl(gsl_matrix* A)
{
	int i,j;

	printf("Going to print gsl_matrix \n");
	for(i = 0; i < A->size1; i++)
	{
		for(j = 0; j < A->size2; j++)
		{
			printf("%.4f  ",gsl_matrix_get(A,i,j));
		}
		printf("\n");
	}
}

static gsl_vector* calc_right_hand_vect(LIST* list ,gsl_vector *vector, double curr_time)
{
	LIST_NODE* curr;
	gsl_vector* tmp_vector;

	tmp_vector = gsl_vector_calloc( vector->size );
 	if( !tmp_vector )
 		return 0;	

	for( curr = list->head ; curr ; curr = curr->next){
		if( curr->type == NODE_SOURCE_I_TYPE ){
 			/* change only the vector */
 			double current = curr->node.source_i.value;
 			double value;
 			double ac_current = 0;

 			if(curr->node.source_i.is_ac)
 			{
 				//printf("Node with name %s is ac\n",curr->node.source_i.name);
 				if(curr->node.source_i.pulse_type == PULSE_EXP)
 					ac_current = calc_trans_exp(curr->node.source_i.i1,curr->node.source_i.i2,curr->node.source_i.td1,curr->node.source_i.tc1,curr->node.source_i.td2,curr->node.source_i.tc2,curr_time);
	 			else if(curr->node.source_i.pulse_type == PULSE_SIN)
	 				ac_current = calc_trans_sin(curr->node.source_i.i1,curr->node.source_i.ia,curr->node.source_i.fr,curr->node.source_i.td,curr->node.source_i.df,curr->node.source_i.ph,curr_time);
	 			else if(curr->node.source_i.pulse_type == PULSE_PWL)
	 				ac_current = calc_trans_pwl(curr->node.source_i.pair_list, curr_time);
	 			else if(curr->node.source_i.pulse_type == PULSE_PULSE)
	 				ac_current = calc_trans_pulse(curr->node.source_i.i1,curr->node.source_i.i2,curr->node.source_i.td,curr->node.source_i.tr,curr->node.source_i.tf,curr->node.source_i.pw,curr->node.source_i.per,curr_time);
	 			else
	 				return NULL;

	 			//printf("AC value: %lf\n",ac_current);

 				current += ac_current;
 			}
 			

 			if( curr->node.source_i.node1 != 0 ){
 				/* ste <+> */
 				value  = gsl_vector_get(tmp_vector , curr->node.source_i.node1 - 1  );
 				value -= current;
 				gsl_vector_set(tmp_vector , curr->node.source_i.node1 -1  , value );
 			}

 			if( curr->node.source_i.node2 != 0 ){
 				/* <-> */
 				value  = gsl_vector_get(tmp_vector , curr->node.source_i.node2 - 1 );
 				value += current;
 				gsl_vector_set(tmp_vector , curr->node.source_i.node2 - 1 , value);
 			}
 		}else if ( curr->type == NODE_SOURCE_V_TYPE  ){
 			int matrix_row = curr->node.source_v.mna_row; 			
 			double value;

			double ac_voltage = 0;

			if(curr->node.source_v.is_ac)
 			{
 				//printf("Node with name %s is ac\n",curr->node.source_v.name);
	 			if(curr->node.source_v.pulse_type == PULSE_EXP)
	 				ac_voltage = calc_trans_exp(curr->node.source_v.i1,curr->node.source_v.i2,curr->node.source_v.td1,curr->node.source_v.tc1,curr->node.source_v.td2,curr->node.source_v.tc2,curr_time);
	 			else if(curr->node.source_v.pulse_type == PULSE_SIN)
	 				ac_voltage = calc_trans_sin(curr->node.source_v.i1,curr->node.source_v.ia,curr->node.source_v.fr,curr->node.source_v.td,curr->node.source_v.df,curr->node.source_v.ph,curr_time);
	 			else if(curr->node.source_v.pulse_type == PULSE_PWL)
	 				ac_voltage = calc_trans_pwl(curr->node.source_v.pair_list, curr_time);
	 			else if(curr->node.source_v.pulse_type == PULSE_PULSE)
	 				ac_voltage = calc_trans_pulse(curr->node.source_v.i1,curr->node.source_v.i2,curr->node.source_v.td,curr->node.source_v.tr,curr->node.source_v.tf,curr->node.source_v.pw,curr->node.source_v.per,curr_time);
	 			else
	 				return NULL;

	 			//printf("AC value: %lf\n",ac_voltage);
	 		}

 			/* set vector value */
 			value = gsl_vector_get(tmp_vector , matrix_row );
 			value += curr->node.source_v.value + ac_voltage;
 			gsl_vector_set(tmp_vector, matrix_row , value);

 		}else
 		{
 			perror("Not a valid element\n");
			return NULL;
		}
	}
	return tmp_vector;
}


int transient_simulation(LIST* list, gsl_matrix *matrix , gsl_matrix *c_matrix , gsl_vector *vector , gsl_vector *x, gsl_permutation* permutation)
{
	LIST* v_i_list;
	double fin_time = list->fin_time;
	double curr_time = 0;
	double time_step = list->time_step;

	//time_step = 0.1;
	int flag;
	gsl_vector *prev_vector;
	gsl_vector *temp_vector;
	gsl_vector *e_t;
	gsl_vector *e_t_minus_one;

	gsl_vector *prev_x;

	gsl_matrix *right_matrix;
	gsl_matrix *tmp_matrix;
	gsl_matrix *curr_matrix;
	int i;

//  sparse simulation
	int vector_size;
	char method;
	int sign;

	sparse_matrix* sp_matrix;
	sparse_vector* sp_vector;
	sparse_vector* sp_x;
	gsl_vector* x_sparse;
	gsl_vector* vector_sparse;
	gsl_vector* vector_gsl;

 		
	if(!list->sparse )
	{
		flag = create_mna(list, &matrix, &vector, 1 ,&c_matrix);
		//print_matrix_gsl(matrix);

		if(!flag ){
			printf("Error creating mna system\n");
			return -1;
		}

	 	v_i_list = create_source_list(list);


		tmp_matrix = gsl_matrix_calloc(matrix->size1 , matrix->size1);
		curr_matrix = gsl_matrix_calloc(matrix->size1 , matrix->size1);
		right_matrix = gsl_matrix_calloc(matrix->size1,matrix->size2);

 		if( !tmp_matrix || !right_matrix)
 			return 0;
 		
		gsl_matrix_memcpy(right_matrix,matrix);
		gsl_matrix_memcpy(tmp_matrix,matrix);
		gsl_matrix_memcpy(curr_matrix,matrix);


		x = gsl_vector_calloc(matrix->size1);
 		if( !x ){
 			printf("X vector : no memory\n");
 			exit(1);
 		}

		if(list->transient_sim == METHOD_TR)
		{
			gsl_matrix_scale(c_matrix, (2/time_step));
			gsl_matrix_add(tmp_matrix,c_matrix);

			gsl_matrix_sub(right_matrix,c_matrix);

		}else
		{
			gsl_matrix_scale (c_matrix, 1/time_step);
			gsl_matrix_add(tmp_matrix,c_matrix);

			gsl_matrix_memcpy(right_matrix , c_matrix);
		}


		x = gsl_vector_calloc(matrix->size1);
		if( !x ){
			printf("X vector : no memory\n");
			exit(1);
		}
	}else
	{
		method = list->solving_method;
 		
 		sp_matrix = (sparse_matrix*)create_mna_sparse( list , &sp_vector , &vector_size);
 		if( !sp_matrix ){
 			fprintf(stderr, "Error creating MNA matrix \n");
 			exit(1);
 		}
 		
 		/* conversion of a double into a gsl */
 		x_sparse = gsl_vector_calloc(sp_matrix->n);
		vector_sparse = gsl_vector_calloc(sp_matrix->n);
 		double_vector_to_gsl_vector(vector_sparse,sp_vector,vector_size);

 		sp_x = (sparse_vector*) malloc( vector_size * sizeof(sparse_vector));

	}
	/*
	printf("matrix:\n");
	print_matrix_gsl(matrix);
	printf("Tmp Matrix:\n");
	print_matrix_gsl(tmp_matrix);
	printf("right_matrix:\n");
	print_matrix_gsl(right_matrix);
	printf("c_matrix:\n");
	print_matrix_gsl(c_matrix);
	*/
	
	prev_vector = gsl_vector_calloc(vector->size);
	temp_vector = gsl_vector_calloc(vector->size);
	
	if( !prev_vector || !temp_vector){
	  printf("Couldn't allocate memory for prev_vector & temp_vector");
	  return 0;
	}

	prev_x = gsl_vector_calloc(x->size);
	
	if( !prev_x ){
	  printf("Couldn't allocate memory for prev_x");
	  return 0;
	}
	
	e_t_minus_one = gsl_vector_calloc(vector->size);
	e_t = gsl_vector_calloc(vector->size);
	
	if( !e_t || !e_t_minus_one){
	    printf("Couldn't allocate memory for e(t) & e(t-1)");
	    return 0;
	}
	  


	printf("TRANSIENT: Solving Method = %d\n",list->solving_method);
	printf("fin_time: %lf\n",fin_time);

	for(curr_time = (-1)*time_step; curr_time <= fin_time; curr_time += time_step)
	{

		if(curr_time == 0)
		{
			gsl_vector_memcpy(e_t_minus_one,vector);		//dc initialize
			gsl_matrix_memcpy(matrix,tmp_matrix);
		}
		if(curr_time >= 0)
		{
			// x(t-1)
			gsl_vector_memcpy(prev_x,x);
			
			//print_vector_gsl(temp_vector);
			
			// right_matrix * x(t-1)
			lh_matrix_vector_mul( prev_x, right_matrix , temp_vector , NON_TRANSP);
			
			//print_vector_gsl(temp_vector);
			
			// calculate e(t)
			e_t = calc_right_hand_vect(v_i_list , e_t_minus_one ,curr_time);

			/*gsl_vector_memcpy(prev_vector,e_t);
			gsl_vector_add(prev_vector,e_t_minus_one);
			printf("e(t) + e(t-1) = \n");
			print_vector_gsl(prev_vector); */
			
			// get ready for next math operations
			gsl_vector_memcpy(prev_vector,e_t_minus_one);
			gsl_vector_memcpy(vector,e_t);
			
			if(list->transient_sim == METHOD_TR)
			{
				// e(t-1) - right_matrix*x(t-1)
				gsl_vector_sub(prev_vector, temp_vector);
				
				
				// e(t) + e(t-1) - right_matrix*x(t-1)
				gsl_vector_add(vector,prev_vector);

				//printf("b = \n");
				//print_vector_gsl(vector);
				
				// e_t_minus_one = e_t for the next iteration
				gsl_vector_memcpy(e_t_minus_one,e_t);

			}			

			LIST_NODE* curr;

		}
		
		for(i = 0; i < x->size; i++)
			gsl_vector_set(x,i,0);

		
	 	if ( !list->sparse ){

			/* Cholesky or LU */
			if( list->solving_method == METHOD_LU || list->solving_method == METHOD_CHOLESKY ){

	 			gsl_matrix_memcpy(curr_matrix , matrix);

	 			decomposition(matrix,&permutation,&sign,list->solving_method);
				

	 			if(list->dc_sweep.node != NULL)
	 			{
	 	 			dc_sweep(*list,matrix,vector,x,permutation,list->solving_method);
	 			}
	 			else
	 			{
	 				int array_size = 1;

	 				solve(matrix,vector,x,permutation,list->solving_method);
	 				//print_vector_gsl(x);

	 				if(list->plot == PLOT_ON)
					{
	 					gsl_vector ** plot_array;

						plot_array = plot_create_vector( array_size , x->size);
						if(plot_array == NULL)
						{
							perror("Error while allocating the ploting array\n");
							exit(0);
						}
		 		
						plot_set_vector_index(plot_array ,x ,0);
				 		if ( list->solving_method == METHOD_LU )
							plot_to_file(list->hashtable,plot_array,array_size,"results_plot_file_lu.txt");
						else
							plot_to_file(list->hashtable,plot_array,array_size,"results_plot_file_chol.txt");
					}
	 			}
	 			gsl_matrix_memcpy(matrix , curr_matrix);

			}
			else if ( list->solving_method == METHOD_CG ){

				if( list->dc_sweep.node != NULL ){
					dc_sweep(*list,matrix,vector,x,permutation,list->solving_method);
				}
				else {
					iter_solve_cg( matrix , vector , x);


					gsl_vector ** plot_array;

					plot_array = plot_create_vector( 1 , x->size);
					if(plot_array == NULL)
					{
						perror("Error while allocating the ploting array\n");
						exit(0);
					}
		 		
					plot_set_vector_index(plot_array ,x ,0);
				 		 	
					plot_to_file(list->hashtable,plot_array,1  ,"results_plot_file_cg.txt");
				}
			}
			else if( list->solving_method == METHOD_BICG){
				
				if( list->dc_sweep.node != NULL ){
					dc_sweep(*list,matrix,vector,x,permutation,list->solving_method);
				}
				else {
					iter_solve_bicg( matrix , vector , x);


					gsl_vector ** plot_array;

					plot_array = plot_create_vector( 1 , x->size);
					if(plot_array == NULL)
					{
						perror("Error while allocating the ploting array\n");
						exit(0);
					}
		 		
					plot_set_vector_index(plot_array ,x ,0);
				 		 	
					plot_to_file(list->hashtable,plot_array,1  ,"results_plot_file_bicg.txt");
				}

			}
	 	}
	 	else {

	 		if( method == METHOD_LU_SPARSE ){
	 			if( !sparse_solve_LU( sp_matrix,sp_vector,sp_x,vector_size) ){
	 				fprintf(stderr, "Solving Method Sparse LU failed\n" );
	 			}

	 			gsl_vector ** plot_array;

				plot_array = plot_create_vector( 1 , vector_size);
				if(plot_array == NULL)
				{
					perror("Error while allocating the ploting array\n");
					exit(0);
				}
		 		vector_gsl = gsl_vector_calloc(vector_size);
	 			double_vector_to_gsl_vector(vector_gsl,sp_x,vector_size);

				plot_set_vector_index(plot_array ,vector_gsl ,0);
				 		 	
				plot_to_file(list->hashtable,plot_array,1  ,"results_plot_file_lu_sparse.txt");


	 		}
	 		else if( method == METHOD_CHOLESKY_SPARSE ){
				if( !sparse_solve_cholesky( sp_matrix,sp_vector,sp_x,vector_size) ){
	 				fprintf(stderr, "Solving Method Sparse Cholesky failed\n" );
	 			}

	 		}	
	 		else if ( method == METHOD_CG_SPARSE ){
	 			if( !sparse_solve_cg( sp_matrix,vector_sparse,x_sparse) ){
	 				fprintf(stderr, "Solving Method Sparse CG failed\n" );
	 			}
	 			gsl_vector ** plot_array;

				plot_array = plot_create_vector( 1 , x_sparse->size);
				if(plot_array == NULL)
				{
					perror("Error while allocating the ploting array\n");
					exit(0);
				}
		 		
				plot_set_vector_index(plot_array ,x_sparse ,0);
				 		 	
				plot_to_file(list->hashtable,plot_array,1  ,"results_plot_file_sparse_cg.txt");
	 		}
	 		else if( method == METHOD_BICG_SPARSE ){
				if( !sparse_solve_bicg( sp_matrix, vector_sparse, x_sparse) ){
	 				fprintf(stderr, "Solving Method Sparse BiCG failed\n" );
	 			}
	 			gsl_vector ** plot_array;

				plot_array = plot_create_vector( 1 , x_sparse->size);
				if(plot_array == NULL)
				{
					perror("Error while allocating the ploting array\n");
					exit(0);
				}
		 		
				plot_set_vector_index(plot_array ,x_sparse ,0);
				 		 	
				plot_to_file(list->hashtable,plot_array,1  ,"results_plot_file_sparse_bicg.txt");
	 

	 		}
	 		else{
	 			fprintf(stderr, "Solving method not specified\n");
	 		}
	 	}
	}
	print_vector_gsl(x);
	/* clean up before exit */
	if(list->sparse)
		cs_spfree(sp_matrix);
	free(vector);
	free(x);
	return 1;

}


double calc_trans_exp(double i1,double i2,double td1,double tc1,double td2,double tc2,double t){

	if( t >= 0 && t <= td1){
		return i1;
	}
	else if (t >= td1 && t <= td2){
		double temp = -(t - td1)/tc1;

		return (i1 + (i2-i1)*(1-exp(temp)) );
	}
	else{
		double temp1 = -(t - td2)/tc2;
		double temp2 = -(t - td1)/tc1;
		
		return (i1 + (i2 - i1)*(exp(temp1)- exp(temp2)) );
	}
}


double calc_trans_sin(double i1,double ia,double fr,double td,double df,double ph,double t){

	if (t >= 0 && t <= td){
		double temp = 2*PI*ph/360;

		return (i1 +ia*sin(temp) );
	}
	else{
		double temp1 = 2*PI*fr*(t - td) + 2*PI*ph/360;
		double temp2 = -(t - td)*df;

		return (i1 + i1*sin(temp1)*exp(temp2) );
	}

}


double calc_trans_pulse(double i1,double i2,double td,double tr,double tf,double pw,double per,double t){

	int k = (int)floor( t / per );

	double lim1 = td;
	double lim2 = td + k*per;
	double lim3 = td + tr + k*per;
	double lim4 = td + tr + pw + k*per;
	double lim5 = td + tr + pw + tf + k*per;

	double slope,b;

	if( t >= 0 && t <= lim1){
		return i1;
	}
	else if(t >=  lim2 && t <= lim3){
		slope = (i2 - i1) / (lim3 - lim2);
		b = i1 - slope * lim2;
		return (slope * t + b);
	}
	else if(lim3 <= t && t <= lim4){
		return i2;
	}
	else if(lim4 <= t && t <= lim5){
		slope = (i2 - i1) / (lim4 - lim5);
		b = i1 - slope * lim5;
		return (slope * t + b);
	}
	else{
		return i1;
	}
}


double calc_trans_pwl(PAIR_LIST* list, double t)
{
	int num_of_pairs = list->n;
	PWL_PAIR_T* curr = list->head;
	double slope,b;
	int i = 0;

	while( i < num_of_pairs)
	{
		double t1 = curr->ti;
		double t2 = curr->next->ti;
		if(t >= t1 && t <= t2)
		{
			double i1 = curr->ii;
			double i2 = curr->next->ii;

			slope = (i2 - i1) / (t2 - t1);
			b = i1 - slope * t1;
			return (slope*t + b);
		}
		curr = curr->next;
		i++;
	}
	return -1;
}

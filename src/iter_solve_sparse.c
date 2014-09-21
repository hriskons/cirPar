#include "sparse_interface.h"
#include "iter_solve_sparse.h"

#include <math.h>
#include "solvers.h"

#define ABS(value)  ( (value) >=0 ? (value) : -(value) )

#define EPS 1e-14

static int iter = ITER_NUM;
static double tolerance = ITER_TOL;

static void print_vector_gsl(gsl_vector* A)
{
	int i;

	for(i = 0; i < A->size; i++)
	{
		printf("\t%.4f\n",gsl_vector_get(A,i));

	}
}
static void print_matrix_gsl(gsl_matrix* A)
{
	int i,j;

	for(i = 0; i < A->size1; i++)
	{
		for(j = 0; j < A->size2; j++)
		{
			printf("\t%.4f\t",gsl_matrix_get(A,i,j));
		}
		printf("\n");
	}
}

void  safe_gsl_vector_alloc(gsl_vector** v, const size_t n)
{

	*v = gsl_vector_alloc(n);
	if(*v != NULL)
		return;
	else{
		fprintf(stderr, "Failed to allocate memory on gsl_vector_alloc\n. Exiting now\n");
		exit (0);
	}
}

void  safe_gsl_vector_calloc(gsl_vector** v, const size_t n)
{

	*v = gsl_vector_calloc(n);
	if(*v != NULL)
		return;
	else{
		fprintf(stderr, "Failed to allocate memory on gsl_vector_alloc\n. Exiting now\n");
		exit (0);
	}
}



gsl_vector* sparse_solve_cg(sparse_matrix* A , gsl_vector* b , gsl_vector* x0){

	int iteration = 0 ;
	gsl_vector *r, *b1, *z, *p, *q, *M;
	gsl_vector *temp_v;

	sparse_vector *q_vec,*p_vec;
	double rho,rho1;
	double beta;
	double alpha;

	safe_gsl_vector_alloc(&r,b->size);

	safe_gsl_vector_alloc(&b1,b->size);

	safe_gsl_vector_alloc(&z,b->size);

	safe_gsl_vector_alloc(&p,b->size);

	safe_gsl_vector_alloc(&q,b->size);

	q_vec = (sparse_vector *)safe_malloc(sizeof(double)*q->size);
	p_vec = (sparse_vector *)safe_malloc(sizeof(double)*p->size);
	safe_gsl_vector_calloc(&temp_v,p->size);


	M = lh_get_inv_diag_sparse(A);
	if( !M ){
		perror("Problem occured while allocating memory\n");
		return NULL;
	}


	gsl_vector_memcpy(b1,b);

	/* r = b - Ax */
	lh_matrix_vector_mul_and_sum_sparse( x0,A,b1,NON_TRANSP,-1.0,1.0);

	gsl_vector_memcpy(r , b1);
	gsl_vector_free(b1);



	while ( iteration < iter && ((lh_norm(r) / lh_norm(b)) > 1e-3 ) ){
		if(!(iteration % 100))
			printf("Iteration: %d Tolerance: %f\n",iteration,tolerance);
		iteration++;

		lh_diag_mul(z,r,M);				// Solve Mz = r
		
		rho = lh_dot_product( r , z);

		if( iteration == 1 ){
			gsl_vector_memcpy(p,z); 	// p = z
		}
		else{
			beta = rho / rho1;

			//gsl_vector* temp_v = gsl_vector_calloc(p->size);

			/* p = z + beta*p */
			lh_scalar_vector_mul(p, beta,p); //  p = beta* p
			
			gsl_vector_add( p , z);			 //  p = z + p
			
		}

		rho1 = rho;

		/* q = Ap */
		//lh_matrix_vector_mul_sparse( p,A,q,NON_TRANSP);
		//gsl_vector_sub(q,q);
		//cs_gaxpy(A,gsl_vector_ptr(p,0),gsl_vector_ptr(q,0));

		gsl_vector_sub(q, q);

		lh_gslVector_to_pointerVector(q,q_vec);
		lh_gslVector_to_pointerVector(p,p_vec);

		cs_gaxpy(A, p_vec, q_vec);

		lh_pointerVector_to_gslVector(q_vec, q);

		alpha = rho / lh_dot_product( p , q);

		/* x = x + alpha * p */


		lh_scalar_vector_mul(temp_v , alpha , p); // temp_v = alha * p
		
		gsl_vector_add( x0 , temp_v);			  // x = x + temp_v

		/* r = r - alpha * q */
		lh_scalar_vector_mul( temp_v , alpha , q); // temp_v = alpha* p
		
		gsl_vector_sub(r,temp_v); // r = r - temp_v 
				   
	}
	printf("Solution approximated in %d recursions",iteration);
	
	/* clean up */
	gsl_vector_free(r);
	gsl_vector_free(z);
	gsl_vector_free(p);
	gsl_vector_free(M);
	gsl_vector_free(q);
	gsl_vector_free(temp_v);

	free(q_vec);
	free(p_vec);

	/* result is written in x0 */
	//printf("going to print x0\n");
	//print_vector_gsl(x0);
	return x0;
}

gsl_vector* sparse_solve_bicg(sparse_matrix* A , gsl_vector* b , gsl_vector* x0){

	int iteration=0;
	double normb;
	double alpha,beta,rho,rho1,omega;

	sparse_vector *q_vec, *q_t_vec;
	sparse_vector *p_vec, *p_t_vec;

	sparse_matrix* A_T;			// A transposed

	gsl_vector *r,*r_t;
	gsl_vector *M;				// M_t is M itself.
	gsl_vector *p,*p_t;
	gsl_vector *z,*z_t;
	gsl_vector *q,*q_t;
	gsl_vector *temp;


	safe_gsl_vector_calloc(&r,b->size);
	safe_gsl_vector_calloc(&r_t,b->size);

	//M = gsl_vector_calloc(b->size);
	M = lh_get_inv_diag_sparse(A);				// init m
	if( !M){
		gsl_vector_free(r);
		gsl_vector_free(r_t);

		return NULL;
	} 

	safe_gsl_vector_calloc(&p,b->size);
	safe_gsl_vector_calloc(&p_t,b->size);
	p_vec = (sparse_vector *)safe_malloc(b->size * sizeof(sparse_vector));
	p_t_vec = (sparse_vector *)safe_malloc(b->size * sizeof(sparse_vector));

	safe_gsl_vector_calloc(&z,b->size);
	safe_gsl_vector_calloc(&z_t,b->size);

	safe_gsl_vector_calloc(&q,b->size);
	safe_gsl_vector_calloc(&q_t,b->size);
	q_vec = (sparse_vector *)safe_malloc(b->size * sizeof(sparse_vector));
	q_t_vec = (sparse_vector *)safe_malloc(b->size * sizeof(sparse_vector));


	safe_gsl_vector_calloc(&temp,b->size);

	//A_T = cs_spalloc(A->m, A->n, A->nzmax, 1, 1);
	A_T = cs_transpose(A, 1);
	if( !A_T ){
		fprintf(stderr,"Error in bicg whilie trying to transpose the matrix.\n");
		return NULL;
	}
	
	gsl_vector_memcpy(temp,b);
	
	/* temp = b - Ax; */
	lh_matrix_vector_mul_and_sum_sparse( x0,A,temp,NON_TRANSP,-1.0,1.0);

	
	// r = temp & r_t = r
	gsl_vector_memcpy(r, temp);
	gsl_vector_memcpy(r_t, r);

	normb = lh_norm(b);
	if(normb < EPS)
		normb =1;

	while( iteration < 5*iter && ( (lh_norm(r) / normb) > tolerance )){

		iteration++;

		lh_diag_mul(z,r,M);			// Solve M * z = r
		lh_diag_mul(z_t,r_t,M);		// Solve M * z_t = r_t
	
		rho = lh_dot_product( r_t , z);

		if(ABS(rho) < EPS){
			printf("Algorith failed\n");
			exit(0);
		}


		if(iteration == 1){
			gsl_vector_memcpy(p , z); 	// p = z 
			gsl_vector_memcpy(p_t , z_t); 	// p_t = z_t 
		}
		else{
			beta = rho/rho1;

			lh_scalar_vector_mul(p, beta, p); //  p = beta* p
			gsl_vector_add( p , z);			 //  p = z + p

			lh_scalar_vector_mul(p_t, beta, p_t); //  p_t = beta * p_t
			gsl_vector_add(p_t, z_t);			 //  p_t = z_t + p_t

		}

		rho1 = rho;

		/* q = Ap */

		gsl_vector_sub(q, q);

		lh_gslVector_to_pointerVector(q,q_vec);
		lh_gslVector_to_pointerVector(p,p_vec);

		cs_gaxpy(A, p_vec, q_vec);
		lh_pointerVector_to_gslVector(q_vec, q);

		/* q_t = A_T*p_t */

		gsl_vector_sub(q_t, q_t);

		lh_gslVector_to_pointerVector(q_t,q_t_vec);
		lh_gslVector_to_pointerVector(p_t,p_t_vec);

		cs_gaxpy(A_T, p_t_vec, q_t_vec);
		lh_pointerVector_to_gslVector(q_t_vec, q_t);

		omega = lh_dot_product(p_t, q);
		if( ABS(omega) < EPS ){
			printf("Algorith failed\n");
			exit(0);
		}

		alpha = rho/omega;

		/* x = x + alpha*q; */

		lh_scalar_vector_mul(temp , alpha , p); // temp = alha * p
		gsl_vector_add( x0 , temp);			  // x = x + temp


		/* 
		r = r -alpha*q; 
		r_t = r_t - alpha*q_t;
		 */

		lh_scalar_vector_mul( temp , alpha , q); // temp = alpha* q
		gsl_vector_sub(r,temp);				   // r = r - temp

		lh_scalar_vector_mul( temp , alpha , q_t); // temp = alpha* q_t
		gsl_vector_sub(r_t,temp);				   // r_t = r_t - temp_v
	}

	// free allocated memory space

	gsl_vector_free(r);
	gsl_vector_free(r_t);
	gsl_vector_free(M);
	gsl_vector_free(p);
	gsl_vector_free(p_t);
	gsl_vector_free(z);
	gsl_vector_free(z_t);
	gsl_vector_free(q);
	gsl_vector_free(q_t);
	gsl_vector_free(temp);

	free(q_vec);
	free(q_t_vec);
	free(p_vec);
	free(p_t_vec);

	cs_free(A_T);

	//printf("going to print x0\n");
	//print_vector_gsl(x0);
	return x0;
}

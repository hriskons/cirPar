/*
 * solvers.c
 *
 *  Created on: Sep 18, 2014
 *      Author: christos
 */
#include "solvers.h"


int sparse_solve_LU(sparse_matrix* matrix, sparse_vector* b, sparse_vector* x, int n){

	if( n < 1 )
		return 0;

	memcpy(x,b ,n * sizeof(double));

	return cs_lusol(2 , matrix , x , 1.0 );

}
void conjugate_gradient_sparse(cs *A, double *b, int n, double *x, double itol)
{
	int i,j;
	int iter;
	double rho,rho1,alpha,beta,omega;

	double *r;
	double *z;
	double *q, *temp_q;
	double *p, *temp_p;
	double *res;
	double *precond;	//Preconditioner


	r = (double *)safe_malloc(n * sizeof(double));
	z = (double *)safe_malloc(n * sizeof(double));
	q = (double *)safe_malloc(n * sizeof(double));
	p = (double *)safe_malloc(n * sizeof(double));
	res = (double *)safe_malloc(n * sizeof(double));
	precond = (double *)safe_malloc(n * sizeof(double));
	temp_q = (double *)safe_malloc(n * sizeof(double));
	temp_p = (double *)safe_malloc(n * sizeof(double));

	for(i = 0; i < n; i++){
		precond[i] =  0;
		r[i] = 0;
		z[i] = 0;
		q[i] = 0;
		temp_q[i] = 0;
		p[i] =0;
		temp_p[i] = 0;
	}

	/* Preconditioner */
	double max;
	int pp;
	for(j = 0; j < n; ++j){
		for(pp = A->p[j], max = fabs(A->x[pp]); pp < A->p[j+1]; pp++)
			if(fabs(A->x[pp]) > max)					//vriskei to diagonio stoixeio
				max = fabs(A->x[pp]);
		precond[j] = 1/max;
	}

	cblas_dcopy (n, x, 1, res, 1);

	//r=b-Ax
	cblas_dcopy (n, b, 1, r, 1);
	memset(p, 0, n*sizeof(double));
	cs_gaxpy (A, x, p);
	for(i=0;i<n;i++){
 		r[i]=r[i]-p[i];

	}

	double r_norm = cblas_dnrm2 (n, r, 1);
	double b_norm = cblas_dnrm2 (n, b, 1);
	if(!b_norm)
		b_norm = 1;

	iter = 0;

	double resid;
	while((resid = r_norm/b_norm) > 1e-3 && iter < ITER_NUM )
	{
		if(!(iter % 100))
			printf("Iteration: %d %f\n",iter,resid);
		iter++;

		cblas_dcopy (n, r, 1, z, 1);				//gia na min allaksei o r

		for(i=0;i<n;i++){
 			z[i]=precond[i]*z[i];

		}

		rho = cblas_ddot (n, z, 1, r, 1);
		if (fpclassify(fabs(rho)) == FP_ZERO){
			printf("RHO aborting CG due to EPS...\n");
			exit(42);
		}

		if (iter == 1){
			cblas_dcopy (n, z, 1, p, 1);
		}
		else{
			beta = rho/rho1;

			//p = z + beta*p;
			cblas_dscal (n, beta, p, 1);	//rescale
			cblas_daxpy (n, 1, z, 1, p, 1);	//p = 1*z + p

		}
		rho1 = rho;

		//q = Ap
		memset(q, 0, n*sizeof(double));
		cs_gaxpy (A, p, q);

		omega = cblas_ddot (n, p, 1, q, 1);
		if (fpclassify(fabs(omega)) == FP_ZERO){
			printf("OMEGA aborting CG due to EPS...\n");
			exit(42);
		}

		alpha = rho/omega;

		//x = x + aplha*p;
		cblas_dcopy (n, p, 1, temp_p, 1);
		cblas_dscal (n, alpha, temp_p, 1);//rescale by alpha
		cblas_daxpy (n, 1, temp_p, 1, res, 1);// sum x = 1*x + temp_p

		//r = r - aplha*q;
		cblas_dcopy (n, q, 1, temp_q, 1);
		cblas_dscal (n, -alpha, temp_q, 1);//rescale by alpha
		cblas_daxpy (n, 1, temp_q, 1, r, 1);// sum r = 1*r - temp_p

		//next step
		r_norm = cblas_dnrm2 (n, r, 1);
	}
	printf("Solution approximated after %d iterations for tolerance %f\n",iter,resid);
	cblas_dcopy (n, res, 1, x, 1);

}

void solve(sparse_matrix* matrix, sparse_vector* vector, gsl_vector* x,int vector_size)
{
	//char method;
	gsl_vector* x_gsl;

	sparse_vector* x_sparse;
	sparse_vector* x_sol;
	gsl_vector* vector_gsl;
	int i;
	int itol_value=1e-3;

	x_sparse = (sparse_vector*)safe_malloc(vector_size * sizeof(double));

	conjugate_gradient_sparse(matrix, vector, vector_size, x_sparse,itol_value);
	print_vector_to_file(x_sparse,vector_size,"solution_cg_gary");


	/* Added by hriskons */
	/* conversion of a double into a gsl
	safe_gsl_vector_calloc(&x_gsl, vector_size);
	safe_gsl_vector_calloc(&vector_gsl, vector_size);
	sparse_to_gsl_vector(vector,vector_gsl,vector_size);


	if( !sparse_solve_cg( matrix,vector_gsl,x_gsl) ){
		fprintf(stderr, "Solving Method Sparse LU failed\n" );
	}
	print_gsl_vector_to_file(x_gsl,"solution_cg_hriskons"); */
}




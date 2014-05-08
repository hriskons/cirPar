#ifndef TRANSIENT_H
#define TRANSIENT_H

#include "nodes.h"
#include <math.h>

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_permutation.h>
#include <gsl/gsl_linalg.h>
#include "node_list.h"
#include "linear_helper.h"
#include "decomposition.h"
#include "circuit_sim.h"
#include "circuit_sim_sparse.h"
#include "plot.h"
#include "iter_solve_sparse.h"
#include "iter_solve.h"

int transient_simulation(LIST* list, gsl_matrix *matrix , gsl_matrix *c_matrix , gsl_vector *vector , gsl_vector *x, gsl_permutation* permutation);

double calc_trans_exp(double i1,double i2,double td1,double tc1,double td2,double tc2,double t);

double calc_trans_sin(double i1,double ia,double fr,double td,double df,double ph,double t);

double calc_trans_pulse(double i1,double i2,double td,double tr,double tf,double pw,double per,double t);

double calc_trans_pwl(PAIR_LIST* list, double t);

#endif
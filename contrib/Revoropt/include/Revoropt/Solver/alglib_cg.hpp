// @licstart revoropt
// This file is part of Revoropt, a library for the computation and 
// optimization of restricted Voronoi diagrams.
//
// Copyright (C) 2013 Vincent Nivoliers <vincent.nivoliers@univ-lyon1.fr>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
// @licend revoropt
#ifndef _REVOROPT_SOLVER_ALGLIB_CG_WRAPPER_H_
#define _REVOROPT_SOLVER_ALGLIB_CG_WRAPPER_H_

#include <libalglib/optimization.h>

namespace Revoropt {
namespace Solver {

/** Solver encapsulation **/

class AlgCG {

  public :

  /* Types */

  /** Solver parameters **/
  class SolverParams {

    public :

    SolverParams() : epsg(0), epsf(0), epsx(0), 
                     max_iterations(10) {}

    double epsg ;
    double epsf ;
    double epsx ;
    alglib::ae_int_t max_iterations ;
  } ;

  /** Iteration data **/ 
  
  typedef struct {
    const double* x ;      //current variable positions
    const unsigned int n ; //size of x and g
    const double fx ;      //objective function value
    const unsigned int k ; //current iteration count
  } IterData ;
  
  /** Objective function data **/

  typedef struct {
    const double* x ;      //current variable positions
    const unsigned int n ; //size of x and g
    double* g ;            //gradient
  } EvalData ;

  /* Solver interface */
  template<typename ObjFun>
  int solve( double* x, unsigned int size, 
             ObjFun* objfun, 
             unsigned int n_iter 
           ) {
    //setup
    iteration_ = 0 ;
    variables_.setcontent(size,x) ;
    objfun_ = objfun ;
    solver_param.max_iterations = n_iter ;

    //build solver and solve
    alglib::mincgcreate(variables_, state_) ;
    alglib::mincgsetcond( state_,
                          solver_param.epsg,
                          solver_param.epsf,
                          solver_param.epsx,
                          solver_param.max_iterations
                        ) ;
    solve_wrapper<ObjFun, void>(this) ;

    //copy the result back
    alglib::mincgreport report ;
    alglib::mincgresults(state_, variables_, report) ;
    std::copy( variables_.getcontent(), 
               variables_.getcontent() + variables_.length(),
               x
             ) ;
    return report.terminationtype ;
  }

  template<typename ObjFun, typename IterCallback>
  int solve( double* x, unsigned int size, 
             ObjFun* objfun, 
             unsigned int n_iter,
             IterCallback* callback
           ) {
    //setup
    iteration_ = 0 ;
    variables_.setcontent(size,x) ;
    objfun_ = objfun ;
    iter_callback_ = callback ;
    solver_param.max_iterations = n_iter ;

    //build solver and solve
    alglib::mincgcreate(variables_, state_) ;
    alglib::mincgsetcond( state_,
                          solver_param.epsg,
                          solver_param.epsf,
                          solver_param.epsx,
                          solver_param.max_iterations
                        ) ;
    alglib::mincgsetxrep(state_, true) ;
    solve_wrapper<ObjFun, IterCallback>(this) ;

    //copy the result back
    alglib::mincgreport report ;
    alglib::mincgresults(state_, variables_, report) ;
    std::copy( variables_.getcontent(), 
               variables_.getcontent() + variables_.length(),
               x
             ) ;
    return report.terminationtype ;
  }

  /* Solver parameters */

  SolverParams solver_param ;

  private :

  /* State */
  alglib::mincgstate state_ ;
  unsigned int iteration_ ;

  /* Variables */

  alglib::real_1d_array variables_ ;

  /* Callbacks */

  void* objfun_ ;
  void* iter_callback_ ;

  /* Functions to be passed to lbfgs */
  template<typename ObjFun, typename IterCallback>
  static void solve_wrapper( AlgCG* instance ) ;

  template<typename IterCallback>
  static void iter_wrapper( const alglib::real_1d_array& x,
                            double f,
                            void* instance
                          ) ;

  template<typename ObjFun>
  static void eval_wrapper( const alglib::real_1d_array& x,
                            double& f,
                            alglib::real_1d_array& grad,
                            void* instance
                          ) ;

} ;

template<typename ObjFun, typename IterCallback>
void AlgCG::solve_wrapper( AlgCG* instance ) {
  //solve
  return alglib::mincgoptimize( instance->state_, 
                                AlgCG::eval_wrapper<ObjFun>, 
                                AlgCG::iter_wrapper<IterCallback>, 
                                instance
                              ) ;
}

template<typename IterCallback>
void AlgCG::iter_wrapper( const alglib::real_1d_array& x,
                          double f,
                          void* instance
                        ) {
  //the solver context is provided as the instance
  AlgCG* s = (AlgCG*) instance ;

  //the iteration callback is a member
  IterCallback* callback = (IterCallback*) s->iter_callback_ ;

  //call it
  AlgCG::IterData data = { 
    x.getcontent(), 
    (unsigned int)x.length(), 
    f,
    ++s->iteration_
  } ;
  (*callback)(&data) ;
}

template<>
void AlgCG::iter_wrapper<void>( const alglib::real_1d_array& x,
                                double f,
                                void* instance
                              ) {
}

template<typename ObjFun>
void AlgCG::eval_wrapper( const alglib::real_1d_array& x,
                          double& f,
                          alglib::real_1d_array& grad,
                          void* instance
                        ) {
  //the solver context is provided as the instance
  AlgCG* s = (AlgCG*) instance ;

  //the objective function is a member
  ObjFun* objfun = (ObjFun*) s->objfun_ ;

  //call the callback
  AlgCG::EvalData data = {
    x.getcontent(),
    (unsigned int)x.length(),
    grad.getcontent()
  } ;
  f = (*objfun)(&data) ;
}

} //end of namespace Solver
} //end of namespace Revoropt

#endif


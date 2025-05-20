/*
	Author: Joel Chacon Castillo
	Email: joel.chacon@cimat.mx
	Code based in the DE competition CEC 2017
*/

#ifndef _HEADER_H_
#define _HEADER_H_

#include <cstdlib>
#include<iostream>
#include <queue>
#include <iomanip>
#include <cstring>
#include <vector>
#include<cmath>
#include <fstream>
#include <string>
#include <sstream>

using namespace std;

typedef  double variable;
typedef variable *Individual;
typedef  double Fitness;

extern int g_function_number;
extern int g_problem_size;
extern unsigned int g_max_num_evaluations;
extern int function_name;

extern int g_pop_size;
extern int g_memory_size;
extern double g_p_best_rate;
extern double g_arc_rate;

extern ofstream outFile;

extern char g_fileName[1000];
extern double g_Di;

//void cec14_test_func(double *, double *,int,int,int);
void cec17_test_func(double *, double *,int,int,int);

class searchAlgorithm {
public:
  virtual Fitness run() = 0;
protected:
  void evaluatePopulation(vector<Individual> &pop, vector<Fitness> &fitness);
  void initializeFitnessFunctionParameters();

  void initializeParameters();
  Individual makeNewIndividual();
  void modifySolutionWithParentMedium(Individual child, Individual parent);
  void setBestSolution(const vector<Individual> &pop, const vector<Fitness> &fitness, Individual &bsf_solution, Fitness &bsf_fitness);
  inline vector<double> cpindividual(Individual &ind)
	{
	   vector <double> copyi(problem_size);
	   for(int i = 0; i < problem_size; i++) copyi[i] = ind[i];
	   return copyi;
	}
  //Return random value with uniform distribution [0, 1)
  inline double randDouble() {
    return (double)rand() / (double) RAND_MAX;
  }

  /*
    Return random value from Cauchy distribution with mean "mu" and variance "gamma"
    http://www.sat.t.u-tokyo.ac.jp/~omi/random_variables_generation.html#Cauchy
  */
  inline double cauchy_g(double mu, double gamma) {
    return mu + gamma * tan(M_PI*(randDouble() - 0.5));
  }

  /*
    Return random value from normal distribution with mean "mu" and variance "gamma"
    http://www.sat.t.u-tokyo.ac.jp/~omi/random_variables_generation.html#Gauss
  */
  inline double gauss(double mu, double sigma){
    return mu + sigma * sqrt(-2.0 * log(randDouble())) * sin(2.0 * M_PI * randDouble());
  }

  //Recursive quick sort with index array
  template<class VarType>
    void sortIndexWithQuickSort(VarType array[], int first, int last, int index[]) {
    VarType x = array[(first + last) / 2];
    int i = first;
    int j = last;
    VarType temp_var = 0;
    int temp_num = 0;

    while (true) {
      while (array[i] < x) i++;    
      while (x < array[j]) j--;      
      if (i >= j) break;

      temp_var = array[i];
      array[i] = array[j];
      array[j] = temp_var;

      temp_num = index[i];
      index[i] = index[j];
      index[j] = temp_num;

      i++;
      j--;
    }

    if (first < (i -1)) sortIndexWithQuickSort(array, first, i - 1, index);  
    if ((j + 1) < last) sortIndexWithQuickSort(array, j + 1, last, index);    
  }
  
  int function_number;
  int problem_size;
  variable max_region;
  variable min_region;
  Fitness optimum;
  // acceptable error value
  Fitness epsilon;
  unsigned int max_num_evaluations;
  int pop_size;
};

class DIVERSITY: public searchAlgorithm {
public:
  virtual Fitness run();
  void setSHADEParameters();
  void operateCurrentTo1Bin(const vector<Individual> &pop, Individual child, int &target, variable &scaling_factor, variable &cross_rate);
  double distance(Individual A, Individual B);
  void replacement(vector<Individual> &pop, vector<Fitness> &fitness, vector<Individual> &children,vector<Fitness> &children_fitness, vector<Individual> &elite, vector<Fitness> &elite_fitness);
 void reducePopulation(vector<Individual> &pop, vector<Fitness> & fitness,vector<Individual> & elite, vector<Fitness> &elite_fitness, int nfes, int max_num_evaluations);
  void updateD();
  int nfes, min_pop_size, max_pop_size;
  double D, Di;
};

class ClusteringBridge;
extern ClusteringBridge* g_clusteringBridge;

#endif



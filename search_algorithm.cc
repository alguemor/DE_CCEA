/*
  L-SHADE implemented by C++ for Special Session & Competition on Real-Parameter Single Objective Optimization at CEC-2014
  See the details of L-SHADE in the following paper:

  * Ryoji Tanabe and Alex Fukunaga: Improving the Search Performance of SHADE Using Linear Population Size Reduction,  Proc. IEEE Congress on Evolutionary Computation (CEC-2014), Beijing, July, 2014.
  
  Version: 1.0   Date: 16/Apr/2014
  Written by Ryoji Tanabe (rt.ryoji.tanabe [at] gmail.com)
*/

#include"de.h"
#include "bridge.h"
#include <climits>
#include <algorithm>

void searchAlgorithm::initializeParameters() {
  function_number = g_function_number;
  problem_size = g_problem_size;
  max_num_evaluations = g_max_num_evaluations;
  pop_size = g_pop_size;
  initializeFitnessFunctionParameters();
}

void searchAlgorithm::evaluatePopulation(vector<Individual> &pop, vector<Fitness> &fitness) {
    for (int i = 0; i < pop_size; i++) {
        if(g_clusteringBridge != nullptr){
            fitness[i] = g_clusteringBridge->evaluateIndividual(pop[i]);
        }else{
            cec17_test_func(pop[i],  &fitness[i], problem_size, 1, function_number);
        }
    }
}

void searchAlgorithm::initializeFitnessFunctionParameters() {
  //epsilon is an acceptable error value.
  epsilon = pow(10.0, -8);
  max_region = 100.0;
  min_region = -100.0;

  optimum = function_number * 100;
}

//set best solution (bsf_solution) and its fitness value (bsf_fitness) in the initial population
void searchAlgorithm::setBestSolution(const vector<Individual> &pop, const vector<Fitness> &fitness, Individual &bsf_solution, Fitness &bsf_fitness) {
  int current_best_individual = 0;

  for (int i = 1; i < pop_size; i++) {
    if (fitness[current_best_individual] > fitness[i]) {
      current_best_individual = i;
    }
  }

  bsf_fitness = fitness[current_best_individual];
  for (int i = 0; i < problem_size; i++) {
    bsf_solution[i] = pop[current_best_individual][i];
  }
}

// make new individual randomly
Individual searchAlgorithm::makeNewIndividual() {
  Individual individual = (variable*)malloc(sizeof(variable) * problem_size);

  if (g_clusteringBridge != nullptr) {
    const auto* problem = g_clusteringBridge->getProblem();
    const auto& dataset = problem->getDataset();
    
    int minX = INT_MAX, minY = INT_MAX;
    int maxX = INT_MIN, maxY = INT_MIN;
    
    for (const auto& point : dataset) {
      if (point.size() >= 2) {
        minX = std::min(minX, point[0]);
        minY = std::min(minY, point[1]);
        maxX = std::max(maxX, point[0]);
        maxY = std::max(maxY, point[1]);
      }
    }
    
    int numClusters = problem_size / 2;
    for (int i = 0; i < numClusters; i++) {
      individual[i * 2] = minX + randDouble() * (maxX - minX);
      individual[i * 2 + 1] = minY + randDouble() * (maxY - minY);
    }
  } else {
    for (int i = 0; i < problem_size; i++) {
      individual[i] = ((max_region - min_region) * randDouble()) + min_region;
    }
  }

  return individual;
}

/*
  For each dimension j, if the mutant vector element v_j is outside the boundaries [x_min , x_max], we applied this bound handling method
  If you'd like to know that precisely, please read:
  J. Zhang and A. C. Sanderson, "JADE: Adaptive differential evolution with optional external archive,"
  IEEE Tran. Evol. Comput., vol. 13, no. 5, pp. 945–958, 2009.
 */
void searchAlgorithm::modifySolutionWithParentMedium(Individual child, Individual parent) {
  int l_problem_size = problem_size;
  variable l_min_region = min_region;
  variable l_max_region = max_region;

  for (int j = 0; j < l_problem_size; j++) {
    if (child[j] < l_min_region) {
      child[j]= (l_min_region + parent[j]) / 2.0;
    }
    else if (child[j] > l_max_region) {
      child[j]= (l_max_region + parent[j]) / 2.0;
    }
  }
}


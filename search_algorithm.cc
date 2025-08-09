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
        fitness[i] = g_clusteringBridge->evaluateIndividual(pop[i]);
    }
}

void searchAlgorithm::initializeFitnessFunctionParameters() {
    //epsilon is an acceptable error value.
    epsilon = pow(10.0, -8);
    
    if(g_clusteringBridge != nullptr){
        const auto* problem = g_clusteringBridge->getProblem();
        const auto& dataset = problem->getDataset();
        int variables = problem->getVariables();
        
        variables_per_cluster = variables;
        num_clusters = problem->getNumClusters();

        min_bounds_per_dim.resize(variables);
        max_bounds_per_dim.resize(variables);

        for(int d = 0; d < variables; d++){
            min_bounds_per_dim[d] = numeric_limits<double>::max();
            max_bounds_per_dim[d] = numeric_limits<double>::lowest();
        }

        for(const auto& point : dataset){
            for(int d = 0; d < variables; d++){
                min_bounds_per_dim[d] = min(min_bounds_per_dim[d], (double)point[d]);
                max_bounds_per_dim[d] = max(max_bounds_per_dim[d], (double)point[d]);
            }
        }
    
        // DEBUG: Mostrar rangos calculados
        cout << "=== RANGOS POR DIMENSION ===" << endl;
        for(int d = 0; d < variables; d++){
            cout << "Dim " << d << ": [" << min_bounds_per_dim[d] 
                 << ", " << max_bounds_per_dim[d] << "]" << endl;
        }

        // Usar el rango más amplio con margen
        min_region = *min_element(min_bounds_per_dim.begin(), min_bounds_per_dim.end()) - 1.0;
        max_region = *max_element(max_bounds_per_dim.begin(), max_bounds_per_dim.end()) + 1.0; 
    }else{
        max_region = 100.0;
        min_region = -100.0;
    }

    optimum = 0.0;
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

    if(g_clusteringBridge != nullptr) {
        const auto* problem = g_clusteringBridge->getProblem();
        const auto& dataset = problem->getDataset();
        int variables = problem->getVariables();
        int numClusters = problem->getNumClusters();
        
        if(!min_bounds_per_dim.empty() && !max_bounds_per_dim.empty()) {
            for(int c = 0; c < numClusters; c++) {
                for(int d = 0; d < variables; d++){
                    int index = c * variables + d;
                    individual[index] = min_bounds_per_dim[d] + 
                                      randDouble() * (max_bounds_per_dim[d] - min_bounds_per_dim[d]);
                }
            }
        } else {
            // Fallback al cálculo en tiempo real
            vector<double> minVals(variables, numeric_limits<double>::max());
            vector<double> maxVals(variables, numeric_limits<double>::lowest());
            const auto& dataset = problem->getDataset();
            
            for(const auto& point : dataset){
                for(int d = 0; d < variables; d++){
                    minVals[d] = min(minVals[d], (double)point[d]);
                    maxVals[d] = max(maxVals[d], (double)point[d]);
                }
            }

            for(int c = 0; c < numClusters; c++) {
                for(int d = 0; d < variables; d++){
                    int index = c * variables + d;
                    individual[index] = minVals[d] + randDouble() * (maxVals[d] - minVals[d]);
                }
            }
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
    
    if(min_bounds_per_dim.empty() || max_bounds_per_dim.empty()){
        for (int j = 0; j < l_problem_size; j++) {
            if (child[j] < l_min_region) {
                child[j]= (l_min_region + parent[j]) / 2.0;
            }else if (child[j] > l_max_region) {
                child[j]= (l_max_region + parent[j]) / 2.0;
            }
        }
        return;
    }

    for (int j = 0; j < problem_size; j++) {
        // Determinar a qué dimensión corresponde este índice
        int dimension = j % variables_per_cluster;
        
        double min_bound = min_bounds_per_dim[dimension];
        double max_bound = max_bounds_per_dim[dimension];
        
        if (child[j] < min_bound) {
            child[j] = (min_bound + parent[j]) / 2.0;
            // ✅ Asegurar que no siga fuera de rango
            if (child[j] < min_bound) {
                child[j] = min_bound;
            }
        }
        else if (child[j] > max_bound) {
            child[j] = (max_bound + parent[j]) / 2.0;
            // ✅ Asegurar que no siga fuera de rango
            if (child[j] > max_bound) {
                child[j] = max_bound;
            }
        }
    }
}


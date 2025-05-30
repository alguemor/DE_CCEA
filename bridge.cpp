#include "bridge.h"
using namespace std;

ClusteringBridge::ClusteringBridge(const string& datasetFile, const string& clustersFile) {
    problem = new Problem(datasetFile, clustersFile);
    problem->loadData();
}

ClusteringBridge::~ClusteringBridge() {
    delete problem;
}

void ClusteringBridge::individualToSolution(Individual individual, Solution*& solution) {
    if (solution == nullptr) {
        solution = new Solution(*problem);
    }
    
    int numClusters = problem->getNumClusters();
    int variables = problem->getVariables();
    
    // Convertir el individuo a centros de clusters
    solution->beforeClusterCenters.clear();
    solution->beforeClusterCenters.resize(numClusters, vector<double>(variables));

    for (int i = 0; i < numClusters; i++) {
        for(int d = 0; d < variables; d++){
            solution->beforeClusterCenters[i][d] = individual[i * variables + d];
        } 
    }
    
    // calcular distancias desde cada punto a los centros
    solution->calculateDistances();
    solution->sortDistances();
}

Fitness ClusteringBridge::evaluateIndividual(Individual individual) {
    Solution* solution = nullptr;
    individualToSolution(individual, solution);
   
    // se cambio desde individualToSolution
    solution->greedy();
    solution->updateEvaluation();

    Fitness fitness = solution->getFitness();
    
    //cout << fitness << endl;

    delete solution;
    return fitness;
}

int ClusteringBridge::getNumClusters() const {
    return problem->getNumClusters();
}

int ClusteringBridge::getNumPoints() const {
    return problem->getPoints();
}

int ClusteringBridge::getDimension() const {
    return problem->getVariables();
}

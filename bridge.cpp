#include "bridge.h"

ClusteringBridge::ClusteringBridge(const std::string& datasetFile, const std::string& clustersFile) {
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
    
    // Convertir el individuo a centros de clusters
    solution->beforeClusterCenters.clear();
    for (int i = 0; i < numClusters; i++) {
        double x = individual[i * 2];
        double y = individual[i * 2 + 1];
        solution->beforeClusterCenters.push_back(std::make_pair(x, y));    
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

#include "bridge.h"

ClusteringBridge::ClusteringBridge(const std::string& datasetFile, const std::string& clustersFile) {
    problem = new Problem(datasetFile, clustersFile);
    problem->loadData();
}

ClusteringBridge::~ClusteringBridge() {
    delete problem;
}

void ClusteringBridge::individualToSolution(Individual individual, Solution& solution) {
    // Implementación será añadida después
}

Fitness ClusteringBridge::evaluateIndividual(Individual individual) {
    // Implementación será añadida después
    return 0.0;
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

#include "de.h"
#include "problem.h"
#include "solutionGreedy.h"
#include "util.h"
#include "bridge.h"
#include <iostream>

int main() {
    ClusteringBridge bridge("input_data/ruspini/01ruspini.txt", "input_data/ruspini/01clusters.txt");
    
    std::cout << "Número de clusters: " << bridge.getNumClusters() << std::endl;
    std::cout << "Número de puntos: " << bridge.getNumPoints() << std::endl;
    std::cout << "Dimensión: " << bridge.getDimension() << std::endl;
    
    return 0;
}

#ifndef BRIDGE_H
#define BRIDGE_H

#include "de.h"
#include "problem.h"
#include "solutionGreedy.h"
#include "util.h"

class ClusteringBridge {
private:
    Problem* problem;
    vector<vector<long double>> bestAfterCenters;
    
public:
    ClusteringBridge(const std::string& datasetFile, const std::string& clustersFile);
    ~ClusteringBridge();
    
    // Convierte un individuo DE a una solución de clustering
    void individualToSolution(Individual individual, Solution*& solution);
    
    // Evalúa un individuo y retorna su fitness
    Fitness evaluateIndividual(Individual individual);
    
    // Obtiene información del problema
    int getNumClusters() const;
    int getNumPoints() const;
    int getDimension() const;
    
    // Acceso al problema
    Problem* getProblem() const { return problem; }

    const vector<vector<long double>>& getBestAfterCenters() const;
    void setBestAfterCenters(const vector<vector<long double>>& centers);
};

#endif

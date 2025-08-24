#ifndef BRIDGE_H
#define BRIDGE_H

#include "de.h"
#include "problem.h"
#include "solution.h"
#include "util.h"

class ClusteringBridge{
private:
    Problem* problem;
    std::string solverMethod;  // greedy/mcfp
    vector<vector<long double>> bestAfterCenters;
    vector<int> bestAssignment;
    vector<long double> bestClusterValues;
    vector<vector<long double>> bestPointDistances;
    Fitness bestFitness;
    
public:
    ClusteringBridge(const std::string& datasetFile, const std::string& clustersFile, const std::string& method);
    ~ClusteringBridge();
    
    // convierte individuo DE a una solución de clustering
    void individualToSolution(Individual individual, Solution*& solution);
    
    // evalua individuo y retorna fitness
    Fitness evaluateIndividual(Individual individual);
    
    int getNumClusters() const;
    int getNumPoints() const;
    int getDimension() const;
    
    Problem* getProblem() const { return problem; }

    const vector<vector<long double>>& getBestAfterCenters() const;
    void setBestAfterCenters(const vector<vector<long double>>& centers);
    
    const vector<int>& getBestAssignment() const;
    const vector<long double>& getBestClusterValues() const;
    const vector<vector<long double>>& getBestPointDistances() const;
    Fitness getBestFitness() const;
    
    void outputComprehensiveSolution(ofstream& outFile) const;
};

#endif

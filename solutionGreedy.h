#ifndef solutionGreedy_h
#define solutionGreedy_h
#include "problem.h"
#include <vector>
#include <tuple>
#include <utility>
#include <random>
#include <numeric>
using namespace std;

class Solution{
private:
    Problem& problem;
    vector<vector<double>> distances;
    vector<tuple<int, int, double>> allDistances;
    vector<int> clusterLimits;
    bool clusterCoordinatesUpdated; // flag coordenadas actualizadas

public:
    Solution(Problem& prob);
    ~Solution();

    vector<vector<double>> beforeClusterCenters;
    // centros despues de asignacion
    vector<vector<double>> afterClusterCenters;
    vector<int> assignment;
    // cache en Solution, logica en Problem
    vector<vector<vector<double>>> clusterCoordinates;
    vector<double> clusterValues;
    double fitness;
    double distance;

    void greedy();
    const vector<vector<vector<double>>>& getClusterCoordinates(); // flag
 
    void calculateDistances();
    void sortDistances();
    void calculateClusterCoordinates();

    // metodo actualizar valores evaluacion
    void updateEvaluation();
    void solveGreedy();
    // getters
    const vector<vector<double>>& getBeforeClusterCenters() const;
    const vector<vector<vector<double>>>& getClusterCoordinates() const;
    const vector<int>& getAssignment() const;
    const vector<double>& getClusterValues() const;
    double getFitness() const;

    const vector<vector<double>>& getAfterClusterCenters() const;
    // distancia
    double getDistance() const;
};

#endif

#ifndef solution_h
#define solution_h
#include "problem.h"
#include <vector>
#include <tuple>
#include <utility>
#include <random>
#include <numeric>
using namespace std;

class Solution{

};

class Greedy{
private:
    Problem& problem;
    vector<vector<long double>> distances;
    vector<tuple<int, int, long double>> allDistances;
    vector<int> clusterLimits;
    bool clusterCoordinatesUpdated; // flag coordenadas actualizadas

public:
    Greedy(Problem& prob);
    ~Greedy();

    vector<vector<long double>> beforeClusterCenters;
    // centros despues de asignacion
    vector<vector<long double>> afterClusterCenters;
    vector<int> assignment;
    // cache en Solution, logica en Problem
    vector<vector<vector<long double>>> clusterCoordinates;
    vector<long double> clusterValues;
    long double fitness;
    long double distance;

    void greedy();
    const vector<vector<vector<long double>>>& getClusterCoordinates(); // flag
 
    void calculateDistances();
    void sortDistances();
    void calculateClusterCoordinates();

    // metodo actualizar valores evaluacion
    void updateEvaluation();
    void solveGreedy();
    // getters
    const vector<vector<long double>>& getBeforeClusterCenters() const;
    const vector<vector<vector<long double>>>& getClusterCoordinates() const;
    const vector<int>& getAssignment() const;
    const vector<long double>& getClusterValues() const;
    long double getFitness() const;

    const vector<vector<long double>>& getAfterClusterCenters() const;
    // distancia
    long double getDistance() const;
};

class MCFP{

};

#endif

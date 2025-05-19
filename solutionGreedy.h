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

    void calculateDistances();
    void sortDistances();
    void calculateClusterCoordinates();

public:
    Solution(Problem& prob);
    ~Solution();

    vector<pair<double, double>> beforeClusterCenters;
    // centros despues de asignacion
    vector<pair<double, double>> afterClusterCenters;
    vector<int> assignment;
    // cache en Solution, logica en Problem
    vector<vector<pair<int, int>>> clusterCoordinates;
    vector<double> clusterValues;
    double fitness;
    double distance;

    void greedy();
    const vector<vector<pair<int, int>>>& getClusterCoordinates(); // flag
    // metodo actualizar valores evaluacion
    void updateEvaluation();
    void solveGreedy();
    // getters
    const vector<pair<double, double>>& getBeforeClusterCenters() const;
    const vector<vector<pair<int, int>>>& getClusterCoordinates() const;
    const vector<int>& getAssignment() const;
    const vector<double>& getClusterValues() const;
    double getFitness() const;

    const vector<pair<double, double>>& getAfterClusterCenters() const;
    // distancia
    double getDistance() const;
};

#endif
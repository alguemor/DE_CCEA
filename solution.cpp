#include "solution.h"
#include "util.h"
#include <iostream>
#include <limits>
#include <algorithm>
using namespace std;

Greedy::Greedy(Problem& prob) : problem(prob), fitness(0.0L), clusterCoordinatesUpdated(false){
    int numClusters = problem.getNumClusters();
    int numPoints = problem.getPoints();
    int variables = problem.getVariables();
    distances.resize(numPoints, vector<long double>(numClusters, 0.0L));
    assignment.resize(numPoints, -1);
    clusterLimits = problem.getLimClusters();
    clusterValues.resize(numClusters, 0.0L);
    clusterCoordinates.resize(numClusters);

    beforeClusterCenters.resize(numClusters, vector<long double>(variables, 0.0L));
    afterClusterCenters.resize(numClusters, vector<long double>(variables, 0.0L));
}

Greedy::~Greedy(){
}

void Greedy::calculateDistances(){
    const auto& dataset = problem.getDataset();
    int numPoints = problem.getPoints();
    int numClusters = problem.getNumClusters();
    int variables = problem.getVariables();

    for(int i = 0; i < numPoints; i++){
        for(int j = 0; j < numClusters; j++){
            long double distance = 0.0L;
            for(int d = 0; d < variables; d++){
                long double dif = dataset[i][d] - beforeClusterCenters[j][d];
                distance += dif * dif;
            }
            distances[i][j] = sqrtl(distance);
        }
    }
}

void Greedy::sortDistances(){
    int numPoints = problem.getPoints();
    int numClusters = problem.getNumClusters();

    allDistances.clear();
    for(int i = 0; i < numPoints; i++){
        for(int j = 0; j < numClusters; j++){
            allDistances.push_back(make_tuple(i, j, distances[i][j]));
        }
    }

    sort(allDistances.begin(), allDistances.end(), [](const auto& a, const auto& b){
        return std::get<2>(a) < std::get<2>(b);
    });
}

void Greedy::greedy(){
    int numPoints = problem.getPoints();

    fill(assignment.begin(), assignment.end(), -1);
    clusterLimits = problem.getLimClusters();

    // DEBUG: Estado inicial
    //cout << "=== DEBUG GREEDY ===" << endl;
    //cout << "Limites iniciales: ";
    //for(int i = 0; i < clusterLimits.size(); i++) {
    //    cout << "C" << i << ":" << clusterLimits[i] << " ";
    //}
    //cout << endl;

    if(allDistances.empty()){
        sortDistances();
    }

    for(const auto& dist_tuple : allDistances){
        int point_id = std::get<0>(dist_tuple);
        int center_id = std::get<1>(dist_tuple);
        if(assignment[point_id] != -1) continue;
        if(clusterLimits[center_id] > 0){
            assignment[point_id] = center_id;
            clusterLimits[center_id]--;
        }
    }

    // DEBUG: Conteo final
    //vector<int> count(problem.getNumClusters(), 0);
    //int unassigned = 0;
    //for(int i = 0; i < assignment.size(); i++) {
    //    if(assignment[i] == -1) {
    //        unassigned++;
    //    } else {
    //        count[assignment[i]]++;
    //    }
    //}

    //cout << "Asignacion final: ";
    //for(int i = 0; i < count.size(); i++) {
    //    cout << "C" << i << ":" << count[i] << " ";
    //}
    //cout << "Sin asignar: " << unassigned << endl;
    //cout << "===================" << endl;

    clusterCoordinatesUpdated = false;
}

void Solution::calculateClusterCoordinates(){
    clusterCoordinates = problem.calculatePointsCoordinatesPerCluster(assignment);
    clusterCoordinatesUpdated = true;
}

void Solution::updateEvaluation(){
    fitness = problem.evaluateSolution(assignment, clusterCoordinates, clusterValues);
    clusterCoordinatesUpdated = true;
}

void Solution::solveGreedy(){
    Util util(problem, *this); // crea instancia de Util

    if(beforeClusterCenters.empty() || beforeClusterCenters[0].empty()){
        beforeClusterCenters = util.generateRandomCenters(problem.getNumClusters());
    }

    calculateDistances();
    sortDistances();
    greedy();
    // actualiza evaluacion con Problem
    updateEvaluation();

    afterClusterCenters = util.calculateRealClusterCoordinates(problem.getNumClusters());

    distance = util.distance(beforeClusterCenters, afterClusterCenters);

    util.printBeforeClusterCenters();
    util.printAfterClusterCenters(); // despues de asignacion
    util.printAssignment();
    //util.printClusterCoordinates();
    util.printClusterValues();
    util.printFitness();

    //util.printDistance();
}

const vector<vector<long double>>& Solution::getBeforeClusterCenters() const{
    return beforeClusterCenters;
}

const vector<vector<long double>>& Solution::getAfterClusterCenters() const{
    return afterClusterCenters;
}

const vector<vector<vector<long double>>>& Solution::getClusterCoordinates(){
    if(!clusterCoordinatesUpdated){
        clusterCoordinates = problem.calculatePointsCoordinatesPerCluster(assignment);
        clusterCoordinatesUpdated = true;
    }
    return clusterCoordinates;
}

const vector<vector<vector<long double>>>& Solution::getClusterCoordinates() const{
    return clusterCoordinates; // asumimos coordenadas calculadas
}

const vector<int>& Solution::getAssignment() const{
    return assignment;
}

const vector<long double>& Solution::getClusterValues() const{
    return clusterValues;
}

long double Solution::getFitness() const{
    return fitness;
}

long double Solution::getDistance() const{
    return distance;
}

#include "solutionGreedy.h"
#include "util.h"
#include <iostream>
#include <limits>
#include <algorithm>
using namespace std;

Solution::Solution(Problem& prob) : problem(prob), fitness(0.0), clusterCoordinatesUpdated(false){
    int numClusters = problem.getNumClusters();
    int numPoints = problem.getPoints();
    distances.resize(numPoints, vector<double>(numClusters, 0.0));
    assignment.resize(numPoints, 0);
    clusterLimits = problem.getLimClusters();
    clusterValues.resize(numClusters, 0.0);
    clusterCoordinates.resize(numClusters);
}

Solution::~Solution(){
}

void Solution::calculateDistances(){
    const auto& dataset = problem.getDataset();
    int numPoints = problem.getPoints();
    int numClusters = problem.getNumClusters();

    for(int i = 0; i < numPoints; i++){
        for(int j = 0; j < numClusters; j++){
            int dx = dataset[i][0] - beforeClusterCenters[j].first;
            int dy = dataset[i][1] - beforeClusterCenters[j].second;
            distances[i][j] = sqrt(dx*dx + dy*dy);
        }
    }
}

void Solution::sortDistances(){
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

void Solution::greedy(){
    int numPoints = problem.getPoints();

    fill(assignment.begin(), assignment.end(), 0);
    clusterLimits = problem.getLimClusters();

    if(allDistances.empty()){
        sortDistances();
    }

    for(const auto& dist_tuple : allDistances){
        int point_id = std::get<0>(dist_tuple);
        int center_id = std::get<1>(dist_tuple);
        if(assignment[point_id] != 0) continue;
        if(clusterLimits[center_id] > 0){
            assignment[point_id] = center_id;
            clusterLimits[center_id]--;
        }
    }

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
    beforeClusterCenters = util.generateRandomCenters(problem.getNumClusters());

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

    util.printDistance();
}

const vector<pair<double, double>>& Solution::getBeforeClusterCenters() const{
    return beforeClusterCenters;
}

const vector<pair<double, double>>& Solution::getAfterClusterCenters() const{
    return afterClusterCenters;
}

const vector<vector<pair<int, int>>>& Solution::getClusterCoordinates(){
    if(!clusterCoordinatesUpdated){
        clusterCoordinates = problem.calculatePointsCoordinatesPerCluster(assignment);
        clusterCoordinatesUpdated = true;
    }
    return clusterCoordinates;
}

const vector<vector<pair<int, int>>>& Solution::getClusterCoordinates() const{
    return clusterCoordinates; // asumimos coordenadas calculadas
}

const vector<int>& Solution::getAssignment() const{
    return assignment;
}

const vector<double>& Solution::getClusterValues() const{
    return clusterValues;
}

double Solution::getFitness() const{
    return fitness;
}

double Solution::getDistance() const{
    return distance;
}
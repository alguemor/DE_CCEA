#include "solution.h"
#include "util.h"
#include <iostream>
#include <limits>
#include <algorithm>
#include <queue> // mcfp
#include <cstring> // mcfp
using namespace std;

Solution::Solution(Problem& prob) : problem(prob), fitness(0.0L), distance(0.0L), clusterCoordinatesUpdated(false){
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

void Solution::calculateDistances(){
    const auto& dataset = problem.getDataset();
    int numPoints = problem.getPoints();
    int numClusters = problem.getNumClusters();
    int variables = problem.getVariables();

    for(int i = 0; i < numPoints; i++){
        for(int j = 0; j < numClusters; j++){
            vector<long double> pointCoords(dataset[i].begin(), dataset[i].end());
            distances[i][j] = squaredDistance(pointCoords, beforeClusterCenters[j]);
        }
    }
}

void Solution::calculateClusterCoordinates(){
    clusterCoordinates = problem.calculatePointsCoordinatesPerCluster(assignment);
    clusterCoordinatesUpdated = true;
}

void Solution::updateEvaluation(){
    fitness = problem.evaluateSolution(assignment, clusterCoordinates, clusterValues);
    clusterCoordinatesUpdated = true;
}

// getters compartidos
const vector<vector<long double>>& Solution::getBeforeClusterCenters() const{
    return beforeClusterCenters;
}

const vector<vector<long double>>& Solution::getAfterClusterCenters() const{
    return afterClusterCenters;
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

const vector<vector<vector<long double>>>& Solution::getClusterCoordinates(){
    if(!clusterCoordinatesUpdated) {
        clusterCoordinates = problem.calculatePointsCoordinatesPerCluster(assignment);
        clusterCoordinatesUpdated = true;
    }
    return clusterCoordinates;
}

const vector<vector<vector<long double>>>& Solution::getClusterCoordinates() const{
    return clusterCoordinates;
}

double Solution::squaredDistance(const vector<long double>& p1, const vector<long double>& p2){
    double sum = 0.0;
    for(int i = 0; i < p1.size() && i < p2.size(); i++){
        double diff = p1[i] - p2[i];
        sum += diff * diff;
    }
    return sum;
}


// greedy method

Greedy::Greedy(Problem& prob) : Solution(prob){
}

Greedy::~Greedy(){
}

void Greedy::sortDistancesGreedy(){
    int numPoints = problem.getPoints();
    int numClusters = problem.getNumClusters();

    allDistances.clear();
    for(int i = 0; i < numPoints; i++) {
        for(int j = 0; j < numClusters; j++) {
            allDistances.push_back(make_tuple(i, j, distances[i][j]));
        }
    }

    sort(allDistances.begin(), allDistances.end(), [](const auto& a, const auto& b) {
        return std::get<2>(a) < std::get<2>(b);
    });
}

void Greedy::greedy(){
    int numPoints = problem.getPoints();

    fill(assignment.begin(), assignment.end(), -1);
    clusterLimits = problem.getLimClusters();

    if(allDistances.empty()) {
        sortDistancesGreedy();
    }

    for(const auto& dist_tuple : allDistances) {
        int point_id = std::get<0>(dist_tuple);
        int center_id = std::get<1>(dist_tuple);
        if(assignment[point_id] != -1) continue;
        if(clusterLimits[center_id] > 0) {
            assignment[point_id] = center_id;
            clusterLimits[center_id]--;
        }
    }

    clusterCoordinatesUpdated = false;
}

void Greedy::solveGreedy(){
    Util util(problem, *this);

    if(beforeClusterCenters.empty() || beforeClusterCenters[0].empty()){
        beforeClusterCenters = util.generateRandomCenters(problem.getNumClusters());
    }

    calculateDistances();
    sortDistancesGreedy();
    greedy();
    updateEvaluation();

    afterClusterCenters = util.calculateRealClusterCoordinates(problem.getNumClusters());
    distance = util.distance(beforeClusterCenters, afterClusterCenters);

    // util.printBeforeClusterCenters();
    // util.printAfterClusterCenters();
    // util.printAssignment();
    // util.printClusterValues();
    // util.printFitness();
}

// mcfp method

MCFP::MCFP(Problem& prob) : Solution(prob), totalNodes(0), mxFlow(0), mnCost(0){
    int numPoints = problem.getPoints();
    int numClusters = problem.getNumClusters();
    
    flowGraph.resize(MAXN);
    dist.resize(MAXN);
    pre.resize(MAXN);
    cap.resize(MAXN);
    in_queue.resize(MAXN);
}

MCFP::~MCFP(){
}

void MCFP::clearGraph(){
    for(int i = 0; i < totalNodes; i++){
        flowGraph[i].clear();    
    }
    flowEdges.clear();
}

void MCFP::addEdge(int u, int v, long long capacity, long long cost){
    flowGraph[u].push_back(flowEdges.size()); 
    flowEdges.push_back((edge){u, v, capacity, 0, cost});
    flowGraph[v].push_back(flowEdges.size()); 
    flowEdges.push_back((edge){v, u, 0, 0, -cost});
}


void MCFP::buildMCFPGraph(){
    int numPoints = problem.getPoints();
    int numClusters = problem.getNumClusters();
    
    totalNodes = 1 + numPoints + numClusters + 1; // source + points + clusters + sink
    clearGraph();
    
    int source = 0;
    int sink = numPoints + numClusters + 1;
    
    // source -> points (capacity 1, cost 0)
    for(int i = 0; i < numPoints; i++){
        addEdge(source, i + 1, 1, 0);
    }
    
    // points -> cluster centers (capacity 1, cost = squared distance)  
    const auto& dataset = problem.getDataset();
    for(int i = 0; i < numPoints; i++){
        vector<long double> pointCoords(dataset[i].begin(), dataset[i].end());
        for(int j = 0; j < numClusters; j++) {
            double cost = squaredDistance(pointCoords, beforeClusterCenters[j]);
            long long intCost = (long long)(cost * 1000); // scale for precision
            addEdge(i + 1, numPoints + 1 + j, 1, intCost);
        }
    }
    
    // cluster centers -> sink (capacity = cluster limit, cost 0)
    for(int j = 0; j < numClusters; j++){
        addEdge(numPoints + 1 + j, sink, clusterLimits[j], 0);
    }
    
    // cout << "Graph built with " << flowEdges.size()/2 << " edges" << endl;
}

void MCFP::solveMCFPFlow(){
    int source = 0;
    int sink = problem.getPoints() + problem.getNumClusters() + 1;
    
    fill(in_queue.begin(), in_queue.begin() + totalNodes, false);
    mxFlow = mnCost = 0;
    
    while(1){
        fill(dist.begin(), dist.begin() + totalNodes, INFCOSTO); 
        dist[source] = 0;
        fill(pre.begin(), pre.begin() + totalNodes, -1); 
        pre[source] = 0;
        fill(cap.begin(), cap.begin() + totalNodes, 0);
        cap[source] = INFFLUJO;        
        
        queue<int> q; 
        q.push(source); 
        in_queue[source] = true;
        
        while(q.size()){
            int u = q.front(); 
            q.pop(); 
            in_queue[u] = false;
            
            for(int edgeIdx : flowGraph[u]){
                edge &E = flowEdges[edgeIdx];
                if(E.rem() && dist[E.v] > dist[u] + E.cost + 1e-9){ 
                    dist[E.v] = dist[u] + E.cost;
                    pre[E.v] = edgeIdx;
                    cap[E.v] = min(cap[u], E.rem());
                    if(!in_queue[E.v]){
                        q.push(E.v);
                        in_queue[E.v] = true;
                    }
                }
            }
        }
        
        if(pre[sink] == -1){
            break;
        }
            
        mxFlow += cap[sink];
        mnCost += cap[sink] * dist[sink];
        
        for(int v = sink; v != source; v = flowEdges[pre[v]].u){
            flowEdges[pre[v]].flow += cap[sink];
            flowEdges[pre[v]^1].flow -= cap[sink];
        }
    }
}

void MCFP::extractAssignmentFromFlow(){
    int numPoints = problem.getPoints();
    int numClusters = problem.getNumClusters();
    
    fill(assignment.begin(), assignment.end(), -1);
    
    for(int i = 0; i < numPoints; i++){
        for(int j = 0; j < numClusters; j++){
            // encuentra la arista por la cual pasa el punto i+1 al cluster numPoints+1+j
            for(int edgeIdx = 0; edgeIdx < flowEdges.size(); edgeIdx += 2){
                if(flowEdges[edgeIdx].u == i + 1 && 
                    flowEdges[edgeIdx].v == numPoints + 1 + j && 
                    flowEdges[edgeIdx].flow > 0){
                    assignment[i] = j;
                    break;
                }
            }
        }
    }
}

void MCFP::solveMCFP(){
    Util util(problem, *this);

    if(beforeClusterCenters.empty() || beforeClusterCenters[0].empty()){
        beforeClusterCenters = util.generateRandomCenters(problem.getNumClusters());
    }

    clusterLimits = problem.getLimClusters();
    buildMCFPGraph();
    solveMCFPFlow();
    extractAssignmentFromFlow();
    updateEvaluation();

    afterClusterCenters = util.calculateRealClusterCoordinates(problem.getNumClusters());
    distance = util.distance(beforeClusterCenters, afterClusterCenters);

    // cout << "MCFP Results:" << endl;
    // cout << "- Max Flow: " << mxFlow << endl;
    // cout << "- MCFP Cost (scaled): " << mnCost << endl;
    
    // util.printBeforeClusterCenters();
    // util.printAfterClusterCenters();
    // util.printAssignment();
    // util.printClusterValues();
    // util.printFitness();
}
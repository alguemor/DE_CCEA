#ifndef solution_h
#define solution_h
#include "problem.h"
#include <vector>
#include <tuple>
#include <utility>
#include <random>
#include <numeric>
#include <queue>
#include <algorithm>
#include <limits>
#include <cstring>
using namespace std;

class Solution { // clase base para funcionalidades compartidas
protected:
    Problem& problem;
    vector<vector<long double>> distances;
    vector<int> clusterLimits;
    bool clusterCoordinatesUpdated; // flag coordenadas actualizadas

public:
    vector<vector<long double>> beforeClusterCenters; // datos
    vector<vector<long double>> afterClusterCenters;
    vector<int> assignment;
    vector<vector<vector<long double>>> clusterCoordinates;
    vector<long double> clusterValues;
    long double fitness;
    long double distance;

    Solution(Problem& prob);
    ~Solution() = default;

    // Shared methods
    void calculateDistances();
    void calculateClusterCoordinates();
    void updateEvaluation();
    
    // Shared getters
    const vector<vector<long double>>& getBeforeClusterCenters() const;
    const vector<vector<long double>>& getAfterClusterCenters() const;
    const vector<int>& getAssignment() const;
    const vector<long double>& getClusterValues() const;
    long double getFitness() const;
    long double getDistance() const;
    const vector<vector<vector<long double>>>& getClusterCoordinates();
    const vector<vector<vector<long double>>>& getClusterCoordinates() const;
};

// sub clases
class Greedy : public Solution {
private:
    vector<tuple<int, int, long double>> allDistances;

public:
    Greedy(Problem& prob);
    ~Greedy();

    void greedy();
    void sortDistancesGreedy();
    void solveGreedy();
    
};

class MCFP : public Solution {
private:
    static const int MAXN = 10000;
    static const long long INFFLUJO = 1e18;
    static const long long INFCOSTO = 1e18;
    struct edge {
        int u, v;
        long long cap, flow;
        long long cost;
        long long rem() { return cap - flow; }
    };
    
    vector<vector<int>> flowGraph;
    vector<edge> flowEdges;
    int totalNodes;
    
    vector<long long> dist;
    vector<int> pre;
    vector<long long> cap;
    vector<bool> in_queue;
    long long mxFlow, mnCost;

public:
    MCFP(Problem& prob);
    ~MCFP();

    void buildMCFPGraph();
    void solveMCFPFlow();
    void extractAssignmentFromFlow();
    void addEdge(int u, int v, long long cap, long long cost);
    void clearGraph();
    void solveMCFP();
    
    double squaredDistance(const vector<long double>& p1, const vector<long double>& p2);
    
};

#endif

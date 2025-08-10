#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cmath>
#include <iomanip>
#include <cstring>
#include <queue>
#include <algorithm>
#include <limits>

using namespace std;
typedef long long ll;

// MinCostMaxFlow implementation
const int MAXN=10000;
const long long INFFLUJO = 1e18;
const long long INFCOSTO = 1e18;

struct edge {
    int u, v;
    long long cap, flow;
    long long cost;
    long long rem() { return cap - flow; }
};

int nodes;
vector<int> G[MAXN];
vector<edge> e;

void addEdge(int u, int v, long long cap, long long cost) {
    G[u].push_back(e.size()); 
    e.push_back((edge){u,v,cap,0,cost});
    G[v].push_back(e.size()); 
    e.push_back((edge){v,u,0,0,-cost});
}

long long dist[MAXN], mnCost;
int pre[MAXN];
long long cap[MAXN], mxFlow;
bool in_queue[MAXN];

void flow(int s, int t) {
    memset(in_queue, 0, sizeof(in_queue));
    mxFlow=mnCost=0;
    while(1){
        fill(dist, dist+nodes, INFCOSTO); 
        dist[s] = 0;
        memset(pre, -1, sizeof(pre)); 
        pre[s]=0;
        memset(cap, 0, sizeof(cap));
        cap[s] = INFFLUJO;        
        queue<int> q; 
        q.push(s); 
        in_queue[s]=1;
        while(q.size()){
            int u=q.front(); 
            q.pop(); 
            in_queue[u]=0;
            for(vector<int>::iterator it = G[u].begin(); it != G[u].end(); it++) {
                edge &E = e[*it];
                if(E.rem() && dist[E.v] > dist[u] + E.cost + 1e-9){ 
                    dist[E.v]=dist[u]+E.cost;
                    pre[E.v] = *it;
                    cap[E.v] = min(cap[u], E.rem());
                    if(!in_queue[E.v]) 
                        q.push(E.v), in_queue[E.v]=1;
                }
            }
        }
        if (pre[t] == -1) 
            break;
        mxFlow += cap[t];
        mnCost += cap[t]*dist[t];
        for (int v = t; v != s; v = e[pre[v]].u) {
            e[pre[v]].flow += cap[t];
            e[pre[v]^1].flow -= cap[t];
        }
    }
}

void clear(int N){
    nodes = N;
    for (int i = 0; i < N; i++){
        G[i].clear();    
    }
    e.clear();
}

// Data structures
struct Point {
    int id;
    vector<double> coords;
    int deAssignedCluster;
    double distanceToCenter;
};

struct ClusterCenter {
    int id;
    vector<double> coords;
    int capacityLimit;
    int deAssignedPoints;
    double deIntraCost;
};

class MCFPValidator {
private:
    vector<Point> points;
    vector<ClusterCenter> clusters;
    double deFitness;
    int numPoints, numClusters, dimensions;

public:
    MCFPValidator() : deFitness(0), numPoints(0), numClusters(0), dimensions(0) {}
    
    bool parseResultFile(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "Error: Cannot open result file " << filename << endl;
            return false;
        }
        
        string line, section = "";
        
        while (getline(file, line)) {
            if (line.empty()) continue;
            
            if (line == "SOLUTION SUMMARY") {
                section = "SUMMARY";
                continue;
            } else if (line == "DATASET POINTS") {
                section = "DATASET";
                continue;
            } else if (line == "CLUSTER CENTERS") {
                section = "CENTERS";
                continue;
            } else if (line == "POINT ASSIGNMENTS") {
                section = "POINTS";
                continue;
            } else if (line == "CLUSTER DETAILS") {
                section = "DETAILS";
                continue;
            } else if (line.find("====") != string::npos || 
                       line.find("Cluster_ID") != string::npos ||
                       line.find("Point_ID") != string::npos) {
                continue;
            }
            
            if (section == "SUMMARY") {
                if (line.find("Final Fitness:") != string::npos) {
                    istringstream iss(line);
                    string dummy1, dummy2;
                    iss >> dummy1 >> dummy2 >> deFitness;
                } else if (line.find("Number of Points:") != string::npos) {
                    istringstream iss(line);
                    string dummy1, dummy2, dummy3;
                    iss >> dummy1 >> dummy2 >> dummy3 >> numPoints;
                } else if (line.find("Number of Clusters:") != string::npos) {
                    istringstream iss(line);
                    string dummy1, dummy2, dummy3;
                    iss >> dummy1 >> dummy2 >> dummy3 >> numClusters;
                } else if (line.find("Problem Dimensions:") != string::npos) {
                    istringstream iss(line);
                    string dummy1, dummy2;
                    iss >> dummy1 >> dummy2 >> dimensions;
                }
            } else if (section == "DATASET") {
                istringstream iss(line);
                Point point;
                iss >> point.id;
                point.coords.resize(dimensions);
                for(int d = 0; d < dimensions; d++) {
                    iss >> point.coords[d];
                }
                points.push_back(point);
            } else if (section == "CENTERS") {
                istringstream iss(line);
                ClusterCenter cluster;
                iss >> cluster.id;
                cluster.coords.resize(dimensions);
                for(int d = 0; d < dimensions; d++) {
                    iss >> cluster.coords[d];
                }
                iss >> cluster.capacityLimit >> cluster.deAssignedPoints >> cluster.deIntraCost;
                clusters.push_back(cluster);
            } else if (section == "POINTS") {
                istringstream iss(line);
                int pointId, assignedCluster;
                vector<double> coords(dimensions);
                double distance;
                
                iss >> pointId;
                for(int d = 0; d < dimensions; d++) {
                    iss >> coords[d];
                }
                iss >> assignedCluster >> distance;
                
                if (pointId < points.size()) {
                    points[pointId].deAssignedCluster = assignedCluster;
                    points[pointId].distanceToCenter = distance;
                }
            }
        }
        
        file.close();
        return true;
    }
    
    double squaredDistance(const vector<double>& p1, const vector<double>& p2) {
        double sum = 0.0;
        for (int i = 0; i < p1.size() && i < p2.size(); i++) {
            double diff = p1[i] - p2[i];
            sum += diff * diff;
        }
        return sum;
    }
    
    void buildMCFPGraph() {
        int totalNodes = 1 + numPoints + numClusters + 1; // source + points + clusters + sink
        clear(totalNodes);
        
        int source = 0;
        int sink = numPoints + numClusters + 1;
        
        cout << "Building MCFP graph..." << endl;
        
        // Source to points (capacity 1, cost 0)
        for (int i = 0; i < numPoints; i++) {
            addEdge(source, i + 1, 1, 0);
        }
        
        // Points to cluster centers (capacity 1, cost = squared distance)
        for (int i = 0; i < numPoints; i++) {
            for (int j = 0; j < numClusters; j++) {
                double cost = squaredDistance(points[i].coords, clusters[j].coords);
                long long intCost = (long long)(cost * 1000); // Scale for precision
                addEdge(i + 1, numPoints + 1 + j, 1, intCost);
            }
        }
        
        // Cluster centers to sink (capacity = cluster limit, cost 0)
        for (int j = 0; j < numClusters; j++) {
            addEdge(numPoints + 1 + j, sink, clusters[j].capacityLimit, 0);
        }
        
        cout << "Graph built with " << e.size()/2 << " edges" << endl;
    }
    
    vector<int> extractMCFPAssignment() {
        vector<int> assignment(numPoints, -1);
        
        for (int i = 0; i < numPoints; i++) {
            for (int j = 0; j < numClusters; j++) {
                // Find the edge from point i+1 to cluster numPoints+1+j
                for (int edgeIdx = 0; edgeIdx < e.size(); edgeIdx += 2) {
                    if (e[edgeIdx].u == i + 1 && 
                        e[edgeIdx].v == numPoints + 1 + j && 
                        e[edgeIdx].flow > 0) {
                        assignment[i] = j;
                        break;
                    }
                }
            }
        }
        
        return assignment;
    }
    
    // DE-CCEA fitness calculation method
    double calculateDEFitness(const vector<int>& assignment) {
        // Group points by cluster - same as Problem::calculatePointsCoordinatesPerCluster
        vector<vector<vector<double>>> coordinates(numClusters);
        
        for (int i = 0; i < numPoints; i++) {
            int cluster = assignment[i];
            if (cluster >= 0 && cluster < numClusters) {
                coordinates[cluster].push_back(points[i].coords);
            }
        }
        
        // Calculate cluster values - same as Problem::calculateClusterValues
        vector<double> clusterValues(numClusters, 0.0);
        for (int c = 0; c < numClusters; c++) {
            if (coordinates[c].empty()) continue;
            
            double sumSquaredDistances = 0.0;
            int clusterSize = coordinates[c].size();
            
            // Sum of all pairwise squared distances within cluster
            for (int i = 0; i < clusterSize; i++) {
                for (int j = 0; j < clusterSize; j++) {
                    double squaredDistance = 0.0;
                    for (int d = 0; d < dimensions; d++) {
                        double diff = coordinates[c][i][d] - coordinates[c][j][d];
                        squaredDistance += diff * diff;
                    }
                    sumSquaredDistances += squaredDistance;
                }
            }
            // Apply DE-CCEA formula: 0.5 * sum / clusterSize
            clusterValues[c] = 0.5 * sumSquaredDistances / (double)clusterSize;
        }
        
        // Calculate final fitness - same as Problem::calculateFitness
        double fitness = 0.0;
        for (int c = 0; c < numClusters; c++) {
            fitness += clusterValues[c];
        }
        
        return fitness;
    }
    
    void validateSolutions() {
        cout << "\n=== MCFP VALIDATION ===" << endl;
        
        // Build and solve MCFP
        buildMCFPGraph();
        
        int source = 0;
        int sink = numPoints + numClusters + 1;
        
        cout << "Running Min-Cost Max-Flow..." << endl;
        flow(source, sink);
        
        cout << "MCFP Results:" << endl;
        cout << "- Max Flow: " << mxFlow << endl;
        cout << "- MCFP Cost (scaled): " << mnCost << endl;
        
        // Extract MCFP assignment
        vector<int> mcfpAssignment = extractMCFPAssignment();
        
        // Calculate DE-CCEA fitness for both assignments
        vector<int> deAssignment(numPoints);
        for (int i = 0; i < numPoints; i++) {
            deAssignment[i] = points[i].deAssignedCluster;
        }
        
        double deFitnessRecalc = calculateDEFitness(deAssignment);
        double mcfpFitness = calculateDEFitness(mcfpAssignment);
        
        cout << "\n=== COMPARISON ===" << endl;
        cout << "DE-CCEA Original Fitness: " << deFitness << endl;
        cout << "DE-CCEA Recalculated:     " << deFitnessRecalc << endl;
        cout << "MCFP Assignment Fitness:  " << mcfpFitness << endl;
        cout << endl;
        
        double improvement = deFitnessRecalc - mcfpFitness;
        cout << "Improvement (DE - MCFP):  " << improvement << endl;
        
        if (abs(improvement) < 1e-6) {
            cout << "✅ BOTH METHODS FOUND SAME OPTIMAL SOLUTION!" << endl;
        } else if (improvement > 1e-6) {
            cout << "🎯 MCFP FOUND BETTER SOLUTION! (Lower fitness)" << endl;
            cout << "   This suggests DE-CCEA got trapped in local optimum." << endl;
        } else {
            cout << "⚠️  DE-CCEA FOUND BETTER SOLUTION!" << endl;
            cout << "   This is unexpected - MCFP should find global optimum." << endl;
        }
        
        // Check if recalculated DE fitness matches original
        if (abs(deFitness - deFitnessRecalc) > 1e-6) {
            cout << "\n⚠️  WARNING: DE-CCEA fitness recalculation differs from original!" << endl;
            cout << "   Difference: " << abs(deFitness - deFitnessRecalc) << endl;
        }
        
        // Show assignment differences
        int differences = 0;
        for (int i = 0; i < numPoints; i++) {
            if (mcfpAssignment[i] != deAssignment[i]) {
                differences++;
            }
        }
        
        cout << "\nAssignment differences: " << differences << "/" << numPoints 
             << " points (" << (100.0 * differences / numPoints) << "%)" << endl;
        
        if (differences > 0 && differences <= 10) {
            cout << "\nDetailed differences:" << endl;
            for (int i = 0; i < numPoints; i++) {
                if (mcfpAssignment[i] != deAssignment[i]) {
                    cout << "Point " << i << ": DE-CCEA=" << deAssignment[i] 
                         << ", MCFP=" << mcfpAssignment[i] << endl;
                }
            }
        }
    }
    
    void printSummary() {
        cout << "=== MCFP Validator for DE-CCEA Clustering ===" << endl;
        cout << "Points: " << numPoints << ", Clusters: " << numClusters 
             << ", Dimensions: " << dimensions << endl;
        cout << "DE-CCEA Fitness: " << deFitness << endl;
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <result_file>" << endl;
        cerr << "Example: " << argv[0] << " results/01/s1_p8_result.txt" << endl;
        return 1;
    }
    
    MCFPValidator validator;
    
    if (!validator.parseResultFile(argv[1])) {
        return 1;
    }
    
    validator.printSummary();
    validator.validateSolutions();
    
    return 0;
}
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

class IterativeMCFPValidator {
private:
    vector<Point> points;
    vector<ClusterCenter> clusters;
    double deFitness;
    int numPoints, numClusters, dimensions;

public:
    IterativeMCFPValidator() : deFitness(0), numPoints(0), numClusters(0), dimensions(0) {}
    
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
    
    // Calculate marginal cost of adding point i to cluster j
    double calculateMarginalCost(int pointId, int clusterId, const vector<int>& currentAssignment) {
        vector<int> pointsInCluster;
        
        // Find all points currently in this cluster
        for (int p = 0; p < numPoints; p++) {
            if (currentAssignment[p] == clusterId) {
                pointsInCluster.push_back(p);
            }
        }
        
        double marginalCost = 0.0;
        int currentSize = pointsInCluster.size();
        
        if (currentSize == 0) {
            // Empty cluster - no cost to add first point
            return 0.0;
        }
        
        // Calculate additional cost of adding this point
        // Cost increase = 2 * sum of distances from new point to existing points
        for (int existingPoint : pointsInCluster) {
            marginalCost += 2.0 * squaredDistance(points[pointId].coords, points[existingPoint].coords);
        }
        
        // Apply DE-CCEA normalization: divide by new cluster size
        marginalCost = 0.5 * marginalCost / (double)(currentSize + 1);
        
        return marginalCost;
    }
    
    void buildIterativeMCFPGraph(const vector<int>& currentAssignment) {
        int totalNodes = 1 + numPoints + numClusters + 1;
        clear(totalNodes);
        
        int source = 0;
        int sink = numPoints + numClusters + 1;
        
        // Source to points
        for (int i = 0; i < numPoints; i++) {
            addEdge(source, i + 1, 1, 0);
        }
        
        // Points to clusters with marginal costs
        for (int i = 0; i < numPoints; i++) {
            for (int j = 0; j < numClusters; j++) {
                double cost = calculateMarginalCost(i, j, currentAssignment);
                long long intCost = (long long)(cost * 10000); // Higher precision scaling
                addEdge(i + 1, numPoints + 1 + j, 1, intCost);
            }
        }
        
        // Clusters to sink
        for (int j = 0; j < numClusters; j++) {
            addEdge(numPoints + 1 + j, sink, clusters[j].capacityLimit, 0);
        }
    }
    
    vector<int> extractMCFPAssignment() {
        vector<int> assignment(numPoints, -1);
        
        for (int i = 0; i < numPoints; i++) {
            for (int j = 0; j < numClusters; j++) {
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
    
    double calculateDEFitness(const vector<int>& assignment) {
        vector<vector<vector<double>>> coordinates(numClusters);
        
        for (int i = 0; i < numPoints; i++) {
            int cluster = assignment[i];
            if (cluster >= 0 && cluster < numClusters) {
                coordinates[cluster].push_back(points[i].coords);
            }
        }
        
        vector<double> clusterValues(numClusters, 0.0);
        for (int c = 0; c < numClusters; c++) {
            if (coordinates[c].empty()) continue;
            
            double sumSquaredDistances = 0.0;
            int clusterSize = coordinates[c].size();
            
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
            clusterValues[c] = 0.5 * sumSquaredDistances / (double)clusterSize;
        }
        
        double fitness = 0.0;
        for (int c = 0; c < numClusters; c++) {
            fitness += clusterValues[c];
        }
        
        return fitness;
    }
    
    void runIterativeOptimization() {
        cout << "\n=== ITERATIVE MCFP OPTIMIZATION ===" << endl;
        
        // Start with DE-CCEA assignment
        vector<int> currentAssignment(numPoints);
        for (int i = 0; i < numPoints; i++) {
            currentAssignment[i] = points[i].deAssignedCluster;
        }
        
        double currentFitness = calculateDEFitness(currentAssignment);
        cout << "Initial DE-CCEA Fitness: " << currentFitness << endl;
        
        int iteration = 0;
        const int maxIterations = 10;
        
        while (iteration < maxIterations) {
            iteration++;
            cout << "\n--- Iteration " << iteration << " ---" << endl;
            
            // Build MCFP graph with current assignment costs
            buildIterativeMCFPGraph(currentAssignment);
            
            // Solve MCFP
            int source = 0;
            int sink = numPoints + numClusters + 1;
            flow(source, sink);
            
            // Extract new assignment
            vector<int> newAssignment = extractMCFPAssignment();
            double newFitness = calculateDEFitness(newAssignment);
            
            cout << "MCFP Fitness: " << newFitness << endl;
            cout << "Improvement: " << (currentFitness - newFitness) << endl;
            
            // Check for improvement
            if (newFitness < currentFitness - 1e-6) {
                // Count assignment differences
                int differences = 0;
                for (int i = 0; i < numPoints; i++) {
                    if (newAssignment[i] != currentAssignment[i]) {
                        differences++;
                    }
                }
                cout << "Assignment changes: " << differences << " points" << endl;
                
                currentAssignment = newAssignment;
                currentFitness = newFitness;
            } else {
                cout << "No improvement found - converged!" << endl;
                break;
            }
        }
        
        cout << "\n=== FINAL RESULTS ===" << endl;
        cout << "DE-CCEA Original: " << deFitness << endl;
        cout << "Iterative MCFP:   " << currentFitness << endl;
        cout << "Target Optimal:   81.27" << endl;
        cout << "Gap to Optimal:   " << (currentFitness - 81.27) << endl;
        
        if (abs(currentFitness - 81.27) < 0.01) {
            cout << "🎯 REACHED TRUE OPTIMUM!" << endl;
        } else if (currentFitness < deFitness) {
            cout << "🎯 IMPROVED OVER DE-CCEA!" << endl;
        } else {
            cout << "⚠️  No improvement over DE-CCEA" << endl;
        }
    }
    
    void printSummary() {
        cout << "=== Iterative MCFP Validator for DE-CCEA Clustering ===" << endl;
        cout << "Points: " << numPoints << ", Clusters: " << numClusters 
             << ", Dimensions: " << dimensions << endl;
        cout << "DE-CCEA Fitness: " << deFitness << endl;
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <result_file>" << endl;
        cerr << "Example: " << argv[0] << " results/04/s1_p12_result.txt" << endl;
        return 1;
    }
    
    IterativeMCFPValidator validator;
    
    if (!validator.parseResultFile(argv[1])) {
        return 1;
    }
    
    validator.printSummary();
    validator.runIterativeOptimization();
    
    return 0;
}
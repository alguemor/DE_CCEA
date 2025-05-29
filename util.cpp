#include "util.h"
#include "solutionGreedy.h"
#include <iostream>
#include <iomanip>
#include <limits>
#include <algorithm>
using namespace std;

Util::Util(const Problem& prob, const Solution& sol) : problem(prob), solution(sol), generator(random_device()()){
    // inicializar generador aleatorio
}

Util::~Util(){

}

vector<vector<double>> Util::generateRandomCenters(int numClusters) const{
    const auto& dataset = problem.getDataset();
    int variables = problem.getVariables();

    vector<double> minValues(variables, numeric_limits<double>::max());
    vector<double> maxValues(variables, numeric_limits<double>::lowest());

    for(const auto& point : dataset){
        if(point.size() == variables){
            for(int d = 0; d < variables; d++){
                minValues[d] = min(minValues[d], point[d]);
                maxValues[d] = max(maxValues[d], point[d]);
            }
        }
    }

    vector<uniform_real_distribution<double>> distros;
    for(int d = 0; d < variables; d++){
        distros.emplace_back(minValues[d], maxValues[d]);
    }

    vector<vector<double>> clusterCenters(numClusters, vector<double>(variables));
    for(int i = 0; i < numClusters; i++){
        for(int d = 0; d < variables; d++){
            clusterCenters[i][d] = distros[d](generator);
        }
    }

    return clusterCenters;
}

vector<vector<double>> Util::calculateRealClusterCoordinates(int numClusters) const{
    const auto& coordinates = solution.getClusterCoordinates();
    int variables = problem.getVariables();
    vector<vector<double>> clusterCenters(numClusters, vector<double>(variables, 0.0));

    for(int c = 0; c < numClusters; c++){ // esto si ya que son todos los clusters
        if(coordinates[c].empty()){
            continue;
        }

        vector<double> sum(variables, 0.0);
        for(const auto& point: coordinates[c]){
            for(int d = 0; d < variables; d++){
                sum[d] += point[d];
            }
        }

        for(int d = 0; d < variables; d++){
            clusterCenters[c][d] = sum[d] / coordinates[c].size();
        }
    }
    return clusterCenters;
}

double Util::distance(const vector<vector<double>>& A, const vector<vector<double>>& B) const{
    if(A.size() != B.size()) return -1.0;
    
    int variables = problem.getVariables();
    if(A.empty() || A[0].size() != variables || B[0].size() != variables) return -1.0;

    const auto& dataset = problem.getDataset();
    
    vector<pair<double, double>> minMaxValues(variables, {numeric_limits<double>::max(), numeric_limits<double>::lowest()});

    for(const auto& point : dataset){
        if(point.size() == variables){
            for(int d = 0; d < variables; d++){
                minMaxValues[d].first = min(minMaxValues[d].first, point[d]);
                minMaxValues[d].second = max(minMaxValues[d].second, point[d]);
            }
        }
    }

    vector<double> ranges(variables);
    for(int d = 0; d < variables; d++){
        ranges[d] = minMaxValues[d].second - minMaxValues[d].first;
        if(ranges[d] == 0) ranges[d] = 1.0;
    }

    double distance = 0.0;
    int D = A.size();

    for(int i = 0; i < D; i++){
        for(int d = 0; d < variables; d++){
            double normalizedDif = (A[i][d] - B[i][d]) / ranges[d];
            distance += normalizedDif * normalizedDif;
        }
    }

    return sqrt(distance) / sqrt(D * variables);
}

void Util::printBeforeClusterCenters() const {
    const auto& centers = solution.getBeforeClusterCenters();
    int variables = problem.getVariables();
    cout << "Centros de clusters (antes de asignacion):" << endl;
    for (int i = 0; i < centers.size(); i++) {
        cout << "  Cluster " << i << ": (";
        for(int d = 0; d < variables; d++){
            cout << fixed << setprecision(2) << centers[i][d];
            if(d < variables - 1) cout << ", ";
        }
        cout << ")" << endl;
    }
}

void Util::printAfterClusterCenters() const {
    const auto& centers = solution.getAfterClusterCenters();
    int variables = problem.getVariables();
    cout << "Centros de clusters (despues de asignacion):" << endl;
    for (int i = 0; i < centers.size(); i++) {
        cout << "  Cluster " << i << ": (";
        for(int d = 0; d < variables; d++){
            cout << fixed << setprecision(2) << centers[i][d];
            if(d < variables - 1) cout << ", ";
        }
        cout << ")" << endl;
    }
}
void Util::printAssignment() const {
    const auto& assignment = solution.getAssignment();
    int numClusters = problem.getNumClusters();
    cout << "Asignacion de puntos a clusters:" << endl;
    // Contar puntos por cluster
    vector<int> count(numClusters, 0);
    for (int c : assignment) {
        if (c >= 0 && c < numClusters) {
            count[c]++;
        }
    }
    for (int i = 0; i < count.size(); i++) {
        cout << "  Cluster " << i << ": " << count[i] << " puntos" << endl;
    }
}

void Util::printClusterCoordinates() const {
    const auto& coordinates = solution.getClusterCoordinates();
    int variables = problem.getVariables();
    cout << "Puntos por cluster:" << endl;
    
    for (int c = 0; c < coordinates.size(); c++) {
        cout << "  Cluster " << c << " (" << coordinates[c].size() << " puntos):" << endl;  
        
        // Limitar la salida si hay muchos puntos
        const int max_points_to_show = 5;
        for (int i = 0; i < min(max_points_to_show, (int)coordinates[c].size()); i++) {
            cout << "    (";
            for(int d = 0; d < variables; d++){
                cout << coordinates[c][i][d];
                if(d < variables - 1) cout << ", ";
            }
            cout << ")" << endl;
        }
        
        if (coordinates[c].size() > max_points_to_show) {
            cout << "    ... y " << (coordinates[c].size() - max_points_to_show) 
                 << " puntos mas" << endl;
        }
    }
}
void Util::printClusterValues() const {
    const auto& values = solution.getClusterValues();
    cout << "Valores de fitness por cluster:" << endl;
    for (int i = 0; i < values.size(); i++) {
        cout << "  Cluster " << i << ": " << values[i] << endl;
    }
}

void Util::printFitness() const {
    const auto& fitness = solution.getFitness();
    cout << "Fitness global: " << fitness << endl;
}

void Util::printDistance() const{
    const auto& distance = solution.getDistance();
    cout << "Distancia entre centros: " << distance << endl;
}

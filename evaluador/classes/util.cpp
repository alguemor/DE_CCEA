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

vector<pair<double, double>> Util::generateRandomCenters(int numClusters) const{
    const auto& dataset = problem.getDataset();
    int minValue = numeric_limits<int>::max();
    int maxValue = numeric_limits<int>::min();
    for(const auto& point : dataset){
        if(!point.empty()){
            auto min_it = min_element(point.begin(), point.end());
            auto max_it = max_element(point.begin(), point.end());
            if(min_it != point.end()) minValue = min(minValue, *min_it);
            if(max_it != point.end()) maxValue = max(maxValue, *max_it);
        }
    }

    uniform_int_distribution<int> range(minValue, maxValue);

    vector<pair<double, double>> clusterCenters;
    for(int i = 0; i < numClusters; i++){
        int x = range(generator);
        int y = range(generator);
        clusterCenters.push_back(make_pair(x, y));
    }

    return clusterCenters;
}

vector<pair<double, double>> Util::calculateRealClusterCoordinates(int numClusters) const{
    const auto& coordinates = solution.getClusterCoordinates();
    vector<pair<double, double>> clusterCenters(numClusters, {0.0, 0.0});
    for(int c = 0; c < numClusters; c++){ // esto si ya que son todos los clusters
        int dx = 0.0;
        int dy = 0.0;
        for(int i = 0; i < coordinates[c].size(); i++){
            dx += coordinates[c][i].first;
            dy += coordinates[c][i].second;
        }
        if(coordinates[c].size() > 0){ // evitar division por cero
            clusterCenters[c].first = dx / coordinates[c].size();
            clusterCenters[c].second = dy / coordinates[c].size();
        }else{
            clusterCenters[c].first = 0.0;
            clusterCenters[c].second = 0.0;
        }
    }
    return clusterCenters;
}

double Util::distance(const vector<pair<double, double>>& A, const vector<pair<double, double>>& B) const{
    if(A.size() != B.size()) return -1.0;
    // valores maximo y minimo
    const auto& dataset = problem.getDataset();
    const auto& var = problem.getVariables(); 
    vector<pair<int, int>> minMaxValues(var, {numeric_limits<int>::max(), numeric_limits<int>::min()});
    for(const auto& point : dataset){
        if(!point.empty()){
            if(point.size() == var){
                for(size_t i = 0; i < var; i++){
                    minMaxValues[i].first = min(minMaxValues[i].first, point[i]);
                    minMaxValues[i].second = max(minMaxValues[i].second, point[i]);
                }
            }
        }
    }
    vector<double> ranges(var);
    for(size_t i = 0; i < var; i++){
        ranges[i] = minMaxValues[i].second - minMaxValues[i].first;
    }
    double distance = 0.0;
    int D = A.size();

    for(size_t i = 0; i < D; i++){
        

        double a_first = A[i].first;
        double b_first = B[i].first;

        double dif_first = (a_first - b_first) / ranges[0];
        distance += dif_first * dif_first;

        double a_second = A[i].second;
        double b_second = B[i].second;

        double dif_second = (a_second - b_second) / ranges[1];
        distance += dif_second * dif_second;
    }

    return sqrt(distance) / sqrt(D);
}

void Util::printBeforeClusterCenters() const {
    const auto& centers = solution.getBeforeClusterCenters();
    cout << "Centros de clusters (antes de asignacion):" << endl;
    for (int i = 0; i < centers.size(); i++) {
        cout << "  Cluster " << i << ": (" 
             << centers[i].first << ", " 
             << centers[i].second << ")" << endl;
    }
}

void Util::printAfterClusterCenters() const {
    const auto& centers = solution.getAfterClusterCenters();
    cout << "Centros de clusters (despues de asignacion):" << endl;
    for (int i = 0; i < centers.size(); i++) {
        cout << "  Cluster " << i << ": (" 
             << fixed << setprecision(2) << centers[i].first << ", " 
             << fixed << setprecision(2) << centers[i].second << ")" << endl;
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
    cout << "Puntos por cluster:" << endl;
    for (int c = 0; c < coordinates.size(); c++) {
        cout << "  Cluster " << c << " (" << coordinates[c].size() << " puntos):" << endl;  
        // Limitar la salida si hay muchos puntos
        const int max_points_to_show = 5;
        for (int i = 0; i < min(max_points_to_show, (int)coordinates[c].size()); i++) {
            cout << "    (" << coordinates[c][i].first << ", " 
                 << coordinates[c][i].second << ")" << endl;
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

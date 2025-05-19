#include "problem.h"
#include <iostream>
using namespace std;

// constructor : inicializa rutas archivos
Problem::Problem(const string& fileDataset, const string& fileClusters)
    : Dataset(fileDataset), Clusters(fileClusters),
    numClusters(0), points(0), variables(0), dataLoaded(false){
}
// destructor
Problem::~Problem(){    
}

void Problem::loadClusters(){
    ifstream fileClusters(Clusters);
    fileClusters >> numClusters;
    for(int i = 0; i < numClusters; i++){
        int lim;
        fileClusters >> lim;
        limClusters.push_back(lim);
    }
    fileClusters.close();
    //printVector1D(limClusters);
}

void Problem::loadDataset(){
    ifstream fileDataset(Dataset);
    fileDataset >> points;
    fileDataset >> variables;
    dataset.clear();
    dataset.resize(points, vector<int>(variables, 0));
    for(int i = 0; i < points; i++){
        for(int j = 0; j < variables; j++){
            fileDataset >> dataset[i][j];
        }
    }
    fileDataset.close();
    //printVector2D(dataset);
}

void Problem::loadData(){
    if(!dataLoaded){ // no acumulacion de datos
        limClusters.clear();
        loadClusters();
        loadDataset();
        dataLoaded = true;
    }
}

int Problem::getNumClusters() const{
    return numClusters;
}

const vector<int>& Problem::getLimClusters() const{
    return limClusters;
}

const vector<vector<int>>& Problem::getDataset() const{
    return dataset;
}

int Problem::getPoints() const{
    return points;
}

int Problem::getVariables() const{
    return variables;
}
// fitness
vector<vector<pair<int, int>>> Problem::calculatePointsCoordinatesPerCluster(const vector<int>& assignment) const{
    int numClusters = getNumClusters();
    vector<vector<pair<int, int>>> coordinates(numClusters);
    if(assignment.size() != points){
        return coordinates;
    }
    for(int i = 0; i < assignment.size(); i++){
        int cluster = assignment[i];
        if(cluster >= 0 && cluster < numClusters){
            int x = dataset[i][0];
            int y = dataset[i][1];
            coordinates[cluster].push_back(make_pair(x, y));
        }
    }
    return coordinates;
}

vector<double> Problem::calculateClusterValues(const vector<vector<pair<int, int>>>& coordinates) const{
    int numClusters = getNumClusters();
    vector<double> values(numClusters, 0.0);
    for(int c = 0; c < numClusters; c++){
        if(coordinates[c].empty()) continue;
        double sumDistances = 0.0;
        for(int i = 0; i < coordinates[c].size(); i++){
            for(int j = i + 1; j < coordinates[c].size(); j++){
                int x1 = coordinates[c][i].first;
                int y1 = coordinates[c][i].second;
                int x2 = coordinates[c][j].first;
                int y2 = coordinates[c][j].second;
                int dx = x2 - x1;
                int dy = y2 - y1;
                double distance = sqrt(dx*dx + dy*dy);
                sumDistances += distance;
            }
        }
        if(coordinates[c].size() > 0){ // evitar division por cero
            values[c] = sumDistances / coordinates[c].size();
        }else{
            values[c] = 0.0;
        }
    }
    return values;
}

double Problem::calculateFitness(const vector<double>& clusterValues) const {
    return accumulate(clusterValues.begin(), clusterValues.end(), 0.0);
}

double Problem::evaluateSolution(const vector<int>& assignment,
                                vector<vector<pair<int, int>>>& coordinates,
                                vector<double>& clusterValues) const{
    coordinates = calculatePointsCoordinatesPerCluster(assignment);
    clusterValues = calculateClusterValues(coordinates);
    return calculateFitness(clusterValues);
                                }

void Problem::printVector1D(const vector<int>& vector) const{
    cout << "[";
    for(size_t i = 0; i < vector.size(); i++) {
        cout << vector[i];
        if(i < vector.size() - 1) cout << ", ";
    }
    cout << "]" << endl;
}

void Problem::printVector2D(const vector<vector<int>>& vector) const{
    cout << "[" << endl;
    for(size_t i = 0; i < vector.size(); i++){
        cout << "  [";
        for(size_t j = 0; j < vector[i].size(); j++){
            cout << vector[i][j];
            if(j < vector[i].size() - 1){
                cout << ", ";
            }
        }
        cout << "]";
        if(i < vector.size() - 1){
            cout << ",";
        }
        cout << endl;
    }
    cout << "]" << endl;
}
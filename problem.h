#ifndef problem_h
#define problem_h
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <numeric>
using namespace std;

class Problem{
private:
    // rutas archivos
    string Dataset;
    string Clusters;
    // almacenamiento datos
    int numClusters;
    vector<int> limClusters;
    vector<vector<double>> dataset;
    int points;
    int variables;
    vector<vector<vector<double>>> coordinates;
    // metodos auxiliares
    void loadClusters();
    void loadDataset();
    bool dataLoaded; // flag carga

public:
    // constructor y destructor
    Problem(const string& fileDataset, const string& fileClusters);
    ~Problem();
    // metodos almacenamiento
    int getNumClusters() const;
    const vector<int>& getLimClusters() const;
    const vector<vector<double>>& getDataset() const;
    int getPoints() const;
    int getVariables() const;
    const vector<vector<vector<double>>>& getClustersCoordinates() const;
    // metodos impresion
    void printVector1D(const vector<int>& vector) const;
    void printVector2D(const vector<vector<double>>& vector) const;
    // metodo carga dataset
    void loadData();
    // metodos de evaluacion
    vector<vector<vector<double>>> calculatePointsCoordinatesPerCluster(const vector<int>& assignment) const;
    vector<double> calculateClusterValues(const vector<vector<vector<double>>>& coordinates) const;
    double calculateFitness(const vector<double>& clusterValues) const;
    // evaluacion completa
    double evaluateSolution(const vector<int>& assignment,
                            vector<vector<vector<double>>>& coordinates,
                            vector<double>& clusterValues) const;
    // metodo distancia entre vectores de soluciones de clusters
    double distance(const vector<vector<vector<double>>>& vectorA,
                    const vector<vector<vector<double>>>& vectorB) const;
};

#endif

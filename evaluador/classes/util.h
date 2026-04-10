#ifndef util_h
#define util_h
#include "problem.h"
#include <vector>
#include <utility>
#include <random>
#include <cmath>
using namespace std;

class Solution; // forward declaration - evita inclsuiones circulares

class Util{
private:
    const Problem& problem;
    const Solution& solution;
    mutable mt19937 generator; // motor numeros aleatorios - generacion aleatoria centroides

    public:
    Util(const Problem& prob, const Solution&sol);
    ~Util();
    // numeros / centros aleatorios
    vector<pair<double, double>> generateRandomCenters(int numClusters) const;
    vector<pair<double, double>> calculateRealClusterCoordinates(int numClusters) const;
    // distancia entre centros
    double distance(const vector<pair<double, double>>& A, const vector<pair<double, double>>& B) const;
    void printDistance() const;
    // impresiones
    void printBeforeClusterCenters() const;
    void printAfterClusterCenters() const; // despues de la asignacion - distancia
    void printAssignment() const;
    void printClusterCoordinates() const;
    void printClusterValues() const;
    void printFitness() const;

    void printVector1D(const vector<int>& vector) const;
    void printVector2D(const vector<vector<int>>& vector) const;
};

#endif
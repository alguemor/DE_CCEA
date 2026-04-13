#ifndef LOCAL_SEARCH_H
#define LOCAL_SEARCH_H

#include "de.h"
#include "problem.h"
#include <vector>

using std::vector;

// Estado incremental por cluster: permite calcular delta de fitness en O(D)
struct LSClusterStats {
    vector<long double> sum;  // sum_i x[i]   (vector D-dim)
    long double sumSq;        // sum_i ||x[i]||^2 (escalar)
    long double V;            // valor actual: sumSq - ||sum||^2 / size
    int size;                 // numero de puntos (fijo con swap 1:1)
};

class ClusteringBridge;

class LocalSearch {
public:
    LocalSearch(ClusteringBridge* bridge, int& nfes_ref, int max_evals,
                int num_clusters, int num_points, int dimension);

    // Aplica busqueda local best-improvement al individuo dado.
    // Recibe: centros DE actuales (individual), fitness inicial, asignacion inicial.
    // Si mejora: actualiza individual con nuevos centroides y retorna nuevo fitness.
    // Si no mejora: retorna current_fitness sin modificar individual.
    Fitness apply(Individual individual, Fitness current_fitness,
                  const vector<int>& initial_assignment,
                  int generation);

    // Retorna la asignacion resultante de la ultima llamada a apply()
    const vector<int>& getLastAssignment() const { return assignment_; }

private:
    ClusteringBridge* bridge_;
    const Problem*    problem_;
    int&  nfes_;
    int   max_evals_;
    int   K_, N_, D_;
    const vector<vector<long double>>* dataset_;

    // Estado de trabajo (se reinicia en cada llamada a apply)
    vector<LSClusterStats> stats_;
    vector<int>            assignment_;
    vector<vector<int>>    clusterPoints_;  // clusterPoints_[c] = indices de puntos en c

    void        initStats(const vector<int>& assignment);
    long double computeSwapDelta(int p1, int clA, int p2, int clB) const;
    void        applySwap(int p1, int clA, int p2, int clB);
    long double currentFitness() const;
    void        updateCentroidsInIndividual(Individual individual) const;
};

#endif // LOCAL_SEARCH_H

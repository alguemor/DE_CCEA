#include "bridge.h"
using namespace std;

ClusteringBridge::ClusteringBridge(const string& datasetFile, const string& clustersFile) {
    problem = new Problem(datasetFile, clustersFile);
    problem->loadData();
}

ClusteringBridge::~ClusteringBridge() {
    delete problem;
}

void ClusteringBridge::individualToSolution(Individual individual, Solution*& solution) {
    if (solution == nullptr) {
        solution = new Solution(*problem);
    }
    
    int numClusters = problem->getNumClusters();
    int variables = problem->getVariables();
    
    // Convertir el individuo a centros de clusters
    solution->beforeClusterCenters.clear();
    solution->beforeClusterCenters.resize(numClusters, vector<double>(variables));

    for (int i = 0; i < numClusters; i++) {
        for(int d = 0; d < variables; d++){
            solution->beforeClusterCenters[i][d] = individual[i * variables + d];
        } 
    }
    
    // calcular distancias desde cada punto a los centros
    solution->calculateDistances();
    solution->sortDistances();
}

Fitness ClusteringBridge::evaluateIndividual(Individual individual) {
    Solution* solution = nullptr;
    individualToSolution(individual, solution);
  
    //cout << "DEBUG: Primer centro del evolutivo: " << solution->beforeClusterCenters[0][0] << endl;
    // DEBUG: Verificar centros del evolutivo
    static int eval_count = 0;
    if(eval_count < 3) {  // Solo primeras 3 evaluaciones
        cout << "\n=== EVAL " << eval_count << " - CENTROS DEL EVOLUTIVO ===" << endl;
        for(int i = 0; i < solution->beforeClusterCenters.size(); i++) {
            cout << "Cluster " << i << ": ";
            for(int d = 0; d < solution->beforeClusterCenters[i].size(); d++) {
                cout << solution->beforeClusterCenters[i][d] << " ";
            }
            cout << endl;
        }
    }

    // se cambio desde individualToSolution
    solution->greedy();
    solution->updateEvaluation();

    Fitness fitness = solution->getFitness();
   
    // DEBUG: Verificar centros finales calculados
    if(eval_count < 3) {
        Util util(*problem, *solution);
        auto realCenters = util.calculateRealClusterCoordinates(problem->getNumClusters());
        cout << "=== CENTROS FINALES CALCULADOS ===" << endl;
        for(int i = 0; i < realCenters.size(); i++) {
            cout << "Cluster " << i << ": ";
            for(int d = 0; d < realCenters[i].size(); d++) {
                cout << realCenters[i][d] << " ";
            }
            cout << endl;
        }
        cout << "Fitness: " << fitness << endl;
        eval_count++;
    }

    //cout << fitness << endl;

    static Fitness bestFitnessSeenSoFar = numeric_limits<Fitness>::max();
    if(fitness < bestFitnessSeenSoFar){
        bestFitnessSeenSoFar = fitness;
        Util util(*problem, *solution);
        bestAfterCenters = util.calculateRealClusterCoordinates(problem->getNumClusters());
        
        // DEBUG: ¿Estamos guardando los correctos?
        cout << "\n*** NUEVA MEJOR SOLUCION - FITNESS: " << fitness << " ***" << endl;
        cout << "Guardando centros finales..." << endl;
    }

    delete solution;
    return fitness;
}

int ClusteringBridge::getNumClusters() const {
    return problem->getNumClusters();
}

int ClusteringBridge::getNumPoints() const {
    return problem->getPoints();
}

int ClusteringBridge::getDimension() const {
    return problem->getVariables();
}

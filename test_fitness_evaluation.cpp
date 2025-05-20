// test_fitness_evaluation.cpp
#include "de.h"
#include "dataset_manager.h"
#include "bridge.h"
#include <iostream>
#include <vector>
#include <climits>

// Declaración de variables globales para compatibilidad
double *OShift = nullptr, *M = nullptr, *y = nullptr, *z = nullptr, *x_bound = nullptr;
int ini_flag = 0, n_flag = 0, func_flag = 0, *SS = nullptr;
int g_function_number = 1;
int g_problem_size = 8; // 4 clusters * 2 dimensiones
unsigned int g_max_num_evaluations = 10000;
int g_pop_size = 10;
double g_Di = 0.1;
ClusteringBridge* g_clusteringBridge = nullptr;

int main() {
    // Inicializar gestor de datasets
    DatasetManager datasetManager;
    if (!datasetManager.datasetExists("ruspini")) {
        std::cerr << "Dataset 'ruspini' no encontrado.\n";
        return 1;
    }
    
    DatasetInfo datasetInfo = datasetManager.getDatasetInfo("ruspini");
    
    // Inicializar puente de clustering
    g_clusteringBridge = new ClusteringBridge(datasetInfo.pointsFile, datasetInfo.clustersFile);
    
    // Actualizar g_problem_size basado en el número de clusters
    g_problem_size = g_clusteringBridge->getNumClusters() * 2;
    
    // Crear un individuo aleatorio (centros de clusters)
    std::vector<double> individual(g_problem_size);
    const auto* problem = g_clusteringBridge->getProblem();
    const auto& dataset = problem->getDataset();
    
    // Encontrar los valores mínimos y máximos
    int minX = INT_MAX, minY = INT_MAX;
    int maxX = INT_MIN, maxY = INT_MIN;
    
    for (const auto& point : dataset) {
        if (point.size() >= 2) {
            minX = std::min(minX, point[0]);
            minY = std::min(minY, point[1]);
            maxX = std::max(maxX, point[0]);
            maxY = std::max(maxY, point[1]);
        }
    }
    
    // Generar centros aleatorios
    int numClusters = g_problem_size / 2;
    for (int i = 0; i < numClusters; i++) {
        individual[i * 2] = minX + (rand() / (double)RAND_MAX) * (maxX - minX);
        individual[i * 2 + 1] = minY + (rand() / (double)RAND_MAX) * (maxY - minY);
    }
    
    // Evaluar el individuo
    Fitness fitness = g_clusteringBridge->evaluateIndividual(individual.data());
    
    std::cout << "Evaluación de fitness para un individuo aleatorio:\n";
    std::cout << "Fitness: " << fitness << "\n\n";
    
    // Mostrar centros de clusters
    std::cout << "Centros de clusters generados:\n";
    for (int i = 0; i < numClusters; i++) {
        std::cout << "Cluster " << (i+1) << ": (" 
                  << individual[i * 2] << ", " 
                  << individual[i * 2 + 1] << ")\n";
    }
    
    // Limpiar memoria
    delete g_clusteringBridge;
    
    return 0;
}

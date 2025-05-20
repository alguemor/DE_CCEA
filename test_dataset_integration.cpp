#include "dataset_manager.h"
#include "bridge.h"
#include <iostream>

int main() {
    // Cargar configuración de datasets
    DatasetManager datasetManager;
    std::cout << "Datasets disponibles:" << std::endl;
    
    for (const auto& name : datasetManager.getAvailableDatasets()) {
        std::cout << "- " << name << std::endl;
    }
    
    // Probar carga de un dataset específico
    if (datasetManager.datasetExists("ruspini")) {
        DatasetInfo info = datasetManager.getDatasetInfo("ruspini");
        std::cout << "\nCargando dataset 'ruspini':" << std::endl;
        std::cout << "Archivo de puntos: " << info.pointsFile << std::endl;
        std::cout << "Archivo de clusters: " << info.clustersFile << std::endl;
        
        // Probar inicialización del puente
        ClusteringBridge bridge(info.pointsFile, info.clustersFile);
        std::cout << "Clusters: " << bridge.getNumClusters() << std::endl;
        std::cout << "Puntos: " << bridge.getNumPoints() << std::endl;
        std::cout << "Dimensiones: " << bridge.getDimension() << std::endl;
    } else {
        std::cout << "Dataset 'ruspini' no encontrado." << std::endl;
    }
    
    return 0;
}

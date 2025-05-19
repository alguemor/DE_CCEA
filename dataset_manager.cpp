#include "dataset_manager.h"
#include <fstream>
#include <iostream>
#include <sstream>

DatasetManager::DatasetManager(const std::string& configFile)
    : configFile(configFile) {
    loadConfig();
}

bool DatasetManager::loadConfig() {
    std::ifstream file(configFile);
    if (!file.is_open()) {
        std::cerr << "Error: No se pudo abrir el archivo de configuración: " << configFile << std::endl;
        return false;
    }
    
    datasets.clear();
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        std::istringstream iss(line);
        DatasetInfo info;
        
        if (iss >> info.name >> info.pointsFile >> info.clustersFile) {
            datasets[info.name] = info;
        }
    }
    
    return true;
}

DatasetInfo DatasetManager::getDatasetInfo(const std::string& name) const {
    auto it = datasets.find(name);
    if (it != datasets.end()) {
        return it->second;
    }
    
    return DatasetInfo();
}

bool DatasetManager::datasetExists(const std::string& name) const {
    return datasets.find(name) != datasets.end();
}

std::vector<std::string> DatasetManager::getAvailableDatasets() const {
    std::vector<std::string> names;
    for (const auto& pair : datasets) {
        names.push_back(pair.first);
    }
    return names;
}

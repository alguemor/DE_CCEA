#ifndef DATASET_MANAGER_H
#define DATASET_MANAGER_H

#include <string>
#include <map>
#include <vector>

struct DatasetInfo {
    std::string name;
    std::string pointsFile;
    std::string clustersFile;
};

class DatasetManager {
private:
    std::map<std::string, DatasetInfo> datasets;
    std::string configFile;
    
public:
    DatasetManager(const std::string& configFile = "datasets.cfg");
    
    bool loadConfig();
    
    DatasetInfo getDatasetInfo(const std::string& name) const;
    
    bool datasetExists(const std::string& name) const;
    
    std::vector<std::string> getAvailableDatasets() const;
};

#endif

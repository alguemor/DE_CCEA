#include "bridge.h"
using namespace std;

ClusteringBridge::ClusteringBridge(const string& datasetFile, const string& clustersFile, const string& method) 
    : solverMethod(method){
    problem = new Problem(datasetFile, clustersFile);
    problem->loadData();
}

ClusteringBridge::~ClusteringBridge(){
    delete problem;
}

void ClusteringBridge::individualToSolution(Individual individual, Solution*& solution){
    // method router
    if(solution == nullptr){
        if(solverMethod == "greedy"){
            solution = new Greedy(*problem);
        }else if(solverMethod == "mcfp"){
            solution = new MCFP(*problem);
        }else{
            solution = new Greedy(*problem);
        }
    }
    
    int numClusters = problem->getNumClusters();
    int variables = problem->getVariables();
    
    // convertir el individuo a centros de clusters
    solution->beforeClusterCenters.clear();
    solution->beforeClusterCenters.resize(numClusters, vector<long double>(variables));

    for(int i = 0; i < numClusters; i++){
        for(int d = 0; d < variables; d++){
            solution->beforeClusterCenters[i][d] = individual[i * variables + d];
        } 
    }
    
    // calcular distancias (metodo compartido)
    solution->calculateDistances();
}

Fitness ClusteringBridge::evaluateIndividual(Individual individual){
    Solution* solution = nullptr;
    individualToSolution(individual, solution);
  
    //cout << "DEBUG: Primer centro del evolutivo: " << solution->beforeClusterCenters[0][0] << endl;
    // DEBUG: Verificar centros del evolutivo
    static int eval_count = 0;
    if(eval_count < 3){  // solo primeras 3 evaluaciones
        // cout << "\n=== EVAL " << eval_count << " - CENTROS DEL EVOLUTIVO ===" << endl;
        // for(int i = 0; i < solution->beforeClusterCenters.size(); i++) {
        //     cout << "Cluster " << i << ": ";
        //     for(int d = 0; d < solution->beforeClusterCenters[i].size(); d++) {
        //         cout << solution->beforeClusterCenters[i][d] << " ";
        //     }
        //     cout << endl;
        // }
    }

    // resolver dependiendo del metodo
    if(solverMethod == "greedy"){
        Greedy* greedySolution = static_cast<Greedy*>(solution);
        greedySolution->sortDistancesGreedy();
        greedySolution->greedy();
    }else if(solverMethod == "mcfp"){
        MCFP* mcfpSolution = static_cast<MCFP*>(solution);
        mcfpSolution->buildMCFPGraph();
        mcfpSolution->solveMCFPFlow();
        mcfpSolution->extractAssignmentFromFlow();
    }
    
    // actualizar evaluacion (metodo compartido)
    solution->updateEvaluation();

    Fitness fitness = solution->getFitness();
   
    // DEBUG: Verificar centros finales calculados
    if(eval_count < 3){
        eval_count++;
    }

    //cout << fitness << endl;

    static Fitness bestFitnessSeenSoFar = numeric_limits<Fitness>::max();
    if(fitness < bestFitnessSeenSoFar){
        bestFitnessSeenSoFar = fitness;
        Util util(*problem, *solution);
        
        // Store comprehensive best solution information
        bestFitness = fitness;
        bestAfterCenters = util.calculateRealClusterCoordinates(problem->getNumClusters());
        bestAssignment = solution->getAssignment();
        bestClusterValues = solution->getClusterValues();
        
        // Calculate point distances to their assigned cluster centers
        bestPointDistances.clear();
        const vector<vector<long double>>& dataset = problem->getDataset();
        bestPointDistances.resize(dataset.size());
        
        for(int i = 0; i < dataset.size(); i++) {
            int assignedCluster = bestAssignment[i];
            bestPointDistances[i].resize(1); // Store just the distance to assigned cluster
            
            // Calculate distance from point i to its assigned cluster center
            long double dist = 0.0;
            for(int d = 0; d < bestAfterCenters[assignedCluster].size(); d++) {
                long double diff = dataset[i][d] - bestAfterCenters[assignedCluster][d];
                dist += diff * diff;
            }
            bestPointDistances[i][0] = sqrt(dist);
        }
        
        // DEBUG: ¿Estamos guardando los correctos?
        // cout << "\n*** NUEVA MEJOR SOLUCION - FITNESS: " << fitness << " ***" << endl;
        // cout << "Guardando informacion completa de la solucion..." << endl;
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

const vector<vector<long double>>& ClusteringBridge::getBestAfterCenters() const {
    return bestAfterCenters;
}

void ClusteringBridge::setBestAfterCenters(const vector<vector<long double>>& centers) {
    bestAfterCenters = centers;
}

const vector<int>& ClusteringBridge::getBestAssignment() const {
    return bestAssignment;
}

const vector<long double>& ClusteringBridge::getBestClusterValues() const {
    return bestClusterValues;
}

const vector<vector<long double>>& ClusteringBridge::getBestPointDistances() const {
    return bestPointDistances;
}

Fitness ClusteringBridge::getBestFitness() const {
    return bestFitness;
}

void ClusteringBridge::outputComprehensiveSolution(ofstream& outFile) const {
    const vector<vector<long double>>& dataset = problem->getDataset();
    const vector<int>& limClusters = problem->getLimClusters();
    
    outFile << "SOLUTION SUMMARY" << endl;
    outFile << "================" << endl;
    outFile << "Final Fitness: " << bestFitness << endl;
    outFile << "Number of Points: " << problem->getPoints() << endl;
    outFile << "Number of Clusters: " << problem->getNumClusters() << endl;
    outFile << "Problem Dimensions: " << problem->getVariables() << endl;
    outFile << endl;
    
    outFile << "DATASET POINTS" << endl;
    outFile << "==============" << endl;
    outFile << "Point_ID Coordinates..." << endl;
    for(int i = 0; i < dataset.size(); i++){
        outFile << i << " ";
        for(int d = 0; d < dataset[i].size(); d++){
            outFile << dataset[i][d] << " ";
        }
        outFile << endl;
    }
    outFile << endl;
    
    outFile << "CLUSTER CENTERS" << endl;
    outFile << "===============" << endl;
    outFile << "Cluster_ID Center_Coordinates... Capacity_Limit Assigned_Points Intra_Cluster_Sum_Squared_Distances" << endl;
    
    for(int c = 0; c < bestAfterCenters.size(); c++){
        outFile << c << " ";
        for(int d = 0; d < bestAfterCenters[c].size(); d++){
            outFile << bestAfterCenters[c][d] << " ";
        }
        
        // Count assigned points to this cluster
        int assignedCount = 0;
        for(int i = 0; i < bestAssignment.size(); i++){
            if(bestAssignment[i] == c) assignedCount++;
        }
        
        outFile << limClusters[c] << " " << assignedCount << " " << bestClusterValues[c] << endl;
    }
    outFile << endl;
    
    outFile << "POINT ASSIGNMENTS" << endl;
    outFile << "=================" << endl;
    outFile << "Point_ID Point_Coordinates... Assigned_Cluster Distance_to_Center" << endl;
    
    for(int i = 0; i < dataset.size(); i++){
        outFile << i << " ";
        for(int d = 0; d < dataset[i].size(); d++){
            outFile << dataset[i][d] << " ";
        }
        outFile << bestAssignment[i] << " " << bestPointDistances[i][0] << endl;
    }
    outFile << endl;
    
    outFile << "CLUSTER DETAILS" << endl;
    outFile << "===============" << endl;
    for(int c = 0; c < problem->getNumClusters(); c++){
        outFile << "Cluster_" << c << ": Points [";
        bool first = true;
        for(int i = 0; i < bestAssignment.size(); i++){
            if(bestAssignment[i] == c){
                if(!first) outFile << ",";
                outFile << i;
                first = false;
            }
        }
        outFile << "]" << endl;
    }
}

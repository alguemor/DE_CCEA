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

extern ofstream logFile;

Fitness ClusteringBridge::evaluateIndividual(Individual individual){
    Solution* solution = nullptr;
    individualToSolution(individual, solution);

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

    // Validate assignment: check all points are assigned
    const vector<int>& assignment = solution->getAssignment();
    int numPoints = problem->getPoints();
    int numClusters = problem->getNumClusters();
    int unassigned = 0;
    vector<int> clusterSizes(numClusters, 0);
    for(int i = 0; i < numPoints; i++) {
        if(assignment[i] < 0 || assignment[i] >= numClusters) {
            unassigned++;
        } else {
            clusterSizes[assignment[i]]++;
        }
    }

    // actualizar evaluacion (metodo compartido)
    solution->updateEvaluation();

    Fitness fitness = solution->getFitness();

    static Fitness bestFitnessSeenSoFar = numeric_limits<Fitness>::max();
    static int totalEvaluations = 0;
    static int evalsSinceLastImprovement = 0;
    totalEvaluations++;
    evalsSinceLastImprovement++;

    // Log fitness evaluations: first 50, then every 500, or on new best
    if(totalEvaluations <= 50 || (totalEvaluations % 500 == 0) || fitness < bestFitnessSeenSoFar) {
        logFile << "EVAL " << totalEvaluations << ": Fitness = " << fitness;
        if(fitness < bestFitnessSeenSoFar) logFile << " *** NEW BEST ***";
        if(unassigned > 0) logFile << " *** UNASSIGNED: " << unassigned << " ***";
        logFile << endl;
    }

    if(fitness < bestFitnessSeenSoFar){
        bestFitnessSeenSoFar = fitness;
        evalsSinceLastImprovement = 0;
        Util util(*problem, *solution);

        logFile << "=== NEW BEST SOLUTION DETAILS (Eval " << totalEvaluations << ") ===" << endl;
        logFile << "Fitness: " << fitness << endl;

        // Cluster sizes vs cardinality constraints
        const vector<int>& limits = problem->getLimClusters();
        logFile << "Cluster sizes (actual/limit): ";
        bool cardinalityViolation = false;
        for(int c = 0; c < numClusters; c++) {
            logFile << "C" << c << "=" << clusterSizes[c] << "/" << limits[c];
            if(clusterSizes[c] != limits[c]) {
                logFile << "!";
                cardinalityViolation = true;
            }
            logFile << " ";
        }
        logFile << endl;
        if(cardinalityViolation) {
            logFile << "*** CARDINALITY CONSTRAINT VIOLATION ***" << endl;
        }
        if(unassigned > 0) {
            logFile << "*** " << unassigned << " POINTS UNASSIGNED ***" << endl;
        }

        // Per-cluster fitness contribution
        const vector<long double>& clusterVals = solution->getClusterValues();
        logFile << "Per-cluster fitness: ";
        for(int c = 0; c < numClusters; c++) {
            logFile << "C" << c << "=" << clusterVals[c] << " ";
        }
        logFile << endl;

        // Store comprehensive best solution information
        bestFitness = fitness;
        bestAfterCenters = util.calculateRealClusterCoordinates(numClusters);
        bestAssignment = solution->getAssignment();
        bestClusterValues = solution->getClusterValues();

        // Calculate point distances to their assigned cluster centers
        bestPointDistances.clear();
        const vector<vector<long double>>& dataset = problem->getDataset();
        bestPointDistances.resize(dataset.size());

        for(size_t i = 0; i < dataset.size(); i++) {
            int assignedCluster = bestAssignment[i];
            bestPointDistances[i].resize(1);
            long double dist = 0.0;
            for(size_t d = 0; d < bestAfterCenters[assignedCluster].size(); d++) {
                long double diff = dataset[i][d] - bestAfterCenters[assignedCluster][d];
                dist += diff * diff;
            }
            bestPointDistances[i][0] = sqrt(dist);
        }

        logFile.flush();
    }

    // Stagnation warning
    if(evalsSinceLastImprovement > 0 && evalsSinceLastImprovement % 10000 == 0) {
        logFile << "*** STAGNATION WARNING: " << evalsSinceLastImprovement
                << " evals without improvement (best=" << bestFitnessSeenSoFar << ") ***" << endl;
        logFile.flush();
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

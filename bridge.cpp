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

    // resolver dependiendo del metodo
    if(solverMethod == "greedy"){
        Greedy* greedySolution = static_cast<Greedy*>(solution);
        greedySolution->sortDistancesGreedy();
        greedySolution->greedy();
    }else if(solverMethod == "mcfp"){
        MCFP* mcfpSolution = static_cast<MCFP*>(solution);
        struct timeval t0, t1, t2, t3;
        gettimeofday(&t0, NULL);
        mcfpSolution->buildMCFPGraph();
        gettimeofday(&t1, NULL);
        mcfpSolution->solveMCFPFlow();
        gettimeofday(&t2, NULL);
        mcfpSolution->extractAssignmentFromFlow();
        gettimeofday(&t3, NULL);
        static int _evalCount = 0; _evalCount++;
        if(_evalCount <= 5 || _evalCount % 500 == 0) {
            double tBuild   = (t1.tv_sec-t0.tv_sec) + (t1.tv_usec-t0.tv_usec)/1e6;
            double tSolve   = (t2.tv_sec-t1.tv_sec) + (t2.tv_usec-t1.tv_usec)/1e6;
            double tExtract = (t3.tv_sec-t2.tv_sec) + (t3.tv_usec-t2.tv_usec)/1e6;
            LOG_DEBUG("MCFP timing eval=" << _evalCount
                      << " build=" << tBuild << "s"
                      << " solve=" << tSolve << "s"
                      << " extract=" << tExtract << "s");
        }
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
        std::ostringstream _oss;
        _oss << "EVAL " << totalEvaluations << ": Fitness = " << fitness;
        if(fitness < bestFitnessSeenSoFar) _oss << " *** NEW BEST ***";
        if(unassigned > 0) _oss << " *** UNASSIGNED: " << unassigned << " ***";
        g_logger.info(_oss.str());
    }

    if(fitness < bestFitnessSeenSoFar){
        bestFitnessSeenSoFar = fitness;
        evalsSinceLastImprovement = 0;
        Util util(*problem, *solution);

        LOG_INFO("=== NEW BEST SOLUTION DETAILS (Eval " << totalEvaluations << ") ===");
        LOG_INFO("Fitness: " << fitness);

        // Cluster sizes vs cardinality constraints
        const vector<int>& limits = problem->getLimClusters();
        { std::ostringstream _oss;
          _oss << "Cluster sizes (actual/limit): ";
          bool cardinalityViolation = false;
          for(int c = 0; c < numClusters; c++) {
              _oss << "C" << c << "=" << clusterSizes[c] << "/" << limits[c];
              if(clusterSizes[c] != limits[c]) { _oss << "!"; cardinalityViolation = true; }
              _oss << " ";
          }
          g_logger.info(_oss.str());
          if(cardinalityViolation) LOG_WARNING("*** CARDINALITY CONSTRAINT VIOLATION ***");
        }
        if(unassigned > 0) LOG_WARNING("*** " << unassigned << " POINTS UNASSIGNED ***");

        // Per-cluster fitness contribution
        const vector<long double>& clusterVals = solution->getClusterValues();
        { std::ostringstream _oss;
          _oss << "Per-cluster fitness: ";
          for(int c = 0; c < numClusters; c++) _oss << "C" << c << "=" << clusterVals[c] << " ";
          g_logger.info(_oss.str());
        }

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

        g_logger.flush();
    }

    // Stagnation warning at 1000, 5000, 10000 evals without improvement
    if(evalsSinceLastImprovement == 1000 || evalsSinceLastImprovement == 5000 ||
       evalsSinceLastImprovement == 10000) {
        LOG_WARNING("STAGNATION: " << evalsSinceLastImprovement
                    << " evals without improvement (best=" << bestFitnessSeenSoFar << ")");
        g_logger.flush();
    }

    delete solution;
    return fitness;
}

Fitness ClusteringBridge::evaluateAndRetainAssignment(Individual individual,
                                                       vector<int>& outAssignment) {
    Solution* solution = nullptr;
    individualToSolution(individual, solution);

    if(solverMethod == "greedy"){
        Greedy* g = static_cast<Greedy*>(solution);
        g->sortDistancesGreedy();
        g->greedy();
    } else if(solverMethod == "mcfp"){
        MCFP* m = static_cast<MCFP*>(solution);
        m->buildMCFPGraph();
        m->solveMCFPFlow();
        m->extractAssignmentFromFlow();
    }

    solution->updateEvaluation();
    Fitness fit = solution->getFitness();

    // Retener la asignacion antes de eliminar la solucion
    outAssignment = solution->getAssignment();

    delete solution;
    return fit;
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

void ClusteringBridge::setBestFitnessFromLS(Fitness fit, const vector<int>& assignment,
                                             Individual individual) {
    bestFitness    = fit;
    bestAssignment = assignment;

    // Recalcular centroides reales desde la asignacion mejorada por LS
    int K = problem->getNumClusters();
    int D = problem->getVariables();
    const auto& dataset = problem->getDataset();

    bestAfterCenters.assign(K, vector<long double>(D, 0.0L));
    vector<int> counts(K, 0);
    for(int p = 0; p < (int)assignment.size(); p++){
        int c = assignment[p];
        if(c < 0 || c >= K) continue;
        for(int d = 0; d < D; d++)
            bestAfterCenters[c][d] += dataset[p][d];
        counts[c]++;
    }
    for(int c = 0; c < K; c++)
        if(counts[c] > 0)
            for(int d = 0; d < D; d++)
                bestAfterCenters[c][d] /= counts[c];

    // Recalcular clusterValues y pointDistances para consistencia
    Solution* sol = nullptr;
    individualToSolution(individual, sol);
    sol->assignment = assignment;   // inyectar asignacion LS
    sol->updateEvaluation();
    bestClusterValues = sol->getClusterValues();
    delete sol;

    // Recalcular distancias de puntos a centroides
    bestPointDistances.clear();
    bestPointDistances.resize(dataset.size());
    for(size_t i = 0; i < dataset.size(); i++){
        int c = bestAssignment[i];
        long double dist = 0.0L;
        for(int d = 0; d < D; d++){
            long double diff = dataset[i][d] - bestAfterCenters[c][d];
            dist += diff * diff;
        }
        bestPointDistances[i] = { sqrtl(dist) };
    }

    LOG_INFO("LS_BRIDGE_UPDATE fitness=" << fit);
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

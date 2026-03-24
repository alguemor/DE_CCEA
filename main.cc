/*
	Author: Joel Chacon Castillo
	Email: joel.chacon@cimat.mx
	Code based in the DE competition CEC 2017
*/

/*
  L-SHADE implemented by C++ for Special Session & Competition on Real-Parameter Single Objective Optimization at CEC-2014
  See the details of L-SHADE in the following paper:

  * Ryoji Tanabe and Alex Fukunaga: Improving the Search Performance of SHADE Using Linear Population Size Reduction,  Proc. IEEE Congress on Evolutionary Computation (CEC-2014), Beijing, July, 2014.
  
  Version: 1.0   Date: 16/Apr/2014
  Written by Ryoji Tanabe (rt.ryoji.tanabe [at] gmail.com)
*/

#include "de.h"
#include "dataset_manager.h"
#include "bridge.h"
#include <iostream>
#include <string>
#include <sys/time.h>

double *OShift,*M,*y,*z,*x_bound;
int ini_flag=0,n_flag,func_flag,*SS;

int g_function_number;
int g_problem_size;
unsigned int g_max_num_evaluations;

int g_pop_size;
double g_arc_rate;
int g_memory_size;
double g_p_best_rate;

ofstream outFile;
ofstream logFile;
char g_fileName[1000];
double g_Di;

ClusteringBridge* g_clusteringBridge = nullptr;

//.Exec sed problem Di
int main(int argc, char **argv) {
 
    int sed = atoi(argv[1]);
    std::string datasetName = argv[2];
    double di = atof(argv[3]);
    std::string method = argv[4];
    //available number of fitness evaluations
    g_max_num_evaluations = (argc > 5) ? atoi(argv[5]) : 100000;

    DatasetManager datasetManager;
    if(!datasetManager.datasetExists(datasetName)){
        std::cerr << "Error: dataset no existe\n";
        return 1;
    }

    DatasetInfo datasetInfo = datasetManager.getDatasetInfo(datasetName);

    g_clusteringBridge = new ClusteringBridge(datasetInfo.pointsFile, datasetInfo.clustersFile, method);

    int numClusters = g_clusteringBridge->getNumClusters();
    int variables = g_clusteringBridge->getDimension();
    g_problem_size = numClusters * variables;
    
    g_pop_size = 200;

    srand(sed);

    //g_function_number = 1;
    g_Di = sqrt(g_problem_size)*di;

    sprintf(g_fileName, "results/%s/s%d_p%d_%s_di%.1f", datasetName.c_str(), sed, g_problem_size, method.c_str(), di);

    // Open output file for writing the final result
    char outputFileName[1000];
    sprintf(outputFileName, "%s_result.txt", g_fileName);
    outFile.open(outputFileName, ios::out);

    // Open log file for debugging information
    char logFileName[1000];
    sprintf(logFileName, "results/logs/%s_s%d_p%d_%s_di%.1f_debug.log", datasetName.c_str(), sed, g_problem_size, method.c_str(), di);
    logFile.open(logFileName, ios::out);

    // Timing
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);

    // Log initial algorithm setup
    logFile << "=== DE-CCEA ALGORITHM DEBUG LOG ===" << endl;
    logFile << "Dataset: " << datasetName << endl;
    logFile << "Method: " << method << endl;
    logFile << "Seed: " << sed << endl;
    logFile << "Problem Size: " << g_problem_size << endl;
    logFile << "Population Size: " << g_pop_size << endl;
    logFile << "Max Evaluations: " << g_max_num_evaluations << endl;
    logFile << "Diversity Parameter (Di): " << g_Di << endl;
    logFile << "Number of Clusters: " << numClusters << endl;
    logFile << "Variables per Cluster: " << variables << endl;
    logFile << "Number of Points: " << g_clusteringBridge->getNumPoints() << endl;

    // Log cardinality constraints
    const vector<int>& limits = g_clusteringBridge->getProblem()->getLimClusters();
    logFile << "Cardinality Constraints: ";
    int totalCapacity = 0;
    for(int c = 0; c < limits.size(); c++) {
        logFile << "C" << c << "=" << limits[c] << " ";
        totalCapacity += limits[c];
    }
    logFile << endl;
    logFile << "Total Capacity: " << totalCapacity << " (Points: " << g_clusteringBridge->getNumPoints() << ")";
    if(totalCapacity != g_clusteringBridge->getNumPoints()) {
        logFile << " *** MISMATCH ***";
    }
    logFile << endl;
    logFile << "Start Time: " << start_time.tv_sec << "." << start_time.tv_usec << endl;
    logFile << "=================================" << endl << endl;
    logFile.flush();

    searchAlgorithm *alg = new DIVERSITY();
    alg->run();

    // Timing end
    gettimeofday(&end_time, NULL);
    double elapsed_time = (end_time.tv_sec - start_time.tv_sec) +
                         (end_time.tv_usec - start_time.tv_usec) / 1000000.0;

    // Output comprehensive solution information
    g_clusteringBridge->outputComprehensiveSolution(outFile);
    delete alg;

    // Log execution time
    logFile << endl << "=================================" << endl;
    logFile << "End Time: " << end_time.tv_sec << "." << end_time.tv_usec << endl;
    logFile << "Total Execution Time: " << elapsed_time << " seconds" << endl;
    logFile << "=================================" << endl;
    logFile.flush();

    outFile << endl << "Execution Time: " << elapsed_time << " seconds" << endl;

    // Close files
    outFile.close();
    logFile.close();

    //free memory of the benchmark
	    delete g_clusteringBridge;
        free(M);
		free(OShift);
		free(y);
		free(z);
		free(x_bound);
    return 0;
}


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
#include <cstdio>
#include <cstring>
#include <sys/stat.h>

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
char g_fileName[1000];
double g_Di;

ClusteringBridge* g_clusteringBridge = nullptr;

static long readRssKb() {
    long rss = 0;
    FILE* fp = fopen("/proc/self/status", "r");
    if (!fp) return 0;
    char line[128];
    while (fgets(line, sizeof(line), fp))
        if (strncmp(line, "VmRSS:", 6) == 0) { sscanf(line + 6, "%ld", &rss); break; }
    fclose(fp);
    return rss;
}

//.Exec sed problem Di
int main(int argc, char **argv) {
 
    int sed = atoi(argv[1]);
    std::string datasetName = argv[2];
    double di = atof(argv[3]);
    std::string method = argv[4];
    //available number of fitness evaluations
    g_max_num_evaluations = (argc > 5) ? atoi(argv[5]) : 100000;
    std::string results_dir = (argc > 6) ? argv[6] : "results";

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

    // Ensure output directories exist
    { char d[1000];
      mkdir(results_dir.c_str(), 0755);
      sprintf(d, "%s/%s", results_dir.c_str(), datasetName.c_str()); mkdir(d, 0755);
      sprintf(d, "%s/logs", results_dir.c_str()); mkdir(d, 0755); }

    sprintf(g_fileName, "%s/%s/s%d_p%d_%s_di%.1f", results_dir.c_str(), datasetName.c_str(), sed, g_problem_size, method.c_str(), di);

    // Open output file for writing the final result
    char outputFileName[1000];
    sprintf(outputFileName, "%s_result.txt", g_fileName);
    outFile.open(outputFileName, ios::out);

    // Open log file for debugging information
    char logFileName[1000];
    sprintf(logFileName, "%s/logs/%s_s%d_p%d_%s_di%.1f_debug.log", results_dir.c_str(), datasetName.c_str(), sed, g_problem_size, method.c_str(), di);
    g_logger.open(logFileName);

    // Timing
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);

    // Log initial algorithm setup
    long rssStart = readRssKb();
    LOG_INFO("=== DE-CCEA ALGORITHM DEBUG LOG ===");
    LOG_INFO("Dataset: " << datasetName);
    LOG_INFO("Method: " << method);
    LOG_INFO("Seed: " << sed);
    LOG_INFO("Problem Size: " << g_problem_size);
    LOG_INFO("Population Size: " << g_pop_size);
    LOG_INFO("Max Evaluations: " << g_max_num_evaluations);
    LOG_INFO("Diversity Parameter (Di): " << g_Di);
    LOG_INFO("Number of Clusters: " << numClusters);
    LOG_INFO("Variables per Cluster: " << variables);
    LOG_INFO("Number of Points: " << g_clusteringBridge->getNumPoints());
    LOG_INFO("Results Dir: " << results_dir);

    // Log cardinality constraints
    const vector<int>& limits = g_clusteringBridge->getProblem()->getLimClusters();
    { std::ostringstream _cardOss;
      _cardOss << "Cardinality Constraints: ";
      int totalCapacity = 0;
      for(int c = 0; c < (int)limits.size(); c++) {
          _cardOss << "C" << c << "=" << limits[c] << " ";
          totalCapacity += limits[c];
      }
      g_logger.info(_cardOss.str());
      std::ostringstream _capOss;
      _capOss << "Total Capacity: " << totalCapacity
              << " (Points: " << g_clusteringBridge->getNumPoints() << ")";
      if(totalCapacity != g_clusteringBridge->getNumPoints()) _capOss << " *** MISMATCH ***";
      g_logger.info(_capOss.str());
    }
    LOG_INFO("Start Time: " << start_time.tv_sec << "." << start_time.tv_usec);
    LOG_INFO("Memory RSS at start: " << rssStart << " kB");
    LOG_INFO("=================================");
    g_logger.flush();

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
    long rssEnd = readRssKb();
    LOG_INFO("=================================");
    LOG_INFO("End Time: " << end_time.tv_sec << "." << end_time.tv_usec);
    LOG_INFO("Total Execution Time: " << elapsed_time << " seconds");
    LOG_INFO("Memory RSS at end: " << rssEnd << " kB");
    LOG_INFO("Memory RSS delta: " << (rssEnd - rssStart) << " kB");
    LOG_INFO("=================================");
    g_logger.flush();

    outFile << endl << "Execution Time: " << elapsed_time << " seconds" << endl;

    // Close files
    outFile.close();
    g_logger.close();

    //free memory of the benchmark
	    delete g_clusteringBridge;
        free(M);
		free(OShift);
		free(y);
		free(z);
		free(x_bound);
    return 0;
}


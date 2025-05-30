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

//.Exec sed problem Di
int main(int argc, char **argv) {
 
    int sed = atoi(argv[1]);
    std::string datasetName = argv[2];
    //available number of fitness evaluations 
    g_max_num_evaluations = 25000;

    // raw data: Record function error value (Fi(x)-Fi(x*)) after (0.01, 0.02, 0.03, 0.05, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0)*MaxFES for each run.
    double di = atof(argv[3]);

    DatasetManager datasetManager;
    if(!datasetManager.datasetExists(datasetName)){
        std::cerr << "Error: dataset no existe\n";
        return 1;
    }

    DatasetInfo datasetInfo = datasetManager.getDatasetInfo(datasetName);

    g_clusteringBridge = new ClusteringBridge(datasetInfo.pointsFile, datasetInfo.clustersFile);

    int numClusters = g_clusteringBridge->getNumClusters();
    int variables = g_clusteringBridge->getDimension();
    g_problem_size = numClusters * variables;
    
    //g_pop_size = 10000;
    g_pop_size = (int)round(sqrt(g_problem_size) * log(g_problem_size) * 50);
    //std::cout << g_pop_size << std::endl;

    srand(sed);

    //g_function_number = 1;
    g_Di = sqrt(g_problem_size)*di;

    sprintf(g_fileName, "results/%s/s%d_p%d", datasetName.c_str(), sed, g_problem_size);

    searchAlgorithm *alg = new DIVERSITY();
    outFile << alg->run();
    delete alg;

    //free memory of the benchmark
	    delete g_clusteringBridge;
        free(M);
		free(OShift);
		free(y);
		free(z);
		free(x_bound);
    return 0;
}


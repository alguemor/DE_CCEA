#include "problem.h"
#include "solution.h"
#include <iostream>
#include <string>

using namespace std;

int main(int argc, char* argv[]){
    if (argc < 4) {
        return 1;
    }
    
    string method = argv[4];
    
    Problem problema(argv[1], argv[2]);
    problema.loadData();
    
    // eleccion metodo
    if (method == "greedy") {
        Greedy solver(problema);
        solver.solveGreedy();
    } else if (method == "mcfp") {
        MCFP solver(problema);
        solver.solveMCFP();
    } else {
        return 1;
    }
    
    return 0;
}
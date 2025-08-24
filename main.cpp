#include "problem.h"
#include "solution.h"
#include <iostream>

int main(int argc, char* argv[]){
    
    Problem problema(argv[1], argv[2]);
    problema.loadData();

    Solution solucion(problema);
    solucion.solveGreedy();

    return 0;
}
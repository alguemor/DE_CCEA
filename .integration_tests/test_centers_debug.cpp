#include "problem.h"
#include "solutionGreedy.h"
#include "util.h"
#include <iostream>

int main(){
    Problem problema("input_data/iris/04iris.txt", "input_data/iris/04clusters.txt");
    problema.loadData();
    
    cout << "=== RANGOS DEL DATASET ===" << endl;
    const auto& dataset = problema.getDataset();
    int variables = problema.getVariables();
    
    vector<double> minVals(variables, numeric_limits<double>::max());
    vector<double> maxVals(variables, numeric_limits<double>::lowest());
    
    for(const auto& point : dataset) {
        for(int d = 0; d < variables; d++) {
            minVals[d] = min(minVals[d], point[d]);
            maxVals[d] = max(maxVals[d], point[d]);
        }
    }
    
    for(int d = 0; d < variables; d++) {
        cout << "Dimensión " << d << ": [" << minVals[d] << ", " << maxVals[d] << "]" << endl;
    }
    
    // Simular centros del algoritmo evolutivo (algunos fuera de rango)
    Solution solucion(problema);
    solucion.beforeClusterCenters = {
        {6.98874, 3.92903, 7.52746, 3.04116},     // Cluster 0
        {6.29179, 2.38743, -0.376687, 1.70816},   // Cluster 1 - tiene valor negativo!
        {6.85948, 3.40673, 6.96177, 1.85998}      // Cluster 2
    };
    
    cout << "\n=== CENTROS ANTES (del evolutivo) ===" << endl;
    for(int i = 0; i < solucion.beforeClusterCenters.size(); i++) {
        cout << "Cluster " << i << ": ";
        for(int d = 0; d < variables; d++) {
            cout << solucion.beforeClusterCenters[i][d] << " ";
        }
        cout << endl;
    }
    
    // Ejecutar greedy
    solucion.calculateDistances();
    solucion.sortDistances();
    solucion.greedy();
    solucion.updateEvaluation();
    
    // Calcular centros finales
    Util util(problema, solucion);
    auto afterCenters = util.calculateRealClusterCoordinates(problema.getNumClusters());
    
    cout << "\n=== CENTROS DESPUES (calculados del greedy) ===" << endl;
    for(int i = 0; i < afterCenters.size(); i++) {
        cout << "Cluster " << i << ": ";
        for(int d = 0; d < variables; d++) {
            cout << afterCenters[i][d] << " ";
        }
        cout << endl;
    }
    
    // Añadir al final del test
    cout << "\n=== TEST BOUND HANDLING ===" << endl;
    vector<double> test_individual = {
        -5.0, 10.0, -2.0, 5.0,     // Cluster 0 - fuera de rango
        8.5, 1.0, 15.0, -1.0,      // Cluster 1 - fuera de rango  
        3.0, 6.0, 2.0, 4.0         // Cluster 2 - algunos fuera
    };

    cout << "Antes del bound handling: ";
    for(double val : test_individual) cout << val << " ";
    cout << endl;

    cout << "\nFitness: " << solucion.getFitness() << endl;
    
    return 0;
}

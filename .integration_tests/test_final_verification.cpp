#include "problem.h"
#include "solutionGreedy.h"
#include "util.h"
#include <iostream>

int main(){
    Problem problema("input_data/iris/04iris.txt", "input_data/iris/04clusters.txt");
    problema.loadData();
    
    // Crear solución con los centros del archivo s2_p12_F
    Solution solucion(problema);
    solucion.beforeClusterCenters = {
        {5.51137, 2.34781, 3.27358, 1.17596},  // Cluster 0 del archivo
        {7.15952, 4.15094, 6.24153, 1.9425},   // Cluster 1 del archivo
        {6.08918, 3.23016, 1.70894, 2.34493}   // Cluster 2 del archivo
    };
    
    cout << "=== CENTROS DEL ARCHIVO s2_p12_F ===" << endl;
    for(int i = 0; i < solucion.beforeClusterCenters.size(); i++) {
        cout << "Cluster " << i << ": ";
        for(int d = 0; d < solucion.beforeClusterCenters[i].size(); d++) {
            cout << solucion.beforeClusterCenters[i][d] << " ";
        }
        cout << endl;
    }
    
    // Ejecutar greedy con estos centros
    solucion.calculateDistances();
    solucion.sortDistances();
    solucion.greedy();
    solucion.updateEvaluation();
    
    // Calcular centros finales
    Util util(problema, solucion);
    auto afterCenters = util.calculateRealClusterCoordinates(problema.getNumClusters());
    
    cout << "\n=== CENTROS DESPUÉS DEL GREEDY ===" << endl;
    for(int i = 0; i < afterCenters.size(); i++) {
        cout << "Cluster " << i << ": ";
        for(int d = 0; d < afterCenters[i].size(); d++) {
            cout << afterCenters[i][d] << " ";
        }
        cout << endl;
    }
    
    cout << "\nFitness: " << solucion.getFitness() << endl;
    
    // Verificar si son iguales (centros del archivo = centros iniciales?)
    cout << "\n=== VERIFICACIÓN ===" << endl;
    bool are_same = true;
    const double tolerance = 1e-4;
    
    for(int i = 0; i < afterCenters.size(); i++) {
        for(int d = 0; d < afterCenters[i].size(); d++) {
            double diff = abs(afterCenters[i][d] - solucion.beforeClusterCenters[i][d]);
            if(diff > tolerance) {
                are_same = false;
                cout << "Diferencia en Cluster " << i << ", Dim " << d 
                     << ": " << diff << endl;
            }
        }
    }
    
    if(are_same) {
        cout << "❌ Los centros del archivo son INICIALES (iguales antes y después)" << endl;
        cout << "❌ Aún hay que arreglar el guardado de centros finales" << endl;
    } else {
        cout << "✅ Los centros del archivo son FINALES (diferentes antes y después)" << endl;
        cout << "✅ El algoritmo está guardando correctamente" << endl;
    }
    
    return 0;
}

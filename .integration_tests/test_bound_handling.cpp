#include "problem.h"
#include "solutionGreedy.h"
#include "bridge.h"
#include "util.h"
#include <iostream>
#include <vector>

// Simular la función de bound handling
void testBoundHandling(vector<double>& individual, 
                      const vector<double>& min_bounds, 
                      const vector<double>& max_bounds,
                      int variables_per_cluster) {
    
    cout << "\n=== APLICANDO BOUND HANDLING ===" << endl;
    for (int j = 0; j < individual.size(); j++) {
        int dimension = j % variables_per_cluster;
        double original = individual[j];
        
        double min_bound = min_bounds[dimension];
        double max_bound = max_bounds[dimension];
        
        if (individual[j] < min_bound) {
            individual[j] = min_bound;
            cout << "Pos " << j << " (dim " << dimension << "): " << original 
                 << " -> " << individual[j] << " (era menor que " << min_bound << ")" << endl;
        }
        else if (individual[j] > max_bound) {
            individual[j] = max_bound;
            cout << "Pos " << j << " (dim " << dimension << "): " << original 
                 << " -> " << individual[j] << " (era mayor que " << max_bound << ")" << endl;
        }
    }
}

int main(){
    Problem problema("input_data/iris/04iris.txt", "input_data/iris/04clusters.txt");
    problema.loadData();
    
    // 1. Obtener rangos reales del dataset
    cout << "=== RANGOS DEL DATASET IRIS ===" << endl;
    const auto& dataset = problema.getDataset();
    int variables = problema.getVariables();
    int numClusters = problema.getNumClusters();
    
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
    
    // 2. Crear individuo con valores fuera de rango (como los del archivo s2_p12_F)
    vector<double> individual = {
        6.98874, 3.92903, 7.52746, 3.04116,     // Cluster 0
        6.29179, 2.38743, -0.376687, 1.70816,   // Cluster 1 - tiene -0.376687 negativo!
        6.85948, 3.40673, 6.96177, 1.85998      // Cluster 2
    };
    
    cout << "\n=== INDIVIDUO ANTES DEL BOUND HANDLING ===" << endl;
    for(int c = 0; c < numClusters; c++) {
        cout << "Cluster " << c << ": ";
        for(int d = 0; d < variables; d++) {
            int index = c * variables + d;
            cout << individual[index] << " ";
            
            // Verificar si está fuera de rango
            if(individual[index] < minVals[d] || individual[index] > maxVals[d]) {
                cout << "[FUERA_RANGO] ";
            }
        }
        cout << endl;
    }
    
    // 3. Aplicar bound handling
    testBoundHandling(individual, minVals, maxVals, variables);
    
    // 4. Mostrar resultado después del bound handling
    cout << "\n=== INDIVIDUO DESPUÉS DEL BOUND HANDLING ===" << endl;
    bool all_in_range = true;
    for(int c = 0; c < numClusters; c++) {
        cout << "Cluster " << c << ": ";
        for(int d = 0; d < variables; d++) {
            int index = c * variables + d;
            cout << individual[index] << " ";
            
            // Verificar si sigue fuera de rango
            if(individual[index] < minVals[d] || individual[index] > maxVals[d]) {
                cout << "[SIGUE_FUERA] ";
                all_in_range = false;
            } else {
                cout << "[OK] ";
            }
        }
        cout << endl;
    }
    
    // 5. Probar con este individuo corregido
    cout << "\n=== PROBANDO CON CENTROS CORREGIDOS ===" << endl;
    
    // Crear bridge y convertir a solución
    ClusteringBridge bridge("input_data/iris/04iris.txt", "input_data/iris/04clusters.txt");
    
    // Convertir vector a Individual (simulando)
    double* individual_array = (double*)malloc(sizeof(double) * individual.size());
    for(int i = 0; i < individual.size(); i++) {
        individual_array[i] = individual[i];
    }
    
    double fitness = bridge.evaluateIndividual(individual_array);
    cout << "Fitness con centros corregidos: " << fitness << endl;
    
    free(individual_array);
    
    cout << "\n=== RESUMEN ===" << endl;
    cout << (all_in_range ? "✓ Todos los centros están en rango" : "❌ Hay centros fuera de rango") << endl;
    
    return 0;
}

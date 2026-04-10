#include <iostream> // entrada/salida estandar
#include <fstream>  // operaciones c/archivos
#include <string>   // manejo cadenas texto
#include <vector>
#include <sstream>
#include <limits>
#include <algorithm>
#include <random>
#include <tuple> // manejo de tuplas
#include <cmath> // funcion sqrt
#include <utility>
using namespace std;

void imprimirVector1D(const vector<int>& vector){
    cout << "[";
    for(size_t i = 0; i < vector.size(); i++) {
        cout << vector[i];
        if(i < vector.size() - 1) {
            cout << ", ";
        }
    }
    cout << "]" << endl;
}

void imprimirVector1Ddouble(const vector<double>& vector){
    cout << "[";
    for(size_t i = 0; i < vector.size(); i++) {
        cout << vector[i];
        if(i < vector.size() - 1) {
            cout << ", ";
        }
    }
    cout << "]" << endl;
}

void imprimirVector1DPair(const vector<pair<int,int>>& vector){
    cout << "[" << endl;
    for(size_t i = 0; i < vector.size(); i++){
        cout << i + 1 << ": (" 
            << vector[i].first << ", " 
            << vector[i].second << ")\n";
    }
    cout << "]" << endl;
}

void imprimirVector2D(const vector<vector<int>>& vector){
    cout << "[" << endl;
    for(size_t i = 0; i < vector.size(); i++){
        cout << "  [";
        for(size_t j = 0; j < vector[i].size(); j++){
            cout << vector[i][j];
            if(j < vector[i].size() - 1){
                cout << ", ";
            }
        }
        cout << "]";
        if(i < vector.size() - 1){
            cout << ",";
        }
        cout << endl;
    }
    cout << "]" << endl;
}

void imprimirVector2Ddouble(const vector<vector<double>>& vector){
    cout << "[" << endl;
    for(size_t i = 0; i < vector.size(); i++){
        cout << "  [";
        for(size_t j = 0; j < vector[i].size(); j++){
            cout << vector[i][j];
            if(j < vector[i].size() - 1){
                cout << ", ";
            }
        }
        cout << "]";
        if(i < vector.size() - 1){
            cout << ",";
        }
        cout << endl;
    }
    cout << "]" << endl;
}

vector<vector<pair<int, int>>> obtenerCoordenadasPorCluster(
	const vector<int>& asignacion,
	const vector<vector<int>>& datos,
	int numClusters
){
	vector<vector<pair<int, int>>> coordenadasPorCluster(numClusters);
	for(int i = 0; i < asignacion.size(); i++){
		int cluster = asignacion[i];
		if(cluster >= 0 && cluster , numClusters){
			int x = datos[i][0];
			int y = datos[i][1];
			coordenadasPorCluster[cluster].push_back(make_pair(x, y));
		}
	}
	return coordenadasPorCluster;
}

vector<double> obtenerValoresClusters(const vector<vector<pair<int, int>>>& coordenadasPorCluster, int numClusters){
	vector<double> valoresClusters(numClusters, 0.0);
	for(int c = 0; c < numClusters; c++){
		double sumaDistancias = 0.0;
		for(int i = 0; i < coordenadasPorCluster[c].size() - 1; i++){
			for(int j = i + 1; j < coordenadasPorCluster[c].size(); j++){
				int x1 = coordenadasPorCluster[c][i].first;
				int y1 = coordenadasPorCluster[c][i].second;
				int x2 = coordenadasPorCluster[c][j].first;
				int y2 = coordenadasPorCluster[c][j].second;
				int dx = x2 - x1;
				int dy = y2 - y1;
				double distancia = sqrt(dx*dx + dy*dy);
				sumaDistancias += distancia;
			}
		}
		valoresClusters[c] = sumaDistancias / coordenadasPorCluster[c].size();
	}
	return valoresClusters;
}

int main(int argc, char* argv[]){
// class problem begin
    string nombreDataset = argv[1];
    string nombreClusters = argv[2];
    ifstream archivoData(nombreDataset);
    ifstream archivoClusters(nombreClusters);
    
    // vector de clusters c/limites
    // numero de clusters
    int numClusters;
    archivoClusters >> numClusters;
    vector<int> limClusters;
    for(int i = 0; i < numClusters; i++){
        int lim;
        archivoClusters >> lim;
        limClusters.push_back(lim);
    }
    //imprimirVector1D(limClusters);
    archivoClusters.close();
    
    // vector del dataset inicial
    // obtener los valores de las columnas y filas
    int filas;
    int columnas;
    archivoData >> filas;
    archivoData >> columnas;
    vector<vector<int>> datos(filas, vector<int>(columnas,0));
    for(int i = 0; i < filas; i++){
        for(int j = 0; j < columnas; j++){
            archivoData >> datos[i][j];
        }
    }
    //imprimirVector2D(datos);
    archivoData.close();

    vector<vector<int>> datos(filas, vector<int>(columnas,0));
    for(int i = 0; i < filas; i++){
        for(int j = 0; j < columnas; j++){
            archivoData >> datos[i][j];
        }
    }
    //imprimirVector2D(datos);
    archivoData.close();
    
// class problem end
// class solution begin

    // generar dos puntos de manera aleatoria (coordenadas x, y) para cada cluster (4)
    // obtener los valores minimo y maximo para los clusters (normalizacion)
    int minimo = numeric_limits<int>::max();
    int maximo = numeric_limits<int>::min();
    for(const auto& fila : datos){
        if(!fila.empty()){
            auto [min_it, max_it] = minmax_element(fila.begin(), fila.end());
            minimo = min(minimo, *min_it);
            maximo = max(maximo, *max_it);
        }
    }
    random_device rd;
    mt19937 gen(rd()); // Motor Mersenne Twister - estandar
    uniform_int_distribution<int> rango(minimo, maximo); // rango
    vector<pair<int, int>> centrosClusters;
    for(int i = 0; i < numClusters; i++){
        int x = rango(gen);
        int y = rango(gen);
        pair<int, int> centro(x, y);
        centrosClusters.push_back(centro);
    }
    imprimirVector1DPair(centrosClusters);
    
    // vector<pair<int, int>> centrosClusters = {
    //     {20, 140},
    //     {100, 140},
    //     {20, 20},
    //     {100, 20}
    // };

    // matriz de distancias entre punto y centroides clusters
    vector<vector<double>> distancias(filas, vector<double>(numClusters,0));
    for(int i = 0; i < filas; i++){
        for(int j = 0; j < numClusters; j++){
            int dx = datos[i][0] - centrosClusters[j].first;
            int dy = datos[i][1] - centrosClusters[j].second;
            distancias[i][j] = sqrt(dx*dx + dy*dy);
        }
    }
    //imprimirVector2Ddouble(distancias);
	

    // asignacion
    vector<int> asignacion(filas, 0);

    // metodo 1 : greedy begin
    // ordenamiento distancias
    vector<tuple<int, int, double>> todas_distancias;
    for(int i = 0; i < filas; i++){
        for(int j = 0; j < numClusters; j++){
            todas_distancias.push_back(make_tuple(i, j, distancias[i][j]));
        }
    }
    sort(todas_distancias.begin(), todas_distancias.end(), [](const tuple<int, int, double>& a, const tuple<int, int, double>& b){
        return get<2>(a) < get<2>(b);
    });
    
    for(const auto& t : todas_distancias){
        int punto_id = get<0>(t);
        int centro_id = get<1>(t);
        if(asignacion[punto_id] != 0) continue;
        if(limClusters[centro_id] > 0){
            asignacion[punto_id] = centro_id;
            limClusters[centro_id]--;
        }
    }
    imprimirVector1D(asignacion);

    // funcion fitness begin
	// coordenadas por cluster
	auto coordenadasPorCluster = obtenerCoordenadasPorCluster(asignacion, datos, numClusters);

	cout << "Puntos por cluster:" << endl;
	for(int c = 0; c < coordenadasPorCluster.size(); c++){
		cout << "Cluster [" << c << "]: " << endl;
		int max = coordenadasPorCluster[c].size();
		for(int i = 0; i < max; i++){
			cout << "	(" << coordenadasPorCluster[c][i].first << ", " << coordenadasPorCluster[c][i].second << ")" << endl;
		}
	}    

    // funcion fitness - aplica para todas las soluciones (metodos)
	// fitness : cluster + global
	auto valoresClusters = obtenerValoresClusters(coordenadasPorCluster, numClusters);
	imprimirVector1Ddouble(valoresClusters);

	double fitness_global = accumulate(valoresClusters.begin(), valoresClusters.end(), 0.0);
	cout << "Fitness: " << fitness_global << endl;
    // funcion fitness end
// class solution end
}

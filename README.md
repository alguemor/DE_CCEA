# DE_CCEA

## Representación

- Cada individuo en la poblacion representa un conjunto de centros de clusters
- La funcion de fitness evalúa la calidad de la asignacion de puntos a clusters

## Evaluación

1. Los individuos (centros de clusters) son generados/evolucionados por DE-EDM
2. Para cada individuo, calculamos las distancias de cada punto a cada centro
3. Aplicamos asignacion de puntos a clusters respetando las restricciones mediante:
   - **Greedy**: Asignacion heurística rápida basada en distancias ordenadas
   - **MCFP**: Asignacion óptima usando Min-Cost Max-Flow
4. Evaluamos la calidad de los clusters usando la suma cuadrática de diferencias

## Métodos de Asignación

### Greedy
- Ordena todas las distancias punto-cluster de menor a mayor
- Asigna cada punto al cluster más cercano con capacidad disponible
- Rápido pero no garantiza optimidad global

### MCFP (Min-Cost Max-Flow)
- Construye red de flujo: fuente → puntos → clusters → sumidero
- Resuelve problema de flujo de costo mínimo para asignacion óptima
- Más lento pero garantiza asignacion globalmente óptima

## Ejecucion

```bash
make
./DE_CCEA_exe [seed] [dataset] [di] [metodo]
```

**Parámetros:**
- `seed`: Semilla para reproducibilidad
- `dataset`: Nombre del dataset (01, 04, 05, 07)
- `di`: Parámetro de diversidad
- `metodo`: Método de asignación (`greedy` o `mcfp`) [default: greedy]

**Ejemplos:**
```bash
./DE_CCEA_exe 123 01 0.5 greedy
./DE_CCEA_exe 123 01 0.5 mcfp
```

## Datasets Disponibles

- `01`: ruspini (75 puntos, 4 clusters)
- `04`: iris (150 puntos, 3 clusters)
- `05`: haptics (463 puntos, 5 clusters)  
- `07`: wine (178 puntos, 3 clusters)

## Configuración

- Tamaño población = `sqrt(problem_size) * log(problem_size) * 50`
- Evaluaciones máximas = 1000
- Resultados guardados en `results/[dataset]/`

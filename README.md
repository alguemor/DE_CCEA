# DE_CCEA

## Representación

- Cada inviduo en la poblacion representa un conjunto de centros de clusters
- La funcion de fitness evalúa la caidad de la signacion de puntos a clusters

## Evaluación

1. Los individuos (centros de clusters) son generados/evolucionados por DE-EDM
2. Para cada individuo, calculamos las distancias de cada punto a cada centro
3. Aplicamos una asignacion greedy de puntos a clusters respetando las restricciones
4. Evaluamos la calidad de los clusters usando la suma cuadrática de diferencias

## Ejecucion

make
./DE_CCEA_exe [seed] [dataset] [di]

- tamaño problacion = 10,000

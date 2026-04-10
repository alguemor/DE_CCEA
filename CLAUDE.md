# CLAUDE.md

This file provides guidance to Claude Code when working with code in this repository.

## Build Commands

```bash
make          # Compila DE_CCEA_exe
make clean    # Elimina archivos .o
```

**Importante**: El Makefile usa `-march=x86-64` (no `-march=native`) para compatibilidad con los nodos del cluster CIMAT.

## Execution

```bash
./DE_CCEA_exe [seed] [dataset] [di] [method] [max_evals] [results_dir]
```

| Parámetro | Descripción | Ejemplo |
|-----------|-------------|---------|
| `seed` | Semilla aleatoria | `1` |
| `dataset` | ID del dataset (ver datasets.cfg) | `01` |
| `di` | Parámetro de diversidad | `0.5` |
| `method` | `mcfp` o `greedy` | `mcfp` |
| `max_evals` | Número máximo de evaluaciones | `100000` |
| `results_dir` | Directorio de resultados (default: `results`) | `results20260331` |

**Ejemplo local**:
```bash
./DE_CCEA_exe 1 01 0.5 mcfp 5000 results_test
```

**Ejemplo cluster**:
```bash
./DE_CCEA_exe 1 05 0.5 mcfp 100000 /home/est_externo_alejandro.guevaramr/DE_CCEA/results20260331
```

## Datasets disponibles (01-10)

| ID | Nombre | Puntos | Dims | Clusters |
|----|--------|--------|------|----------|
| 01 | ruspini | 75 | 2 | 4 |
| 02 | breasttissue | 106 | 9 | 6 |
| 03 | ecoli | 336 | 7 | 8 |
| 04 | iris | 150 | 4 | 3 |
| 05 | haptics | 155 | 1092 | 5 |
| 06 | dermatology | 366 | 34 | 6 |
| 07 | wine | 178 | 13 | 3 |
| 08 | parkinsons | 195 | 22 | 2 |
| 09 | sonar | 208 | 60 | 2 |
| 10 | seeds | 210 | 7 | 3 |

Los datasets 11-29 están en `~/instancias/ajustadas/` pendientes de integrar.

## Arquitectura

### Componentes principales

- **main.cc** — Entry point. Parsea args, crea directorios, abre logger, ejecuta DIVERSITY
- **de.h** — Header principal con interfaces y variables globales
- **diversity.cc** — Algoritmo L-SHADE con diversidad (variant de DE)
- **bridge.h/cpp** — ClusteringBridge: conecta individuos DE con soluciones de clustering
- **problem.h/cpp** — Carga dataset, calcula fitness (sum of squared distances)
- **solution.h/cpp** — SolutionMCFP: asignación por Min-Cost Max-Flow
- **dataset_manager.h/cpp** — Gestiona configuración de datasets desde datasets.cfg
- **util.h/cpp** — Utilidades de clustering
- **logger.h/cpp** — Logger class con niveles INFO/DEBUG/WARNING/ERROR y timestamps

### Logger

```cpp
// Uso directo
g_logger.info("mensaje");
g_logger.warning("advertencia");

// Macros con stream operator
LOG_INFO("GEN " << gen << " best=" << best);
LOG_WARNING("STAGNATION: " << n << " evals without improvement");
LOG_DEBUG("MCFP timing build=" << t << "s");
```

Formato de log: `[1774942708.909279] [INFO ] mensaje`

### Flujo de datos

1. `DatasetManager` carga config desde `datasets.cfg`
2. `ClusteringBridge` inicializa `Problem` con datos y archivo de clusters
3. L-SHADE evoluciona individuos (vectores de centros de clusters)
4. Por cada individuo: `ClusteringBridge` → `SolutionMCFP` → MCFP solver
5. MCFP asigna puntos a clusters respetando cardinalidades (optimal assignment)
6. `Problem` calcula fitness = suma de distancias cuadradas intra-cluster

### Representación

- Individuo DE = centros de clusters como array plano: `[c1_x1, c1_x2, ..., ck_xd]`
- `problem_size` = `numClusters × dimensionalidad`
- `pop_size` = 200 (fijo)
- Fitness: minimizar sum of squared distances (ccMSSC problem)

## Estructura de directorios

```
DE_CCEA/
├── input_data/         # Datasets (01/ a 10/)
├── results/            # Resultados por defecto (gitignored)
│   ├── 01/ ... 10/     # result.txt y _F por seed
│   └── logs/           # debug logs por seed
├── memory/             # Historial de sesiones de desarrollo
│   ├── SESSIONS.md     # Indice de sesiones
│   └── sessions/       # Notas por sesion
├── logger.h/cpp        # Logger class
├── bridge.h/cpp        # ClusteringBridge
├── diversity.cc        # L-SHADE algorithm
├── main.cc             # Entry point
└── datasets.cfg        # Configuracion de datasets
```

## Workflow con el cluster CIMAT

### Compilar en el cluster
```bash
# En el login node del cluster:
cd ~/DE_CCEA
make clean && make
# IMPORTANTE: NO usar -march=native (diferentes CPUs en compute nodes)
```

### Ejecutar con SLURM
Los task files y slurm scripts estan en `~/Documentos/task-cluster/` y `~/Documentos/slurms/`.

Cada task file tiene 30 lineas (seeds 1-30):
```
cd /home/est_externo_alejandro.guevaramr/DE_CCEA && ./DE_CCEA_exe 1 05 0.5 mcfp 100000 /home/.../results
```

**Notas importantes**:
- `--ntasks` minimo = 2 (MPI_Scheduler necesita 1 master + 1 worker)
- Dataset haptics (05): usar `--ntasks=2` (secuencial) para evitar OOM kills
- El directorio de `--output` en slurm debe existir antes de sbatch
- Usar paths absolutos en el 6to argumento (no relativos)

## Branches del repositorio

| Branch | Descripcion |
|--------|-------------|
| `main` | Version mas reciente y estable |
| `robust-logging` | Origen del merge actual (puede eliminarse) |
| `haptics-logging` | Logging parcial previo al Logger class |
| `mcfp-class` | Cuando se implemento MCFP como clase |
| `cluster-silent` | Version para cluster sin stdout |
| `master` | Version historica original |
| `archive/ccea-v1` | CCEA original solo Greedy (~May 2025) |
| `archive/de-ccea-snapshot` | DE-CCEA intermedio (~Aug 2025) |

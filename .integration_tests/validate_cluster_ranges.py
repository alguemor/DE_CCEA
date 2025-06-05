#!/usr/bin/env python3
"""
Script para verificar que los centros de clusters estén dentro de los rangos válidos del dataset.

Uso:
    python validate_cluster_ranges.py ~/results/ruspini/
    python validate_cluster_ranges.py ~/results/iris/
"""

import os
import sys
import glob
import numpy as np
from pathlib import Path

def load_dataset_ranges(dataset_path):
    """
    Carga un dataset y calcula los rangos mínimos y máximos por dimensión.
    
    Args:
        dataset_path (str): Ruta al archivo del dataset
        
    Returns:
        tuple: (min_ranges, max_ranges, num_points, num_dimensions)
    """
    try:
        with open(dataset_path, 'r') as f:
            # Primera línea: número de puntos y dimensiones
            first_line = f.readline().strip().split()
            num_points = int(first_line[0])
            num_dimensions = int(first_line[1])
            
            # Leer todos los puntos
            data = []
            for line in f:
                line = line.strip()
                if line:  # Ignorar líneas vacías
                    values = [float(x) for x in line.split()]
                    if len(values) == num_dimensions:
                        data.append(values)
            
            # Convertir a numpy array para facilitar cálculos
            data = np.array(data)
            
            # Calcular rangos por dimensión
            min_ranges = np.min(data, axis=0)
            max_ranges = np.max(data, axis=0)
            
            return min_ranges, max_ranges, num_points, num_dimensions
            
    except Exception as e:
        print(f"Error leyendo dataset {dataset_path}: {e}")
        return None, None, None, None

def get_dataset_path(results_dir):
    """
    Determina la ruta del dataset basándose en el directorio de resultados.
    
    Args:
        results_dir (str): Directorio de resultados (ej: ~/results/ruspini/)
        
    Returns:
        str: Ruta al archivo del dataset
    """
    # Obtener el nombre del dataset del directorio padre
    dataset_name = Path(results_dir).name.lower()
    
    # Mapeo de nombres de dataset a archivos
    dataset_mapping = {
        'ruspini': '01ruspini.txt',
        'iris': '04iris.txt',
        # Agregar más datasets aquí según sea necesario
    }
    
    if dataset_name in dataset_mapping:
        # Construir ruta al dataset
        home_dir = Path.home()
        dataset_path = home_dir / 'input_data' / dataset_mapping[dataset_name]
        return str(dataset_path)
    else:
        print(f"Dataset desconocido: {dataset_name}")
        print(f"Datasets soportados: {list(dataset_mapping.keys())}")
        return None

def parse_result_file(file_path, num_clusters, num_dimensions):
    """
    Parsea un archivo de resultados y extrae los centros de clusters.
    
    Args:
        file_path (str): Ruta al archivo de resultados
        num_clusters (int): Número de clusters esperados
        num_dimensions (int): Número de dimensiones por cluster
        
    Returns:
        tuple: (fitness, centers) donde centers es una lista de listas
    """
    try:
        with open(file_path, 'r') as f:
            content = f.readline().strip()
            values = [float(x) for x in content.split()]
            
            if len(values) < 1:
                return None, None
            
            fitness = values[0]
            center_values = values[1:]
            
            expected_values = num_clusters * num_dimensions
            if len(center_values) != expected_values:
                print(f"Warning: {file_path} tiene {len(center_values)} valores, esperados {expected_values}")
                return fitness, None
            
            # Reorganizar en clusters
            centers = []
            for c in range(num_clusters):
                cluster_center = []
                for d in range(num_dimensions):
                    index = c * num_dimensions + d
                    cluster_center.append(center_values[index])
                centers.append(cluster_center)
            
            return fitness, centers
            
    except Exception as e:
        print(f"Error leyendo archivo {file_path}: {e}")
        return None, None

def validate_centers(centers, min_ranges, max_ranges, tolerance=1e-6):
    """
    Valida que los centros estén dentro de los rangos permitidos.
    
    Args:
        centers (list): Lista de centros de clusters
        min_ranges (np.array): Rangos mínimos por dimensión
        max_ranges (np.array): Rangos máximos por dimensión
        tolerance (float): Tolerancia para errores de punto flotante
        
    Returns:
        tuple: (is_valid, violations)
    """
    violations = []
    
    for cluster_idx, center in enumerate(centers):
        for dim_idx, value in enumerate(center):
            min_val = min_ranges[dim_idx]
            max_val = max_ranges[dim_idx]
            
            if value < (min_val - tolerance) or value > (max_val + tolerance):
                violations.append({
                    'cluster': cluster_idx,
                    'dimension': dim_idx,
                    'value': value,
                    'min_allowed': min_val,
                    'max_allowed': max_val,
                    'range': f"[{min_val:.3f}, {max_val:.3f}]"
                })
    
    return len(violations) == 0, violations

def main():
    if len(sys.argv) != 2:
        print("Uso: python validate_cluster_ranges.py <directorio_resultados>")
        print("Ejemplo: python validate_cluster_ranges.py ~/results/ruspini/")
        sys.exit(1)
    
    results_dir = sys.argv[1]
    
    # Verificar que el directorio existe
    if not os.path.isdir(results_dir):
        print(f"Error: El directorio {results_dir} no existe")
        sys.exit(1)
    
    # Obtener ruta del dataset
    dataset_path = get_dataset_path(results_dir)
    if not dataset_path:
        sys.exit(1)
    
    # Verificar que el dataset existe
    if not os.path.isfile(dataset_path):
        print(f"Error: El archivo del dataset {dataset_path} no existe")
        sys.exit(1)
    
    # Cargar rangos del dataset
    print(f"Cargando dataset: {dataset_path}")
    min_ranges, max_ranges, num_points, num_dimensions = load_dataset_ranges(dataset_path)
    
    if min_ranges is None:
        sys.exit(1)
    
    print(f"Dataset: {num_points} puntos, {num_dimensions} dimensiones")
    print("Rangos por dimensión:")
    for i in range(num_dimensions):
        print(f"  Dim {i}: [{min_ranges[i]:.3f}, {max_ranges[i]:.3f}]")
    
    # Determinar número de clusters (asumiendo que es consistente en todos los archivos)
    # Para ruspini típicamente 4 clusters, para iris 3 clusters
    dataset_name = Path(results_dir).name.lower()
    num_clusters_map = {
        'ruspini': 4,
        'iris': 3,
    }
    num_clusters = num_clusters_map.get(dataset_name, 3)  # Default 3
    
    # Buscar archivos de resultados
    pattern = os.path.join(results_dir, "s*_p*_F")
    result_files = glob.glob(pattern)
    
    if not result_files:
        print(f"No se encontraron archivos de resultados en {results_dir}")
        print(f"Patrón buscado: {pattern}")
        sys.exit(1)
    
    result_files.sort()
    print(f"\nEncontrados {len(result_files)} archivos de resultados")
    
    # Validar cada archivo
    total_files = len(result_files)
    valid_files = 0
    total_violations = 0
    
    print(f"\nValidando archivos (esperando {num_clusters} clusters)...")
    print("=" * 80)
    
    for file_path in result_files:
        file_name = os.path.basename(file_path)
        fitness, centers = parse_result_file(file_path, num_clusters, num_dimensions)
        
        if centers is None:
            print(f"❌ {file_name}: Error al parsear archivo")
            continue
        
        is_valid, violations = validate_centers(centers, min_ranges, max_ranges)
        
        if is_valid:
            print(f"✅ {file_name}: Fitness={fitness:.4f} - Todos los centros en rango")
            valid_files += 1
        else:
            print(f"❌ {file_name}: Fitness={fitness:.4f} - {len(violations)} violaciones")
            total_violations += len(violations)
            
            # Mostrar violaciones
            for v in violations:
                print(f"   Cluster {v['cluster']}, Dim {v['dimension']}: "
                      f"{v['value']:.6f} fuera de {v['range']}")
    
    # Resumen final
    print("=" * 80)
    print(f"RESUMEN:")
    print(f"  Archivos procesados: {total_files}")
    print(f"  Archivos válidos: {valid_files}")
    print(f"  Archivos con errores: {total_files - valid_files}")
    print(f"  Total de violaciones: {total_violations}")
    
    if valid_files == total_files:
        print(f"🎉 ¡Todos los archivos tienen centros dentro de los rangos válidos!")
        sys.exit(0)
    else:
        print(f"⚠️  {total_files - valid_files} archivos tienen centros fuera de rango")
        sys.exit(1)

if __name__ == "__main__":
    main()

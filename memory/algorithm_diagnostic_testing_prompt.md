# Algorithm Diagnostic Testing Prompt

## Problem Identification

Based on the fitness analysis results, there are significant performance gaps between your algorithms (greedy and mcfp) and the state-of-the-art optimal values, particularly for the **HapticsSmall dataset (directory 05)**:

- **Optimal value**: 1.794e+04
- **Your greedy result**: 9.304e+06
- **Your mcfp result**: 9.287e+06
- **Gap from optimal**: ~51,700% (both methods)

This massive discrepancy suggests fundamental algorithmic implementation issues specific to the HapticsSmall dataset, as the other 9 datasets (directories 01-04, 06-10) show similar, equal, or only small percentage differences from the best known solutions.

## Critical Issues to Investigate

### 1. **Data Preprocessing Problems**
- **Test**: Verify data loading and normalization
- **Questions**:
  - Are features properly scaled/normalized for high-dimensional data (1092 dimensions)?
  - Is the distance metric correctly implemented for high-dimensional spaces?
  - Are there missing values or data corruption issues?

### 2. **Clustering Algorithm Implementation**
- **Test**: Validate core clustering logic
- **Questions**:
  - Is the centroid calculation correct for high-dimensional data?
  - Are you using appropriate distance metrics (Euclidean vs Manhattan vs Cosine)?
  - Is the cluster assignment step working correctly?
  - Are convergence criteria too loose or incorrect?

### 3. **Objective Function Calculation**
- **Test**: Manual verification of fitness calculation
- **Questions**:
  - Is the within-cluster sum of squares (WCSS) calculated correctly?
  - Are you minimizing the right objective function?
  - Is there a scaling issue in the fitness calculation?

### 4. **High-Dimensional Data Handling**
- **Test**: Compare performance on low vs high-dimensional datasets
- **Analysis**: HapticsSmall has 1092 dimensions - curse of dimensionality issues
- **Questions**:
  - Are you handling high-dimensional spaces appropriately?
  - Should you implement dimensionality reduction (PCA, t-SNE)?
  - Is the distance calculation stable in high dimensions?

## Diagnostic Testing Strategy

### Phase 1: Basic Validation ✅ COMPLETED
**Status**: Already tested. Results show that 9 out of 10 datasets perform well (similar, equal, or small percentage differences from optimal). **Only HapticsSmall is the outlier**, indicating the issue is dataset-specific rather than a fundamental algorithm problem.

### Phase 2: Algorithm Debugging (PRIORITY)
```
1. Add extensive logging to track:
   - Initial centroids
   - Fitness evolution per iteration
   - Cluster assignments at each step
   - Convergence behavior
   - Store logs in separate files under results/ directory

2. Compare your fitness calculation with manual calculation for small subset
~~3. Test different initialization strategies (K-means++, random, furthest-first)~~ [SKIP - for future]
4. Verify termination conditions aren't stopping too early
```

### Phase 3: HapticsSmall-Specific Investigation
```
1. Analyze data distribution:
   - Check for outliers or extreme values
   - Verify data ranges and scaling
   - Look for correlated features

~~2. Test different approaches:~~ [SKIP]
   ~~- Apply PCA to reduce dimensions to 50-100~~
   ~~- Try different distance metrics~~
   ~~- Experiment with different cluster initialization~~
   ~~- Test with normalized vs unnormalized data~~
```

### Phase 4: Parameter Tuning
```
1. Grid search on critical parameters:
   - Maximum iterations
   - Convergence tolerance
   - Population size (for evolutionary approaches)
   - Mutation/crossover rates

~~2. Compare against reference implementations:~~ [SKIP]
   ~~- Scikit-learn KMeans~~
   ~~- Standard k-means++ initialization~~
   ~~- Professional clustering libraries~~
```

## Implementation Tests to Run

### Test 1: Logging on Smaller Dataset
```bash
# Create new branch from cluster silent branch
git checkout cluster-silent
git checkout -b diagnostic-logging-test

# Run algorithm with extensive logging on a smaller dataset (e.g., Iris - directory 04)
# Store logs in: results/logs/iris_greedy_debug.log
#               results/logs/iris_mcfp_debug.log
```

**Logging Requirements**:
- **Separate log files** stored in `results/logs/` directory
- **Log format**: `{dataset}_{method}_debug.log`
- **Content to log**:
  - Initial centroids coordinates
  - Fitness value at each iteration
  - Number of points per cluster at each iteration
  - Convergence criteria values
  - Final cluster assignments
  - Total iterations performed

### Test 2: Logging on HapticsSmall Dataset
```bash
# After Test 1 validation, run on HapticsSmall
# Store logs in: results/logs/haptics_greedy_debug.log
#               results/logs/haptics_mcfp_debug.log
```

**Comparison Analysis**:
- Compare logging patterns between Iris (working) and HapticsSmall (problematic)
- Identify where HapticsSmall behavior diverges
- Look for:
  - Abnormal fitness progression
  - Clustering instability
  - Convergence issues
  - Centroid drift patterns

## Cluster Execution Setup

### Repository Setup
```bash
# Create new branch for diagnostic testing
git checkout cluster-silent
git checkout -b diagnostic-haptics-investigation

# Ensure logs directory exists
mkdir -p results/logs/
```

### Logging Implementation
```python
# Example logging structure
import logging

def setup_logging(dataset_name, method_name):
    log_filename = f"results/logs/{dataset_name}_{method_name}_debug.log"
    logging.basicConfig(
        filename=log_filename,
        level=logging.DEBUG,
        format='%(asctime)s - %(levelname)s - %(message)s',
        filemode='w'
    )
    return logging.getLogger()

# In your algorithm:
logger = setup_logging("haptics", "greedy")
logger.info(f"Initial centroids: {centroids}")
logger.info(f"Iteration {i}: Fitness = {current_fitness}")
logger.info(f"Cluster sizes: {cluster_sizes}")
```

## Specific Focus for HapticsSmall

### Data Analysis Priority
1. **Dimension impact**: 1092 dimensions vs others (max 1798 for Hierarchical, but that one works)
2. **Value ranges**: Check if HapticsSmall has unusual data ranges
3. **Data distribution**: Look for extreme outliers or unusual patterns
4. **Correlation structure**: High correlation might cause numerical issues

### Expected Outcomes
- **Test 1 (Iris)**: Should confirm normal algorithm behavior with detailed logging
- **Test 2 (HapticsSmall)**: Should reveal specific points where algorithm behavior diverges

## Success Criteria

After implementing diagnostic logging:
1. **Clear identification** of where HapticsSmall processing differs from working datasets
2. **Actionable insights** into what causes the 51,700% gap
3. **Specific algorithmic modifications** needed for high-dimensional clustering
4. **HapticsSmall target**: Reduce gap to < 500% from optimal (closer to 1.794e+04)

## Next Steps

1. **Setup**: Create diagnostic-haptics-investigation branch from cluster-silent
2. **Test 1**: Implement and run logging on Iris dataset
3. **Validation**: Ensure logging captures all necessary information
4. **Test 2**: Run logging on HapticsSmall dataset
5. **Analysis**: Compare logs to identify root cause
6. **Fix**: Implement targeted solution based on findings
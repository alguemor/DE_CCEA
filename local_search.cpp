#include "local_search.h"
#include "bridge.h"
#include "logger.h"
#include <cmath>
#include <limits>
using namespace std;

LocalSearch::LocalSearch(ClusteringBridge* bridge, int& nfes_ref, int max_evals,
                         int num_clusters, int num_points, int dimension)
    : bridge_(bridge), nfes_(nfes_ref), max_evals_(max_evals),
      K_(num_clusters), N_(num_points), D_(dimension)
{
    problem_ = bridge_->getProblem();
    dataset_ = &problem_->getDataset();

    stats_.resize(K_);
    for(int c = 0; c < K_; c++){
        stats_[c].sum.resize(D_, 0.0L);
        stats_[c].sumSq = 0.0L;
        stats_[c].V     = 0.0L;
        stats_[c].size  = 0;
    }
    assignment_.resize(N_, -1);
    clusterPoints_.resize(K_);
}

// ---------------------------------------------------------------------------
// initStats: construye Sum[c], SumSq[c] y V[c] desde la asignacion
// V(C) = SumSq[C] - ||Sum[C]||^2 / size[C]
// ---------------------------------------------------------------------------
void LocalSearch::initStats(const vector<int>& assignment) {
    assignment_ = assignment;

    for(int c = 0; c < K_; c++){
        fill(stats_[c].sum.begin(), stats_[c].sum.end(), 0.0L);
        stats_[c].sumSq = 0.0L;
        stats_[c].V     = 0.0L;
        stats_[c].size  = 0;
        clusterPoints_[c].clear();
    }

    for(int p = 0; p < N_; p++){
        int c = assignment[p];
        if(c < 0 || c >= K_) continue;
        const auto& x = (*dataset_)[p];
        long double sq = 0.0L;
        for(int d = 0; d < D_; d++){
            stats_[c].sum[d] += x[d];
            sq += x[d] * x[d];
        }
        stats_[c].sumSq += sq;
        stats_[c].size++;
        clusterPoints_[c].push_back(p);
    }

    for(int c = 0; c < K_; c++){
        if(stats_[c].size == 0) continue;
        long double dot = 0.0L;
        for(int d = 0; d < D_; d++)
            dot += stats_[c].sum[d] * stats_[c].sum[d];
        stats_[c].V = stats_[c].sumSq - dot / (long double)stats_[c].size;
    }
}

// ---------------------------------------------------------------------------
// computeSwapDelta: delta de fitness si se intercambia p1(clA) <-> p2(clB)
// Complejidad: O(D)
// ---------------------------------------------------------------------------
long double LocalSearch::computeSwapDelta(int p1, int clA, int p2, int clB) const {
    const auto& xp1 = (*dataset_)[p1];
    const auto& xp2 = (*dataset_)[p2];

    long double sp1 = 0.0L, sp2 = 0.0L, cross = 0.0L;
    long double dA1 = 0.0L, dA2 = 0.0L; // dot(newSumA, newSumA) parciales
    long double dB1 = 0.0L, dB2 = 0.0L;

    for(int d = 0; d < D_; d++){
        long double vp1 = xp1[d], vp2 = xp2[d];
        sp1 += vp1 * vp1;
        sp2 += vp2 * vp2;

        // newSumA[d] = sumA[d] - vp1 + vp2
        long double nA = stats_[clA].sum[d] - vp1 + vp2;
        dA1 += nA * nA;

        // newSumB[d] = sumB[d] - vp2 + vp1
        long double nB = stats_[clB].sum[d] - vp2 + vp1;
        dB1 += nB * nB;
    }

    long double sA = (long double)stats_[clA].size;
    long double sB = (long double)stats_[clB].size;

    long double newSumSqA = stats_[clA].sumSq - sp1 + sp2;
    long double newSumSqB = stats_[clB].sumSq - sp2 + sp1;

    long double newVA = newSumSqA - dA1 / sA;
    long double newVB = newSumSqB - dB1 / sB;

    return (newVA - stats_[clA].V) + (newVB - stats_[clB].V);
}

// ---------------------------------------------------------------------------
// applySwap: actualiza stats_ y assignment_ tras aceptar un swap
// ---------------------------------------------------------------------------
void LocalSearch::applySwap(int p1, int clA, int p2, int clB) {
    const auto& xp1 = (*dataset_)[p1];
    const auto& xp2 = (*dataset_)[p2];

    long double sp1 = 0.0L, sp2 = 0.0L;
    for(int d = 0; d < D_; d++){
        sp1 += xp1[d] * xp1[d];
        sp2 += xp2[d] * xp2[d];
    }

    // Actualizar cluster A
    stats_[clA].sumSq += sp2 - sp1;
    for(int d = 0; d < D_; d++)
        stats_[clA].sum[d] += xp2[d] - xp1[d];
    {
        long double dot = 0.0L;
        for(int d = 0; d < D_; d++) dot += stats_[clA].sum[d] * stats_[clA].sum[d];
        stats_[clA].V = stats_[clA].sumSq - dot / (long double)stats_[clA].size;
    }

    // Actualizar cluster B
    stats_[clB].sumSq += sp1 - sp2;
    for(int d = 0; d < D_; d++)
        stats_[clB].sum[d] += xp1[d] - xp2[d];
    {
        long double dot = 0.0L;
        for(int d = 0; d < D_; d++) dot += stats_[clB].sum[d] * stats_[clB].sum[d];
        stats_[clB].V = stats_[clB].sumSq - dot / (long double)stats_[clB].size;
    }

    // Actualizar asignaciones
    assignment_[p1] = clB;
    assignment_[p2] = clA;

    // Actualizar listas de puntos
    for(auto& pt : clusterPoints_[clA]) if(pt == p1){ pt = p2; break; }
    for(auto& pt : clusterPoints_[clB]) if(pt == p2){ pt = p1; break; }
}

long double LocalSearch::currentFitness() const {
    long double total = 0.0L;
    for(int c = 0; c < K_; c++) total += stats_[c].V;
    return total;
}

void LocalSearch::updateCentroidsInIndividual(Individual individual) const {
    for(int c = 0; c < K_; c++){
        if(stats_[c].size == 0) continue;
        long double inv = 1.0L / (long double)stats_[c].size;
        for(int d = 0; d < D_; d++)
            individual[c * D_ + d] = stats_[c].sum[d] * inv;
    }
}

// ---------------------------------------------------------------------------
// apply: best-improvement local search por intercambio de puntos entre clusters
// ---------------------------------------------------------------------------
Fitness LocalSearch::apply(Individual individual, Fitness current_fitness,
                           const vector<int>& initial_assignment,
                           int generation) {
    initStats(initial_assignment);

    long double fitBefore  = currentFitness();
    long double fitCurrent = fitBefore;
    bool improved_overall  = false;
    int  total_iters       = 0;

    LOG_INFO("LS_START gen=" << generation
             << " nfes=" << nfes_ << " fit=" << fitBefore);

    while(nfes_ < max_evals_) {
        total_iters++;

        long double bestDelta = 0.0L;
        int bestP1 = -1, bestClA = -1, bestP2 = -1, bestClB = -1;
        long long neighborsEval = 0;
        bool budgetExhausted = false;

        for(int clA = 0; clA < K_ && !budgetExhausted; clA++) {
            for(int clB = clA + 1; clB < K_ && !budgetExhausted; clB++) {
                for(int p1 : clusterPoints_[clA]) {
                    for(int p2 : clusterPoints_[clB]) {
                        nfes_++;
                        neighborsEval++;

                        long double delta = computeSwapDelta(p1, clA, p2, clB);
                        if(delta < bestDelta){
                            bestDelta = delta;
                            bestP1 = p1; bestClA = clA;
                            bestP2 = p2; bestClB = clB;
                        }

                        if(nfes_ >= max_evals_){
                            budgetExhausted = true;
                            break;
                        }
                    }
                    if(budgetExhausted) break;
                }
            }
        }

        if(bestP1 == -1) {
            // Optimum local en el espacio de swaps
            LOG_INFO("LS_LOCAL_OPT gen=" << generation
                     << " iter=" << total_iters
                     << " neighbors=" << neighborsEval
                     << " fit=" << fitCurrent
                     << " nfes=" << nfes_);
            break;
        }

        applySwap(bestP1, bestClA, bestP2, bestClB);
        fitCurrent += bestDelta;
        improved_overall = true;

        LOG_INFO("LS_SWAP gen=" << generation
                 << " iter=" << total_iters
                 << " p1=" << bestP1 << " A=" << bestClA
                 << " p2=" << bestP2 << " B=" << bestClB
                 << " delta=" << bestDelta
                 << " fit=" << fitCurrent
                 << " nfes=" << nfes_);

        if(budgetExhausted) break;
    }

    if(improved_overall) {
        LOG_INFO("LS_IMPROVED gen=" << generation
                 << " iters=" << total_iters
                 << " before=" << fitBefore
                 << " after=" << fitCurrent
                 << " delta=" << (fitCurrent - fitBefore)
                 << " nfes=" << nfes_);
        updateCentroidsInIndividual(individual);
        return (Fitness)fitCurrent;
    }

    LOG_DEBUG("LS_NO_IMPROVE gen=" << generation
              << " iters=" << total_iters
              << " fit=" << fitBefore
              << " nfes=" << nfes_);
    return current_fitness;
}

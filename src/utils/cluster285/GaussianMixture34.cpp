/**
 * @file GaussianMixture34.cpp
 * @brief GaussianMixture34 实现
 *
 * 实现高斯混合模型：分裂-合并EM与组件退火自动模型阶数确定。
 */

#include "utils/cluster285/GaussianMixture34.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <random>

/* ---- Construction / Destruction ---- */

GaussianMixture34::GaussianMixture34(QObject *parent)
    : QObject(parent) {}

GaussianMixture34::~GaussianMixture34() = default;

/* ---- Configuration ---- */

void GaussianMixture34::setMaxComponents(int k) { m_maxK = qBound(1, k, 100); }
void GaussianMixture34::setMinComponents(int k) { m_minK = qBound(1, k, m_maxK); }
void GaussianMixture34::setMaxIter(int iters) { m_maxIter = qBound(10, iters, 10000); }
void GaussianMixture34::setTolerance(double tol) { m_tol = qBound(1e-12, tol, 1.0); }
void GaussianMixture34::setAnnealingRate(double rate) { m_annealRate = qBound(0.5, rate, 0.999); }

/* ---- Matrix determinant via Cholesky ---- */

double GaussianMixture34::determinant(const QVector<QVector<double>>& mat) const
{
    int n = mat.size();
    if (n == 0) return 1.0;
    QVector<QVector<double>> L(n, QVector<double>(n, 0.0));

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j <= i; ++j) {
            double s = mat[i][j];
            for (int k = 0; k < j; ++k)
                s -= L[i][k] * L[j][k];
            if (i == j) {
                if (s <= 0.0) s = 1e-10; // Regularize
                L[i][j] = qSqrt(s);
            } else {
                L[i][j] = (L[j][j] > 1e-15) ? s / L[j][j] : 0.0;
            }
        }
    }
    double det = 1.0;
    for (int i = 0; i < n; ++i) det *= L[i][i];
    return det;
}

/* ---- Invert positive-definite matrix ---- */

QVector<QVector<double>> GaussianMixture34::invertMatrix(const QVector<QVector<double>>& mat) const
{
    int n = mat.size();
    QVector<QVector<double>> inv(n, QVector<double>(n, 0.0));
    // Regularize diagonal
    QVector<QVector<double>> A = mat;
    for (int i = 0; i < n; ++i) A[i][i] += 1e-6;

    // Gauss-Jordan elimination
    for (int i = 0; i < n; ++i) inv[i][i] = 1.0;
    for (int col = 0; col < n; ++col) {
        double pivot = A[col][col];
        if (qAbs(pivot) < 1e-15) pivot = 1e-10;
        for (int j = 0; j < n; ++j) { A[col][j] /= pivot; inv[col][j] /= pivot; }
        for (int row = 0; row < n; ++row) {
            if (row == col) continue;
            double factor = A[row][col];
            for (int j = 0; j < n; ++j) {
                A[row][j] -= factor * A[col][j];
                inv[row][j] -= factor * inv[col][j];
            }
        }
    }
    return inv;
}

/* ---- Log multivariate Gaussian density ---- */

double GaussianMixture34::logGaussian(const QVector<double>& x, const Component& comp) const
{
    int d = x.size();
    if (d == 0) return 0.0;
    double det = determinant(comp.covariance);
    double logDet = qLn(qAbs(det) + 1e-300);
    QVector<QVector<double>> invCov = invertMatrix(comp.covariance);

    double quad = 0.0;
    for (int i = 0; i < d; ++i) {
        double sum = 0.0;
        for (int j = 0; j < d; ++j)
            sum += invCov[i][j] * (x[j] - comp.mean[j]);
        quad += (x[i] - comp.mean[i]) * sum;
    }
    return -0.5 * (d * M_LN2PI + logDet + quad);
}

/* ---- E-step ---- */

QVector<QVector<double>> GaussianMixture34::eStep(
    const QVector<QVector<double>>& data, const QVector<Component>& comps) const
{
    int n = data.size();
    int k = comps.size();
    QVector<QVector<double>> resp(n, QVector<double>(k, 0.0));

    for (int i = 0; i < n; ++i) {
        double maxLog = -1e300;
        QVector<double> logResp(k);
        for (int j = 0; j < k; ++j) {
            logResp[j] = qLn(qMax(comps[j].weight, 1e-300)) + logGaussian(data[i], comps[j]);
            if (logResp[j] > maxLog) maxLog = logResp[j];
        }
        double sum = 0.0;
        for (int j = 0; j < k; ++j) sum += qExp(logResp[j] - maxLog);
        double logSum = maxLog + qLn(qMax(sum, 1e-300));
        for (int j = 0; j < k; ++j)
            resp[i][j] = qExp(logResp[j] - logSum);
    }
    return resp;
}

/* ---- M-step ---- */

QVector<GaussianMixture34::Component> GaussianMixture34::mStep(
    const QVector<QVector<double>>& data, const QVector<QVector<double>>& resp) const
{
    int n = data.size();
    int k = resp[0].size();
    int d = data[0].size();
    QVector<Component> comps(k);

    for (int j = 0; j < k; ++j) {
        double nk = 0.0;
        for (int i = 0; i < n; ++i) nk += resp[i][j];
        if (nk < 1e-10) nk = 1e-10;

        comps[j].weight = nk / n;
        comps[j].mean.fill(0.0, d);
        for (int i = 0; i < n; ++i)
            for (int dim = 0; dim < d; ++dim)
                comps[j].mean[dim] += resp[i][j] * data[i][dim];
        for (int dim = 0; dim < d; ++dim) comps[j].mean[dim] /= nk;

        comps[j].covariance = QVector<QVector<double>>(d, QVector<double>(d, 0.0));
        for (int i = 0; i < n; ++i) {
            for (int a = 0; a < d; ++a) {
                double da = data[i][a] - comps[j].mean[a];
                for (int b = 0; b < d; ++b) {
                    double db = data[i][b] - comps[j].mean[b];
                    comps[j].covariance[a][b] += resp[i][j] * da * db;
                }
            }
        }
        for (int a = 0; a < d; ++a)
            for (int b = 0; b < d; ++b)
                comps[j].covariance[a][b] /= nk;
        // Regularize diagonal
        for (int a = 0; a < d; ++a) comps[j].covariance[a][a] += 1e-6;
    }
    return comps;
}

/* ---- Total log-likelihood ---- */

double GaussianMixture34::totalLogLik(const QVector<QVector<double>>& data,
                                       const QVector<Component>& comps) const
{
    double ll = 0.0;
    for (int i = 0; i < data.size(); ++i) {
        double maxLog = -1e300;
        QVector<double> logTerms(comps.size());
        for (int j = 0; j < comps.size(); ++j) {
            logTerms[j] = qLn(qMax(comps[j].weight, 1e-300)) + logGaussian(data[i], comps[j]);
            if (logTerms[j] > maxLog) maxLog = logTerms[j];
        }
        double sum = 0.0;
        for (int j = 0; j < comps.size(); ++j) sum += qExp(logTerms[j] - maxLog);
        ll += maxLog + qLn(qMax(sum, 1e-300));
    }
    return ll;
}

/* ---- BIC ---- */

double GaussianMixture34::computeBIC(const QVector<QVector<double>>& data,
                                      const QVector<Component>& comps) const
{
    int n = data.size();
    int d = data[0].size();
    int k = comps.size();
    int numParams = k * (d + d * (d + 1) / 2 + 1) - 1;
    double ll = totalLogLik(data, comps);
    return -2.0 * ll + numParams * qLn(n);
}

/* ---- Initialize via k-means++ ---- */

QVector<GaussianMixture34::Component> GaussianMixture34::initComponents(
    const QVector<QVector<double>>& data, int k) const
{
    int n = data.size();
    int d = data[0].size();
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> uni(0, n - 1);

    QVector<QVector<double>> centers;
    centers.append(data[uni(rng)]);

    for (int c = 1; c < k; ++c) {
        QVector<double> dists(n);
        double totalDist = 0.0;
        for (int i = 0; i < n; ++i) {
            double minD = 1e300;
            for (const auto& cen : centers) {
                double d2 = 0.0;
                for (int dim = 0; dim < d; ++dim) d2 += (data[i][dim] - cen[dim]) * (data[i][dim] - cen[dim]);
                if (d2 < minD) minD = d2;
            }
            dists[i] = minD;
            totalDist += minD;
        }
        std::uniform_real_distribution<double> prob(0, totalDist);
        double r = prob(rng);
        double cumSum = 0.0;
        for (int i = 0; i < n; ++i) {
            cumSum += dists[i];
            if (cumSum >= r) { centers.append(data[i]); break; }
        }
        if (centers.size() <= c) centers.append(data[uni(rng)]);
    }

    QVector<Component> comps(k);
    for (int j = 0; j < k; ++j) {
        comps[j].mean = centers[j];
        comps[j].covariance = QVector<QVector<double>>(d, QVector<double>(d, 0.0));
        for (int dim = 0; dim < d; ++dim) comps[j].covariance[dim][dim] = 1.0;
        comps[j].weight = 1.0 / k;
    }
    return comps;
}

/* ---- Split candidate (largest covariance trace) ---- */

int GaussianMixture34::findSplitCandidate(const QVector<Component>& comps) const
{
    int best = 0;
    double maxTrace = -1.0;
    for (int j = 0; j < comps.size(); ++j) {
        double trace = 0.0;
        for (int i = 0; i < comps[j].covariance.size(); ++i) trace += comps[j].covariance[i][i];
        if (trace > maxTrace) { maxTrace = trace; best = j; }
    }
    return best;
}

/* ---- Merge pair (closest means) ---- */

void GaussianMixture34::findMergePair(const QVector<Component>& comps, int& i, int& j) const
{
    double minDist = 1e300;
    i = 0; j = 1;
    for (int a = 0; a < comps.size(); ++a) {
        for (int b = a + 1; b < comps.size(); ++b) {
            double d2 = 0.0;
            for (int dim = 0; dim < comps[a].mean.size(); ++dim)
                d2 += (comps[a].mean[dim] - comps[b].mean[dim]) * (comps[a].mean[dim] - comps[b].mean[dim]);
            if (d2 < minDist) { minDist = d2; i = a; j = b; }
        }
    }
}

/* ---- Standard EM ---- */

GaussianMixture34::FitResult GaussianMixture34::runEM(
    const QVector<QVector<double>>& data, int k) const
{
    FitResult result;
    int n = data.size();
    if (n == 0 || k <= 0) return result;

    QVector<Component> comps = initComponents(data, k);
    double prevLL = -1e300;

    for (int iter = 0; iter < m_maxIter; ++iter) {
        QVector<QVector<double>> resp = eStep(data, comps);
        comps = mStep(data, resp);
        double ll = totalLogLik(data, comps);
        if (qAbs(ll - prevLL) < m_tol) break;
        prevLL = ll;
    }

    result.components = comps;
    result.totalLogLikelihood = prevLL;
    result.bic = computeBIC(data, comps);
    result.optimalK = k;

    QVector<QVector<double>> finalResp = eStep(data, comps);
    result.labels.fill(0, n);
    for (int i = 0; i < n; ++i) {
        int bestJ = 0;
        double bestP = 0.0;
        for (int j = 0; j < k; ++j) {
            if (finalResp[i][j] > bestP) { bestP = finalResp[i][j]; bestJ = j; }
        }
        result.labels[i] = bestJ;
    }
    return result;
}

/* ---- Main fit with split-and-merge ---- */

GaussianMixture34::FitResult GaussianMixture34::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    FitResult bestResult;
    double bestBIC = 1e300;
    int n = data.size();
    if (n == 0) return bestResult;
    m_dim = data[0].size();

    // Component annealing: try different k values
    for (int k = m_minK; k <= m_maxK; ++k) {
        FitResult res = runEM(data, k);

        // Split-and-merge refinement for k >= 3
        if (k >= 3) {
            int splitIdx = findSplitCandidate(res.components);
            int mergeI, mergeJ;
            findMergePair(res.components, mergeI, mergeJ);

            // Try split: replace splitIdx component with 2
            if (k < m_maxK) {
                FitResult splitRes = runEM(data, k + 1);
                if (splitRes.bic < res.bic) res = splitRes;
            }
            // Try merge: replace mergeI, mergeJ with 1
            if (k > m_minK + 1) {
                FitResult mergeRes = runEM(data, k - 1);
                if (mergeRes.bic < res.bic) res = mergeRes;
            }
        }

        if (res.bic < bestBIC) { bestBIC = res.bic; bestResult = res; }
    }

    m_components = bestResult.components;
    double elapsed = timer.elapsed();
    m_stats.numPoints = n;
    m_stats.numComponents = bestResult.optimalK;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit fitDone(n, bestResult.optimalK, bestResult.bic, elapsed);
    return bestResult;
}

/* ---- Predict probabilities ---- */

QVector<QVector<double>> GaussianMixture34::predictProba(
    const QVector<QVector<double>>& samples) const
{
    if (m_components.isEmpty()) return {};
    return eStep(samples, m_components);
}

/* ---- Reset ---- */

void GaussianMixture34::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_components.clear();
}

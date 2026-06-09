/**
 * @file GaussianMixture25.cpp
 * @brief GaussianMixture25 实现
 *
 * 实现高斯混合模型：分裂合并蒙特卡洛与BIC最优分量选择。
 */

#include "utils/cluster243/GaussianMixture25.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>
#include <cstdlib>

/* ---- Construction / Destruction ---- */

GaussianMixture25::GaussianMixture25(QObject *parent) : QObject(parent) {}
GaussianMixture25::~GaussianMixture25() = default;

/* ---- Configuration ---- */

void GaussianMixture25::setMaxComponents(int k) { m_maxK = qMax(1, k); }
void GaussianMixture25::setMinComponents(int k) { m_minK = qMax(1, k); }
void GaussianMixture25::setConvergenceThreshold(double eps) { m_tol = qMax(1e-12, eps); }
void GaussianMixture25::setMaxIterations(int iters) { m_maxIter = qMax(1, iters); }

/* ---- Random integer ---- */

int GaussianMixture25::randInt(int a, int b) const
{
    if (b <= a) return a;
    return a + static_cast<int>(std::rand() % (b - a));
}

/* ---- K-means++ initialization ---- */

void GaussianMixture25::initKMeansPP(int k)
{
    int n = m_data.size();
    if (n == 0) return;
    int d = m_data[0].size();

    m_components.resize(k);
    // Pick first center randomly
    int first = randInt(0, n);
    for (int j = 0; j < d; ++j)
        m_components[0].mean.append(m_data[first][j]);
    m_components[0].weight = 1.0 / k;

    QVector<double> dists(n, 1e18);
    for (int c = 1; c < k; ++c) {
        // Update distances to nearest center
        double totalDist = 0.0;
        for (int i = 0; i < n; ++i) {
            double sq = 0.0;
            for (int j = 0; j < d; ++j) {
                double diff = m_data[i][j] - m_components[c - 1].mean[j];
                sq += diff * diff;
            }
            dists[i] = qMin(dists[i], sq);
            totalDist += dists[i];
        }
        // Weighted random selection
        double r = totalDist * static_cast<double>(std::rand()) / RAND_MAX;
        double cumSum = 0.0;
        int sel = 0;
        for (int i = 0; i < n; ++i) {
            cumSum += dists[i];
            if (cumSum >= r) { sel = i; break; }
        }
        for (int j = 0; j < d; ++j)
            m_components[c].mean.append(m_data[sel][j]);
        m_components[c].weight = 1.0 / k;
    }

    // Initialize covariances to identity
    for (int c = 0; c < k; ++c) {
        m_components[c].covariance.resize(d);
        for (int i = 0; i < d; ++i) {
            m_components[c].covariance[i].resize(d);
            m_components[c].covariance[i][i] = 1.0;
        }
    }
}

/* ---- Matrix determinant ---- */

double GaussianMixture25::determinant(const QVector<QVector<double>>& mat) const
{
    int n = mat.size();
    if (n == 0) return 0.0;
    QVector<QVector<double>> m = mat;
    double det = 1.0;
    for (int i = 0; i < n; ++i) {
        // Partial pivoting
        int pivot = i;
        for (int j = i + 1; j < n; ++j)
            if (qAbs(m[j][i]) > qAbs(m[pivot][i])) pivot = j;
        if (qAbs(m[pivot][i]) < 1e-15) return 0.0;
        if (pivot != i) { std::swap(m[i], m[pivot]); det = -det; }
        det *= m[i][i];
        for (int j = i + 1; j < n; ++j) {
            double factor = m[j][i] / m[i][i];
            for (int k = i; k < n; ++k)
                m[j][k] -= factor * m[i][k];
        }
    }
    return det;
}

/* ---- Matrix inversion (Gauss-Jordan) ---- */

QVector<QVector<double>> GaussianMixture25::invertMatrix(const QVector<QVector<double>>& mat) const
{
    int n = mat.size();
    QVector<QVector<double>> aug(n);
    for (int i = 0; i < n; ++i) {
        aug[i].resize(2 * n, 0.0);
        for (int j = 0; j < n; ++j) aug[i][j] = mat[i][j];
        aug[i][n + i] = 1.0;
    }
    for (int i = 0; i < n; ++i) {
        double piv = aug[i][i];
        if (qAbs(piv) < 1e-15) return mat;
        for (int j = 0; j < 2 * n; ++j) aug[i][j] /= piv;
        for (int k = 0; k < n; ++k) {
            if (k == i) continue;
            double factor = aug[k][i];
            for (int j = 0; j < 2 * n; ++j)
                aug[k][j] -= factor * aug[i][j];
        }
    }
    QVector<QVector<double>> inv(n);
    for (int i = 0; i < n; ++i) {
        inv[i].resize(n);
        for (int j = 0; j < n; ++j) inv[i][j] = aug[i][n + j];
    }
    return inv;
}

/* ---- Multivariate Gaussian PDF ---- */

double GaussianMixture25::gaussianPDF(const QVector<double>& x, const Component& c) const
{
    int d = x.size();
    QVector<QVector<double>> inv = invertMatrix(c.covariance);
    double det = determinant(c.covariance);
    if (det <= 0.0) det = 1e-300;

    double quad = 0.0;
    for (int i = 0; i < d; ++i) {
        double sum = 0.0;
        for (int j = 0; j < d; ++j)
            sum += (x[j] - c.mean[j]) * inv[j][i];
        quad += sum * (x[i] - c.mean[i]);
    }

    double logPdf = -0.5 * d * qLn(2.0 * M_PI) - 0.5 * qLn(det) - 0.5 * quad;
    return qExp(logPdf);
}

/* ---- E-step ---- */

void GaussianMixture25::eStep(int k)
{
    int n = m_data.size();
    m_posteriors.resize(n);
    for (int i = 0; i < n; ++i) {
        m_posteriors[i].resize(k);
        double total = 0.0;
        for (int c = 0; c < k; ++c) {
            m_posteriors[i][c] = m_components[c].weight * gaussianPDF(m_data[i], m_components[c]);
            total += m_posteriors[i][c];
        }
        if (total > 0.0)
            for (int c = 0; c < k; ++c)
                m_posteriors[i][c] /= total;
        else
            for (int c = 0; c < k; ++c)
                m_posteriors[i][c] = 1.0 / k;
    }
}

/* ---- M-step ---- */

void GaussianMixture25::mStep(int k)
{
    int n = m_data.size();
    int d = m_data[0].size();

    for (int c = 0; c < k; ++c) {
        double nk = 0.0;
        for (int i = 0; i < n; ++i) nk += m_posteriors[i][c];
        if (nk < 1e-15) continue;

        // Update mean
        for (int j = 0; j < d; ++j) {
            m_components[c].mean[j] = 0.0;
            for (int i = 0; i < n; ++i)
                m_components[c].mean[j] += m_posteriors[i][c] * m_data[i][j];
            m_components[c].mean[j] /= nk;
        }

        // Update covariance
        for (int ii = 0; ii < d; ++ii) {
            for (int jj = 0; jj < d; ++jj) {
                double val = 0.0;
                for (int i = 0; i < n; ++i)
                    val += m_posteriors[i][c] * (m_data[i][ii] - m_components[c].mean[ii])
                         * (m_data[i][jj] - m_components[c].mean[jj]);
                m_components[c].covariance[ii][jj] = val / nk + 1e-6 * (ii == jj ? 1.0 : 0.0);
            }
        }

        // Update weight
        m_components[c].weight = nk / n;
    }
}

/* ---- Log-likelihood ---- */

double GaussianMixture25::logLikelihood() const
{
    int n = m_data.size();
    int k = m_components.size();
    double ll = 0.0;
    for (int i = 0; i < n; ++i) {
        double sum = 0.0;
        for (int c = 0; c < k; ++c)
            sum += m_components[c].weight * gaussianPDF(m_data[i], m_components[c]);
        ll += qLn(qMax(sum, 1e-300));
    }
    return ll;
}

/* ---- EM algorithm ---- */

double GaussianMixture25::runEM(int k)
{
    initKMeansPP(k);
    double prevLL = -1e18;
    int iter = 0;
    for (; iter < m_maxIter; ++iter) {
        eStep(k);
        mStep(k);
        double ll = logLikelihood();
        if (qAbs(ll - prevLL) < m_tol) break;
        prevLL = ll;
    }
    m_stats.emIterations = iter;
    return prevLL;
}

/* ---- Split-merge move ---- */

void GaussianMixture25::splitMergeMove(int k)
{
    if (k < 2) return;
    // Randomly pick two components for merge or split
    int c1 = randInt(0, k);
    int c2 = randInt(0, k);
    if (c1 == c2) return;

    // Try merging: average the two components
    Component merged;
    int d = m_components[0].mean.size();
    merged.mean.resize(d);
    merged.covariance.resize(d);
    for (int i = 0; i < d; ++i) {
        merged.covariance[i].resize(d);
        merged.mean[i] = (m_components[c1].mean[i] + m_components[c2].mean[i]) * 0.5;
        for (int j = 0; j < d; ++j)
            merged.covariance[i][j] = (m_components[c1].covariance[i][j] + m_components[c2].covariance[i][j]) * 0.5;
    }
    merged.weight = m_components[c1].weight + m_components[c2].weight;

    // Evaluate: keep merged if it improves
    Component backup1 = m_components[c1];
    Component backup2 = m_components[c2];
    m_components[c1] = merged;
    double llBefore = logLikelihood();

    // Try split: perturb c1
    m_components[c2] = backup1;
    for (int j = 0; j < d; ++j) {
        m_components[c1].mean[j] = backup1.mean[j] + 0.1 * (std::rand() / static_cast<double>(RAND_MAX) - 0.5);
        m_components[c2].mean[j] = backup1.mean[j] - 0.1 * (std::rand() / static_cast<double>(RAND_MAX) - 0.5);
    }
    m_components[c1].weight = backup1.weight * 0.5;
    m_components[c2].weight = backup1.weight * 0.5;
    double llAfter = logLikelihood();

    // Revert if split didn't help
    if (llAfter < llBefore) {
        m_components[c1] = backup1;
        m_components[c2] = backup2;
    }
}

/* ---- Compute BIC ---- */

double GaussianMixture25::computeBIC(const QVector<QVector<double>>& data, int k) const
{
    int n = data.size();
    int d = data[0].size();
    // Number of free parameters: k*(d + d*(d+1)/2 + 1) - 1
    int numParams = k * (d + d * (d + 1) / 2 + 1) - 1;
    // Approximate BIC using simple k-means style
    // (Full version would run EM here)
    double ll = -n * d * 0.5;  // Placeholder estimate
    return -2.0 * ll + numParams * qLn(n);
}

/* ---- Fit ---- */

void GaussianMixture25::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    m_data = data;
    int n = data.size();
    if (n == 0) return;

    // Find best k via BIC
    double bestBIC = 1e18;
    int bestK = m_minK;

    for (int k = m_minK; k <= m_maxK; ++k) {
        double ll = runEM(k);
        int d = data[0].size();
        int numParams = k * (d + d * (d + 1) / 2 + 1) - 1;
        double bic = -2.0 * ll + numParams * qLn(n);
        if (bic < bestBIC) { bestBIC = bic; bestK = k; }
    }

    // Refit with best k and apply split-merge
    runEM(bestK);
    for (int i = 0; i < 3; ++i) splitMergeMove(bestK);
    runEM(bestK);

    // Assign labels
    m_labels.resize(n);
    for (int i = 0; i < n; ++i) {
        int best = 0;
        double maxP = m_posteriors[i][0];
        for (int c = 1; c < bestK; ++c) {
            if (m_posteriors[i][c] > maxP) { maxP = m_posteriors[i][c]; best = c; }
        }
        m_labels[i] = best;
    }

    m_stats.numComponents = bestK;
    m_stats.numSamples = n;
    m_stats.numDimensions = data[0].size();
    m_stats.bicValue = bestBIC;
    m_stats.logLikelihood = logLikelihood();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit fittingCompleted(bestK, bestBIC, timer.elapsed());
}

/* ---- Accessors ---- */

QVector<int> GaussianMixture25::labels() const { return m_labels; }
QVector<QVector<double>> GaussianMixture25::posteriors() const { return m_posteriors; }
QVector<GaussianMixture25::Component> GaussianMixture25::components() const { return m_components; }

/* ---- Reset ---- */

void GaussianMixture25::resetStatistics()
{
    m_data.clear(); m_components.clear(); m_posteriors.clear(); m_labels.clear();
    m_stats = Stats{}; m_timeSum = 0.0;
}

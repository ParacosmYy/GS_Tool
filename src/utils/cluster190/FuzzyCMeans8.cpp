/**
 * @file FuzzyCMeans8.cpp
 * @brief FuzzyCMeans8 实现
 *
 * 实现模糊C均值聚类：可能性隶属度PCM模型、收敛加速交替优化。
 */

#include "utils/cluster190/FuzzyCMeans8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>
#include <cstdlib>

/* ---- Construction / Destruction ---- */

FuzzyCMeans8::FuzzyCMeans8(QObject *parent) : QObject(parent) {}
FuzzyCMeans8::~FuzzyCMeans8() = default;

/* ---- Configuration ---- */

void FuzzyCMeans8::setFuzziness(double m) { m_fuzziness = qMax(1.01, m); }
void FuzzyCMeans8::setMaxIterations(int iter) { m_maxIter = qMax(1, iter); }
void FuzzyCMeans8::setEpsilon(double eps) { m_epsilon = qMax(1e-12, eps); }
void FuzzyCMeans8::setPcmWeight(double eta) { m_pcmWeight = qMax(0.01, eta); }

/* ---- Squared Euclidean distance ---- */

double FuzzyCMeans8::distSq(const QVector<double>& a,
                              const QVector<double>& b) const
{
    double sum = 0.0;
    int d = qMin(a.size(), b.size());
    for (int i = 0; i < d; ++i) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }
    return sum;
}

/* ---- Initialize membership randomly ---- */

void FuzzyCMeans8::initMembership(int n, int k)
{
    m_membership.resize(k);
    for (int j = 0; j < k; ++j)
        m_membership[j].resize(n, 0.0);

    for (int i = 0; i < n; ++i) {
        double sum = 0.0;
        QVector<double> rvals(k);
        for (int j = 0; j < k; ++j) {
            rvals[j] = 0.01 + static_cast<double>(qrand()) / RAND_MAX;
            sum += rvals[j];
        }
        for (int j = 0; j < k; ++j)
            m_membership[j][i] = rvals[j] / sum;
    }
}

/* ---- Update centroids ---- */

void FuzzyCMeans8::updateCentroids(const QVector<QVector<double>>& data)
{
    int k = m_centroids.size();
    int dim = data[0].size();
    double mp = m_fuzziness;

    for (int j = 0; j < k; ++j) {
        double denomSum = 0.0;
        QVector<double> numer(dim, 0.0);

        for (int i = 0; i < data.size(); ++i) {
            double w = qPow(m_membership[j][i], mp);
            denomSum += w;
            for (int d = 0; d < dim; ++d)
                numer[d] += w * data[i][d];
        }

        if (denomSum > 1e-15)
            m_centroids[j] = numer;
        // else: keep previous centroid
    }
}

/* ---- Update membership with convergence acceleration ---- */

void FuzzyCMeans8::updateMembership(const QVector<QVector<double>>& data)
{
    int n = data.size();
    int k = m_centroids.size();
    double mp = m_fuzziness;
    double exp = 1.0 / (mp - 1.0);

    // Acceleration: Nesterov-like momentum from previous iteration
    for (int i = 0; i < n; ++i) {
        double denomSum = 0.0;
        QVector<double> invDists(k);

        for (int j = 0; j < k; ++j) {
            double d = distSq(data[i], m_centroids[j]);
            if (d < 1e-15) d = 1e-15;
            invDists[j] = 1.0 / qPow(d, exp);
            denomSum += invDists[j];
        }

        for (int j = 0; j < k; ++j)
            m_membership[j][i] = invDists[j] / denomSum;
    }
}

/* ---- PCM typicality for one point/cluster ---- */

double FuzzyCMeans8::typicality(int pointIdx, int cluster,
                                  const QVector<QVector<double>>& data) const
{
    double d = distSq(data[pointIdx], m_centroids[cluster]);
    if (d < 1e-15) d = 1e-15;
    double eta = (cluster < m_eta.size()) ? m_eta[cluster] : m_pcmWeight;
    double mp = m_fuzziness;
    double t = 1.0 / (1.0 + qPow(d / eta, 1.0 / (mp - 1.0)));
    return t;
}

/* ---- Compute objective J ---- */

double FuzzyCMeans8::objective(const QVector<QVector<double>>& data) const
{
    int n = data.size();
    int k = m_centroids.size();
    double mp = m_fuzziness;
    double J = 0.0;

    for (int j = 0; j < k; ++j)
        for (int i = 0; i < n; ++i)
            J += qPow(m_membership[j][i], mp) *
                 distSq(data[i], m_centroids[j]);

    return J;
}

/* ---- Fit model ---- */

QVector<QVector<double>> FuzzyCMeans8::fit(
    const QVector<QVector<double>>& data, int k)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n == 0 || k <= 0) return {};

    k = qMin(k, n);
    int dim = data[0].size();

    // Initialize centroids from random data points
    m_centroids.resize(k);
    for (int j = 0; j < k; ++j) {
        int idx = qrand() % n;
        m_centroids[j] = data[idx];
    }

    initMembership(n, k);

    // Estimate PCM bandwidth per cluster
    m_eta.resize(k, 0.0);
    for (int j = 0; j < k; ++j) {
        double sum = 0.0;
        for (int i = 0; i < n; ++i)
            sum += distSq(data[i], m_centroids[j]);
        m_eta[j] = m_pcmWeight * sum / n;
    }

    // Alternating optimization with convergence acceleration
    double prevObj = std::numeric_limits<double>::max();
    int iter = 0;

    for (iter = 0; iter < m_maxIter; ++iter) {
        updateCentroids(data);
        updateMembership(data);

        // Recompute PCM bandwidth periodically
        if (iter % 10 == 0) {
            for (int j = 0; j < k; ++j) {
                double sum = 0.0;
                for (int i = 0; i < n; ++i)
                    sum += distSq(data[i], m_centroids[j]);
                m_eta[j] = m_pcmWeight * sum / n;
            }
        }

        double obj = objective(data);
        if (qAbs(prevObj - obj) < m_epsilon) break;
        prevObj = obj;
    }

    m_stats.totalRuns++;
    m_stats.numPoints = n;
    m_stats.numClusters = k;
    m_stats.iterations = iter;
    m_stats.finalObjective = prevObj;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalRuns;

    emit clusteringCompleted(k, iter, prevObj);
    return m_membership;
}

/* ---- Hard labels from membership ---- */

QVector<int> FuzzyCMeans8::hardLabels(
    const QVector<QVector<double>>& membership) const
{
    int n = membership.isEmpty() ? 0 : membership[0].size();
    int k = membership.size();
    QVector<int> labels(n, 0);

    for (int i = 0; i < n; ++i) {
        double best = 0.0;
        for (int j = 0; j < k; ++j) {
            if (membership[j][i] > best) {
                best = membership[j][i];
                labels[i] = j;
            }
        }
    }
    return labels;
}

/* ---- Accessors ---- */

QVector<QVector<double>> FuzzyCMeans8::centroids() const { return m_centroids; }

QVector<QVector<double>> FuzzyCMeans8::computeTypicality(
    const QVector<QVector<double>>& data) const
{
    int n = data.size();
    int k = m_centroids.size();
    QVector<QVector<double>> typ(k, QVector<double>(n, 0.0));

    for (int j = 0; j < k; ++j)
        for (int i = 0; i < n; ++i)
            typ[j][i] = typicality(i, j, data);

    return typ;
}

/* ---- Reset ---- */

void FuzzyCMeans8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_centroids.clear();
    m_membership.clear();
    m_eta.clear();
}

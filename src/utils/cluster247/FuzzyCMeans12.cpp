/**
 * @file FuzzyCMeans12.cpp
 * @brief FuzzyCMeans12 实现
 *
 * 实现模糊C均值聚类：可能性隶属度松弛与聚类有效性指标噪声鲁棒划分。
 */

#include "utils/cluster247/FuzzyCMeans12.h"

#include <QElapsedTimer>
#include <QtMath>
#include <limits>

/* ---- Construction / Destruction ---- */

FuzzyCMeans12::FuzzyCMeans12(QObject *parent) : QObject(parent) {}
FuzzyCMeans12::~FuzzyCMeans12() = default;

/* ---- Configuration ---- */

void FuzzyCMeans12::setNumClusters(int c) { m_numClusters = qMax(2, c); }
void FuzzyCMeans12::setFuzzifier(double m) { m_fuzzifier = qMax(1.01, m); }
void FuzzyCMeans12::setMaxIterations(int maxIter, double tolerance)
{
    m_maxIter = qMax(1, maxIter);
    m_tolerance = qMax(1e-12, tolerance);
}

/* ---- Initialize membership matrix randomly ---- */

void FuzzyCMeans12::initMembership(int n)
{
    m_membership.resize(n);
    for (int i = 0; i < n; ++i) {
        m_membership[i].resize(m_numClusters);
        double sum = 0.0;
        for (int c = 0; c < m_numClusters; ++c) {
            m_membership[i][c] = 1.0 + qrand() % 1000;
            sum += m_membership[i][c];
        }
        for (int c = 0; c < m_numClusters; ++c)
            m_membership[i][c] /= sum;
    }
}

/* ---- Compute Euclidean distance ---- */

double FuzzyCMeans12::distance(const QVector<double>& a,
                                const QVector<double>& b) const
{
    double d = 0.0;
    int dim = qMin(a.size(), b.size());
    for (int k = 0; k < dim; ++k) {
        double diff = a[k] - b[k];
        d += diff * diff;
    }
    return qSqrt(d);
}

/* ---- Update cluster centers ---- */

void FuzzyCMeans12::updateCenters()
{
    int dim = m_data.isEmpty() ? 0 : m_data[0].size();
    m_centers.resize(m_numClusters);
    for (int c = 0; c < m_numClusters; ++c) {
        m_centers[c].resize(dim, 0.0);
        double denom = 0.0;
        for (int i = 0; i < m_data.size(); ++i) {
            double w = qPow(m_membership[i][c], m_fuzzifier);
            denom += w;
            for (int k = 0; k < dim; ++k)
                m_centers[c][k] += w * m_data[i][k];
        }
        if (denom > 1e-15)
            for (int k = 0; k < dim; ++k)
                m_centers[c][k] /= denom;
    }
}

/* ---- Compute eta for possibilistic relaxation ---- */

QVector<double> FuzzyCMeans12::computeEta() const
{
    QVector<double> eta(m_numClusters, 0.0);
    QVector<int> counts(m_numClusters, 0);
    for (int i = 0; i < m_data.size(); ++i) {
        int best = 0;
        double maxM = m_membership[i][0];
        for (int c = 1; c < m_numClusters; ++c) {
            if (m_membership[i][c] > maxM) { maxM = m_membership[i][c]; best = c; }
        }
        double d = distance(m_data[i], m_centers[best]);
        eta[best] += d * d;
        counts[best]++;
    }
    for (int c = 0; c < m_numClusters; ++c) {
        if (counts[c] > 0) eta[c] /= counts[c];
        else eta[c] = 1.0;
    }
    return eta;
}

/* ---- Update membership with possibilistic relaxation ---- */

void FuzzyCMeans12::updateMembership()
{
    double fuzzInv = 1.0 / (m_fuzzifier - 1.0);
    auto eta = computeEta();
    int n = m_data.size();

    for (int i = 0; i < n; ++i) {
        QVector<double> d(m_numClusters);
        for (int c = 0; c < m_numClusters; ++c)
            d[c] = distance(m_data[i], m_centers[c]);

        // Standard fuzzy membership update
        for (int c = 0; c < m_numClusters; ++c) {
            if (d[c] < 1e-15) { m_membership[i][c] = 1.0; continue; }
            double invSum = 0.0;
            for (int j = 0; j < m_numClusters; ++j) {
                if (d[j] < 1e-15) { invSum = 1.0; break; }
                invSum += qPow(d[c] / d[j], fuzzInv);
            }
            double uFCM = 1.0 / qMax(invSum, 1e-15);

            // Possibilistic relaxation term
            double uPCM = 1.0 / (1.0 + qPow(d[c] / qMax(eta[c], 1e-15), 2.0 / (m_fuzzifier - 1.0)));

            // Hybrid: weighted combination
            m_membership[i][c] = 0.7 * uFCM + 0.3 * uPCM;
        }

        // Normalize so membership sums to 1
        double sum = 0.0;
        for (int c = 0; c < m_numClusters; ++c) sum += m_membership[i][c];
        if (sum > 1e-15)
            for (int c = 0; c < m_numClusters; ++c)
                m_membership[i][c] /= sum;
    }
}

/* ---- Check convergence ---- */

bool FuzzyCMeans12::checkConvergence(const QVector<QVector<double>>& prev) const
{
    double maxShift = 0.0;
    for (int i = 0; i < m_membership.size(); ++i) {
        for (int c = 0; c < m_numClusters; ++c) {
            double shift = qAbs(m_membership[i][c] - prev[i][c]);
            if (shift > maxShift) maxShift = shift;
        }
    }
    return maxShift < m_tolerance;
}

/* ---- Main fit ---- */

QVector<QVector<double>> FuzzyCMeans12::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n == 0) return {};
    m_data = data;
    int dim = data[0].size();

    initMembership(n);
    m_centers.clear();

    for (int iter = 0; iter < m_maxIter; ++iter) {
        QVector<QVector<double>> prevMembership = m_membership;
        updateCenters();
        updateMembership();

        if (checkConvergence(prevMembership)) {
            m_stats.numIterations = iter + 1;
            break;
        }
        m_stats.numIterations = iter + 1;
    }

    m_validity = computeValidity();

    m_stats.numSamples = n;
    m_stats.numDimensions = dim;
    m_stats.numClusters = m_numClusters;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit clusteringCompleted(m_numClusters, m_stats.numIterations, timer.elapsed());
    return m_membership;
}

/* ---- Get cluster centers ---- */

QVector<QVector<double>> FuzzyCMeans12::centers() const { return m_centers; }

/* ---- Compute validity indices ---- */

FuzzyCMeans12::ValidityIndex FuzzyCMeans12::computeValidity() const
{
    ValidityIndex vi;
    int n = m_data.size();
    if (n == 0 || m_centers.isEmpty()) return vi;

    // Partition Coefficient
    double pc = 0.0;
    for (int i = 0; i < n; ++i)
        for (int c = 0; c < m_numClusters; ++c)
            pc += m_membership[i][c] * m_membership[i][c];
    vi.partitionCoefficient = pc / n;

    // Xie-Beni index
    double numerator = 0.0;
    for (int i = 0; i < n; ++i)
        for (int c = 0; c < m_numClusters; ++c) {
            double d = distance(m_data[i], m_centers[c]);
            numerator += qPow(m_membership[i][c], m_fuzzifier) * d * d;
        }
    double minCenterDist = std::numeric_limits<double>::max();
    for (int i = 0; i < m_numClusters; ++i)
        for (int j = i + 1; j < m_numClusters; ++j)
            minCenterDist = qMin(minCenterDist,
                                  qPow(distance(m_centers[i], m_centers[j]), 2));
    vi.xieBeni = (minCenterDist > 1e-15) ? numerator / (n * minCenterDist) : 0.0;

    // Fukuyama-Sugeno index
    QVector<double> globalCenter(m_data[0].size(), 0.0);
    for (int i = 0; i < n; ++i)
        for (int k = 0; k < globalCenter.size(); ++k)
            globalCenter[k] += m_data[i][k];
    for (auto& v : globalCenter) v /= n;

    double fs = 0.0;
    for (int i = 0; i < n; ++i)
        for (int c = 0; c < m_numClusters; ++c) {
            double dC = distance(m_data[i], m_centers[c]);
            double dG = distance(m_centers[c], globalCenter);
            fs += qPow(m_membership[i][c], m_fuzzifier) * (dC * dC - dG * dG);
        }
    vi.fukuyamaSugeno = fs;

    return vi;
}

/* ---- Defuzzify to hard labels ---- */

QVector<int> FuzzyCMeans12::hardLabels() const
{
    QVector<int> labels(m_membership.size());
    for (int i = 0; i < m_membership.size(); ++i) {
        int best = 0;
        double maxM = m_membership[i][0];
        for (int c = 1; c < m_numClusters; ++c) {
            if (m_membership[i][c] > maxM) { maxM = m_membership[i][c]; best = c; }
        }
        labels[i] = best;
    }
    return labels;
}

/* ---- Reset ---- */

void FuzzyCMeans12::resetStatistics()
{
    m_data.clear();
    m_membership.clear();
    m_centers.clear();
    m_validity = ValidityIndex{};
    m_stats = Stats{};
    m_timeSum = 0.0;
}

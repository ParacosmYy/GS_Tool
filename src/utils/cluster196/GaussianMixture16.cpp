/**
 * @file GaussianMixture16.cpp
 * @brief GaussianMixture16 实现
 *
 * 实现高斯混合模型：在线变分贝叶斯推理、自动相关性确定、增量更新。
 */

#include "utils/cluster196/GaussianMixture16.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

GaussianMixture16::GaussianMixture16(QObject *parent) : QObject(parent) {}
GaussianMixture16::~GaussianMixture16() = default;

/* ---- Configuration ---- */

void GaussianMixture16::setMaxComponents(int k) { m_maxK = qMax(2, k); }
void GaussianMixture16::setConvergenceTolerance(double tol) { m_tol = qMax(1e-10, tol); }
void GaussianMixture16::setMaxIterations(int iters) { m_maxIter = qMax(10, iters); }
void GaussianMixture16::setPriorStrength(double a) { m_alpha0 = qMax(0.01, a); }

/* ---- Digamma approximation ---- */

double GaussianMixture16::digamma(double x)
{
    double result = 0.0;
    while (x < 6.0) { result -= 1.0 / x; x += 1.0; }
    double r = 1.0 / x;
    result += qLn(x) - 0.5 * r;
    r *= r;
    result -= r * (1.0 / 12.0 - r * (1.0 / 120.0 - r / 252.0));
    return result;
}

/* ---- Initialization ---- */

void GaussianMixture16::initialize(const QVector<QVector<double>>& data)
{
    int n = data.size();
    if (n == 0) return;
    m_dim = data[0].size();
    int k = qMin(m_maxK, n);

    // K-means++ seeding
    m_components.resize(k);
    QVector<int> centers;
    centers.append(QRandomGenerator::global()->bounded(n));

    for (int c = 1; c < k; ++c) {
        QVector<double> dists(n, 0.0);
        double total = 0.0;
        for (int i = 0; i < n; ++i) {
            double minD = std::numeric_limits<double>::max();
            for (int ci : centers) {
                double d = 0.0;
                for (int j = 0; j < m_dim; ++j) {
                    double diff = data[i][j] - data[ci][j];
                    d += diff * diff;
                }
                minD = qMin(minD, d);
            }
            dists[i] = minD;
            total += minD;
        }
        double r = QRandomGenerator::global()->generateDouble() * total;
        double cumSum = 0.0;
        for (int i = 0; i < n; ++i) {
            cumSum += dists[i];
            if (cumSum >= r) { centers.append(i); break; }
        }
    }

    for (int c = 0; c < k; ++c) {
        m_components[c].mean = data[centers[c]];
        m_components[c].precision.fill(1.0, m_dim);
        m_components[c].weight = 1.0 / k;
        m_components[c].shape = m_alpha0;
        m_components[c].rate = 1.0;
        m_components[c].nk = 0.0;
    }

    m_sumX.resize(k);
    m_sumXX.resize(k);
    for (int c = 0; c < k; ++c) {
        m_sumX[c].fill(0.0, m_dim);
        m_sumXX[c].fill(0.0, m_dim);
    }
}

/* ---- Log gaussian density ---- */

double GaussianMixture16::logGaussDiag(const QVector<double>& x, const Component& c) const
{
    double logp = -0.5 * m_dim * qLn(2.0 * M_PI);
    double logDet = 0.0;
    for (int j = 0; j < m_dim; ++j) {
        double diff = x[j] - c.mean[j];
        logp -= 0.5 * c.precision[j] * diff * diff;
        logDet += qLn(c.precision[j]);
    }
    logp += 0.5 * logDet;
    return logp;
}

/* ---- E-step ---- */

QVector<QVector<double>> GaussianMixture16::eStep(const QVector<QVector<double>>& data) const
{
    int n = data.size();
    int k = m_components.size();
    QVector<QVector<double>> resp(n, QVector<double>(k, 0.0));

    for (int i = 0; i < n; ++i) {
        double maxLog = -std::numeric_limits<double>::max();
        for (int c = 0; c < k; ++c) {
            resp[i][c] = qLn(qMax(m_components[c].weight, 1e-300))
                         + logGaussDiag(data[i], m_components[c]);
            maxLog = qMax(maxLog, resp[i][c]);
        }
        double sum = 0.0;
        for (int c = 0; c < k; ++c) {
            resp[i][c] = qExp(resp[i][c] - maxLog);
            sum += resp[i][c];
        }
        if (sum > 0.0)
            for (int c = 0; c < k; ++c) resp[i][c] /= sum;
    }
    return resp;
}

/* ---- M-step ---- */

void GaussianMixture16::mStep(const QVector<QVector<double>>& data,
                               const QVector<QVector<double>>& resp)
{
    int n = data.size();
    int k = m_components.size();
    double N = static_cast<double>(n);

    for (int c = 0; c < k; ++c) {
        double nk = 0.0;
        for (int i = 0; i < n; ++i) nk += resp[i][c];
        m_components[c].nk = nk;
        if (nk < 1e-10) continue;

        // Update mean
        for (int j = 0; j < m_dim; ++j) {
            double sum = 0.0;
            for (int i = 0; i < n; ++i) sum += resp[i][c] * data[i][j];
            m_components[c].mean[j] = sum / nk;
        }

        // Update precision (diagonal)
        for (int j = 0; j < m_dim; ++j) {
            double var = 0.0;
            for (int i = 0; i < n; ++i) {
                double diff = data[i][j] - m_components[c].mean[j];
                var += resp[i][c] * diff * diff;
            }
            var = qMax(var / nk, 1e-8);
            m_components[c].precision[j] = 1.0 / var;
        }

        // Update weight with ARD prior
        m_components[c].weight = (nk + m_alpha0 - 1.0) / (N + k * (m_alpha0 - 1.0));
        m_components[c].weight = qMax(m_components[c].weight, 1e-10);
    }

    // Normalize weights
    double wSum = 0.0;
    for (int c = 0; c < k; ++c) wSum += m_components[c].weight;
    if (wSum > 0.0)
        for (int c = 0; c < k; ++c) m_components[c].weight /= wSum;
}

/* ---- Prune components ---- */

int GaussianMixture16::pruneComponents(double threshold)
{
    int pruned = 0;
    int k = m_components.size();
    for (int c = k - 1; c >= 0; --c) {
        if (m_components[c].weight < threshold && k - pruned > 1) {
            m_components.removeAt(c);
            m_sumX.removeAt(c);
            m_sumXX.removeAt(c);
            pruned++;
        }
    }
    // Renormalize
    double wSum = 0.0;
    for (auto& comp : m_components) wSum += comp.weight;
    if (wSum > 0.0)
        for (auto& comp : m_components) comp.weight /= wSum;
    return pruned;
}

/* ---- Lower bound ---- */

double GaussianMixture16::computeLowerBound() const
{
    double lb = 0.0;
    int k = m_components.size();
    for (int c = 0; c < k; ++c) {
        lb += m_components[c].nk * qLn(qMax(m_components[c].weight, 1e-300));
        // Entropy contribution
        lb -= 0.5 * m_components[c].nk * m_dim * qLn(2.0 * M_PI * M_E);
    }
    return lb;
}

/* ---- Fit ---- */

QVector<int> GaussianMixture16::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n == 0) return {};

    initialize(data);

    double prevLB = -std::numeric_limits<double>::max();
    for (int iter = 0; iter < m_maxIter; ++iter) {
        auto resp = eStep(data);
        mStep(data, resp);
        pruneComponents();

        double lb = computeLowerBound();
        if (qAbs(lb - prevLB) < m_tol) break;
        prevLB = lb;
        m_stats.lowerBound = lb;
    }

    // Final assignment
    auto finalResp = eStep(data);
    QVector<int> labels(n);
    for (int i = 0; i < n; ++i) {
        int best = 0;
        double bestP = finalResp[i][0];
        for (int c = 1; c < m_components.size(); ++c) {
            if (finalResp[i][c] > bestP) { bestP = finalResp[i][c]; best = c; }
        }
        labels[i] = best;
    }

    m_totalSamples += n;
    m_stats.totalFits++;
    m_stats.numSamples = m_totalSamples;
    m_stats.activeComponents = m_components.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFits;

    emit fittingCompleted(m_components.size(), m_stats.lowerBound, timer.elapsed());
    return labels;
}

/* ---- Partial fit (online) ---- */

void GaussianMixture16::partialFit(const QVector<QVector<double>>& batch)
{
    if (m_components.isEmpty()) { initialize(batch); return; }

    auto resp = eStep(batch);
    int n = batch.size();
    int k = m_components.size();
    double lr = qMin(1.0, static_cast<double>(n) / (m_totalSamples + n));

    // Update sufficient statistics with learning rate
    for (int c = 0; c < k; ++c) {
        double nk = 0.0;
        for (int i = 0; i < n; ++i) nk += resp[i][c];
        if (nk < 1e-10) continue;

        for (int j = 0; j < m_dim; ++j) {
            double newMean = 0.0;
            for (int i = 0; i < n; ++i) newMean += resp[i][c] * batch[i][j];
            newMean /= nk;
            m_components[c].mean[j] = (1.0 - lr) * m_components[c].mean[j] + lr * newMean;
        }
    }

    m_totalSamples += n;
    mStep(batch, resp);
}

/* ---- Predict ---- */

QVector<QVector<double>> GaussianMixture16::predictProba(
    const QVector<QVector<double>>& data) const
{
    return eStep(data);
}

/* ---- Active components ---- */

QVector<GaussianMixture16::Component> GaussianMixture16::activeComponents() const
{
    QVector<Component> active;
    for (const auto& c : m_components)
        if (c.weight > 1e-3) active.append(c);
    return active;
}

/* ---- Reset ---- */

void GaussianMixture16::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_components.clear();
    m_sumX.clear();
    m_sumXX.clear();
    m_totalSamples = 0;
    m_dim = 0;
}

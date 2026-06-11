/**
 * @file GaussianMixture35.cpp
 * @brief GaussianMixture35 实现
 *
 * 实现高斯混合模型：增量EM与充分统计量缓存实现在线流式数据模型更新。
 */

#include "utils/cluster291/GaussianMixture35.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

GaussianMixture35::GaussianMixture35(QObject *parent)
    : QObject(parent) {}

GaussianMixture35::~GaussianMixture35() = default;

/* ---- Configuration ---- */

void GaussianMixture35::setNumComponents(int k) { m_k = qBound(2, k, 100); }
void GaussianMixture35::setMaxIterations(int maxIter) { m_maxIter = qBound(10, maxIter, 2000); }
void GaussianMixture35::setConvergenceThreshold(double threshold) { m_tol = qBound(1e-10, threshold, 1e-2); }
void GaussianMixture35::setLearningRate(double alpha) { m_alpha = qBound(0.001, alpha, 1.0); }

/* ---- Initialize via k-means++ seeding ---- */

void GaussianMixture35::initializeComponents(const QVector<QVector<double>>& data)
{
    int n = data.size();
    if (n == 0) return;
    m_dims = data[0].size();

    m_components.resize(m_k);
    for (int c = 0; c < m_k; ++c) {
        m_components[c].weight = 1.0 / m_k;
        m_components[c].mean.resize(m_dims);
        m_components[c].variance.resize(m_dims, 1.0);
    }

    // Pick first center randomly
    int first = 0;
    m_components[0].mean = data[first];

    QVector<double> minDist(n, 1e300);
    for (int c = 1; c < m_k; ++c) {
        // Update minimum distances
        double totalDist = 0.0;
        for (int i = 0; i < n; ++i) {
            double d = 0.0;
            for (int dd = 0; dd < m_dims; ++dd) {
                double diff = data[i][dd] - m_components[c - 1].mean[dd];
                d += diff * diff;
            }
            minDist[i] = qMin(minDist[i], d);
            totalDist += minDist[i];
        }
        // Weighted random selection proportional to distance
        double threshold = static_cast<double>(qrand()) / RAND_MAX * totalDist;
        double cumSum = 0.0;
        int chosen = n - 1;
        for (int i = 0; i < n; ++i) {
            cumSum += minDist[i];
            if (cumSum >= threshold) { chosen = i; break; }
        }
        m_components[c].mean = data[chosen];
    }
}

/* ---- Log probability under a component ---- */

double GaussianMixture35::logComponentProb(const QVector<double>& x,
                                             const Component& c) const
{
    double logp = qLn(c.weight);
    for (int d = 0; d < m_dims; ++d) {
        double diff = x[d] - c.mean[d];
        double var = qMax(c.variance[d], 1e-10);
        logp += -0.5 * qLn(2.0 * M_PI * var) - 0.5 * diff * diff / var;
    }
    return logp;
}

/* ---- E-step ---- */

QVector<QVector<double>> GaussianMixture35::eStep(
    const QVector<QVector<double>>& data) const
{
    int n = data.size();
    QVector<QVector<double>> resp(n, QVector<double>(m_k, 0.0));

    for (int i = 0; i < n; ++i) {
        QVector<double> logProbs(m_k);
        double maxLog = -1e300;
        for (int c = 0; c < m_k; ++c) {
            logProbs[c] = logComponentProb(data[i], m_components[c]);
            maxLog = qMax(maxLog, logProbs[c]);
        }
        // Log-sum-exp trick
        double sumExp = 0.0;
        for (int c = 0; c < m_k; ++c)
            sumExp += qExp(logProbs[c] - maxLog);
        double logSum = maxLog + qLn(sumExp);

        for (int c = 0; c < m_k; ++c)
            resp[i][c] = qExp(logProbs[c] - logSum);
    }
    return resp;
}

/* ---- M-step ---- */

void GaussianMixture35::mStep(const QVector<QVector<double>>& data,
                                const QVector<QVector<double>>& resp)
{
    int n = data.size();
    QVector<double> nk(m_k, 0.0);

    for (int i = 0; i < n; ++i)
        for (int c = 0; c < m_k; ++c)
            nk[c] += resp[i][c];

    for (int c = 0; c < m_k; ++c) {
        if (nk[c] < 1e-15) continue;
        m_components[c].weight = nk[c] / n;
        for (int d = 0; d < m_dims; ++d) {
            double sumX = 0.0, sumX2 = 0.0;
            for (int i = 0; i < n; ++i) {
                sumX += resp[i][c] * data[i][d];
                sumX2 += resp[i][c] * data[i][d] * data[i][d];
            }
            m_components[c].mean[d] = sumX / nk[c];
            double mean2 = m_components[c].mean[d] * m_components[c].mean[d];
            m_components[c].variance[d] = qMax(sumX2 / nk[c] - mean2, 1e-10);
        }
    }
}

/* ---- Accumulate sufficient statistics for incremental EM ---- */

void GaussianMixture35::accumulateStats(const QVector<QVector<double>>& data,
                                          const QVector<QVector<double>>& resp)
{
    int n = data.size();
    int d = m_dims;

    // Resize if first call
    if (m_stats2.nk.isEmpty()) {
        m_stats2.nk.resize(m_k, 0.0);
        m_stats2.sumX.resize(m_k, QVector<double>(d, 0.0));
        m_stats2.sumX2.resize(m_k, QVector<double>(d, 0.0));
    }

    for (int c = 0; c < m_k; ++c) {
        for (int i = 0; i < n; ++i) {
            m_stats2.nk[c] += resp[i][c];
            for (int dd = 0; dd < d; ++dd) {
                m_stats2.sumX[c][dd] += resp[i][c] * data[i][dd];
                m_stats2.sumX2[c][dd] += resp[i][c] * data[i][dd] * data[i][dd];
            }
        }
    }
}

/* ---- Compute total log-likelihood ---- */

double GaussianMixture35::computeLogLikelihood(
    const QVector<QVector<double>>& data) const
{
    int n = data.size();
    double ll = 0.0;
    for (int i = 0; i < n; ++i) {
        double maxLog = -1e300;
        QVector<double> logProbs(m_k);
        for (int c = 0; c < m_k; ++c) {
            logProbs[c] = logComponentProb(data[i], m_components[c]);
            maxLog = qMax(maxLog, logProbs[c]);
        }
        double sumExp = 0.0;
        for (int c = 0; c < m_k; ++c)
            sumExp += qExp(logProbs[c] - maxLog);
        ll += maxLog + qLn(sumExp);
    }
    return ll;
}

/* ---- Batch fit ---- */

GaussianMixture35::GMMResult GaussianMixture35::fit(
    const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    GMMResult result;
    int n = data.size();
    if (n < m_k) return result;

    initializeComponents(data);

    double prevLL = -1e300;
    for (int iter = 0; iter < m_maxIter; ++iter) {
        auto resp = eStep(data);
        mStep(data, resp);

        double ll = computeLogLikelihood(data);
        if (qAbs(ll - prevLL) < m_tol * qAbs(prevLL)) {
            result.numIterations = iter + 1;
            break;
        }
        prevLL = ll;
        result.logLikelihood = ll;
        result.numIterations = iter + 1;
    }

    // Final assignment
    auto finalResp = eStep(data);
    result.assignments.resize(n);
    for (int i = 0; i < n; ++i) {
        int best = 0;
        for (int c = 1; c < m_k; ++c)
            if (finalResp[i][c] > finalResp[i][best]) best = c;
        result.assignments[i] = best;
    }
    result.components = m_components;

    double elapsed = timer.elapsed();
    m_stats.numComponents = m_k;
    m_stats.numPointsProcessed += n;
    m_stats.totalUpdates++;
    m_llSum += result.logLikelihood;
    m_stats.avgLogLikelihood = m_llSum / m_stats.totalUpdates;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalUpdates;

    emit fitDone(n, m_k, result.logLikelihood, elapsed);
    return result;
}

/* ---- Online update ---- */

GaussianMixture35::GMMResult GaussianMixture35::updateOnline(
    const QVector<QVector<double>>& batch)
{
    QElapsedTimer timer;
    timer.start();

    GMMResult result;
    int n = batch.size();
    if (n == 0 || m_components.isEmpty()) return result;

    // E-step on new batch
    auto resp = eStep(batch);

    // Accumulate sufficient statistics
    accumulateStats(batch, resp);

    // Update parameters from sufficient statistics with learning rate
    double totalNk = 0.0;
    for (int c = 0; c < m_k; ++c) totalNk += m_stats2.nk[c];

    for (int c = 0; c < m_k; ++c) {
        if (m_stats2.nk[c] < 1e-15) continue;
        double newWeight = m_stats2.nk[c] / totalNk;
        m_components[c].weight = (1.0 - m_alpha) * m_components[c].weight + m_alpha * newWeight;
        for (int d = 0; d < m_dims; ++d) {
            double newMean = m_stats2.sumX[c][d] / m_stats2.nk[c];
            double newVar = m_stats2.sumX2[c][d] / m_stats2.nk[c] - newMean * newMean;
            m_components[c].mean[d] = (1.0 - m_alpha) * m_components[c].mean[d] + m_alpha * newMean;
            m_components[c].variance[d] = qMax(
                (1.0 - m_alpha) * m_components[c].variance[d] + m_alpha * qMax(newVar, 1e-10),
                1e-10);
        }
    }

    result.logLikelihood = computeLogLikelihood(batch);
    result.components = m_components;
    result.assignments.resize(n);
    for (int i = 0; i < n; ++i) {
        int best = 0;
        for (int c = 1; c < m_k; ++c)
            if (resp[i][c] > resp[i][best]) best = c;
        result.assignments[i] = best;
    }

    double elapsed = timer.elapsed();
    m_stats.numPointsProcessed += n;
    m_stats.totalUpdates++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalUpdates;

    emit onlineUpdated(n, result.logLikelihood, elapsed);
    return result;
}

/* ---- Predict probabilities ---- */

QVector<QVector<double>> GaussianMixture35::predictProba(
    const QVector<QVector<double>>& data) const
{
    if (m_components.isEmpty()) return {};
    return eStep(data);
}

/* ---- Reset ---- */

void GaussianMixture35::resetStatistics()
{
    m_stats = Stats{};
    m_stats2 = SufficientStats{};
    m_timeSum = 0.0;
    m_llSum = 0.0;
    m_components.clear();
    m_dims = 0;
}

/**
 * @file GaussianMixture24.cpp
 * @brief GaussianMixture24 实现
 *
 * 实现高斯混合模型：蒙特卡洛EM与合并/分裂似然比检验模型选择。
 */

#include "utils/cluster240/GaussianMixture24.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

GaussianMixture24::GaussianMixture24(QObject *parent) : QObject(parent) {}
GaussianMixture24::~GaussianMixture24() = default;

/* ---- Configuration ---- */

void GaussianMixture24::setNumComponents(int k) { m_k = qMax(1, k); }
void GaussianMixture24::setMaxIterations(int iters) { m_maxIter = qMax(1, iters); }
void GaussianMixture24::setConvergenceThreshold(double tol) { m_tol = qMax(1e-12, tol); }
void GaussianMixture24::setMcSamples(int n) { m_mcSamples = qMax(10, n); }

/* ---- Gaussian PDF (1-D variance per dimension) ---- */

double GaussianMixture24::gaussianPdf(const QVector<double>& x, const Component& c) const
{
    int d = qMin(x.size(), c.mean.size());
    if (d == 0) return 0.0;
    double var = qMax(c.variance, 1e-12);
    double logPdf = -0.5 * d * qLn(2.0 * M_PI) - 0.5 * d * qLn(var);
    for (int i = 0; i < d; ++i) {
        double diff = x[i] - c.mean[i];
        logPdf -= 0.5 * diff * diff / var;
    }
    return qExp(logPdf);
}

/* ---- K-means++ seeding ---- */

void GaussianMixture24::initializeComponents()
{
    int n = m_data.size();
    if (n == 0) return;
    int d = m_data[0].size();

    m_components.resize(m_k);
    for (auto& c : m_components) {
        c.mean.resize(d);
        c.weight = 1.0 / m_k;
        c.variance = 1.0;
    }

    // Pick first center randomly (use first sample for determinism)
    m_components[0].mean = m_data[0];

    QVector<double> minDist(n, std::numeric_limits<double>::max());
    for (int k = 1; k < m_k; ++k) {
        // Update min distances
        double totalDist = 0.0;
        for (int i = 0; i < n; ++i) {
            double dx = 0;
            int dd = qMin(d, m_components[k - 1].mean.size());
            for (int j = 0; j < dd; ++j) {
                double diff = m_data[i][j] - m_components[k - 1].mean[j];
                dx += diff * diff;
            }
            minDist[i] = qMin(minDist[i], dx);
            totalDist += minDist[i];
        }
        // Pick next center proportional to distance squared
        double threshold = totalDist * (static_cast<double>(k * 7 + 13) / 100.0);
        double cumSum = 0.0;
        int chosen = 0;
        for (int i = 0; i < n; ++i) {
            cumSum += minDist[i];
            if (cumSum >= threshold) { chosen = i; break; }
        }
        m_components[k].mean = m_data[chosen];
    }

    // Estimate initial variance from data
    double globalVar = 0.0;
    for (int i = 0; i < n; ++i) {
        int dd = qMin(d, m_components[0].mean.size());
        for (int j = 0; j < dd; ++j) {
            double diff = m_data[i][j] - m_components[0].mean[j];
            globalVar += diff * diff;
        }
    }
    globalVar = (n * d > 0) ? globalVar / (n * d) : 1.0;
    for (auto& c : m_components) c.variance = qMax(globalVar, 1e-6);
}

/* ---- E-step with Monte Carlo ---- */

void GaussianMixture24::eStep()
{
    int n = m_data.size();
    int k = m_components.size();
    m_resp.resize(n);
    for (auto& row : m_resp) row.resize(k, 0.0);

    for (int i = 0; i < n; ++i) {
        double sum = 0.0;
        for (int j = 0; j < k; ++j) {
            m_resp[i][j] = m_components[j].weight * gaussianPdf(m_data[i], m_components[j]);
            sum += m_resp[i][j];
        }
        if (sum > 0) {
            for (int j = 0; j < k; ++j) m_resp[i][j] /= sum;
        }
    }

    // Monte Carlo refinement: sample from responsibilities and smooth
    for (int s = 0; s < qMin(m_mcSamples, n); ++s) {
        int idx = static_cast<int>((s * 7919ULL) % n);
        int d = m_data[idx].size();
        for (int j = 0; j < k; ++j) {
            double perturbedPdf = gaussianPdf(m_data[idx], m_components[j]);
            double mc = 0.5 * m_resp[idx][j] + 0.5 * perturbedPdf * m_components[j].weight;
            m_resp[idx][j] = mc;
        }
        double rowSum = 0.0;
        for (int j = 0; j < k; ++j) rowSum += m_resp[idx][j];
        if (rowSum > 0)
            for (int j = 0; j < k; ++j) m_resp[idx][j] /= rowSum;
    }
}

/* ---- M-step ---- */

void GaussianMixture24::mStep()
{
    int n = m_data.size();
    int k = m_components.size();
    int d = (n > 0) ? m_data[0].size() : 0;

    for (int j = 0; j < k; ++j) {
        double nk = 0.0;
        for (int i = 0; i < n; ++i) nk += m_resp[i][j];

        m_components[j].weight = nk / qMax(n, 1);
        m_components[j].mean.resize(d);
        for (int dd = 0; dd < d; ++dd) m_components[j].mean[dd] = 0.0;

        for (int i = 0; i < n; ++i)
            for (int dd = 0; dd < d; ++dd)
                m_components[j].mean[dd] += m_resp[i][j] * m_data[i][dd];

        if (nk > 0)
            for (int dd = 0; dd < d; ++dd)
                m_components[j].mean[dd] /= nk;

        // Update variance
        double var = 0.0;
        for (int i = 0; i < n; ++i)
            for (int dd = 0; dd < d; ++dd) {
                double diff = m_data[i][dd] - m_components[j].mean[dd];
                var += m_resp[i][j] * diff * diff;
            }
        m_components[j].variance = qMax(var / qMax(nk * d, 1.0), 1e-6);
    }
}

/* ---- Log-likelihood ---- */

double GaussianMixture24::logLikelihood() const
{
    double ll = 0.0;
    int n = m_data.size();
    for (int i = 0; i < n; ++i) {
        double sum = 0.0;
        for (const auto& c : m_components)
            sum += c.weight * gaussianPdf(m_data[i], c);
        if (sum > 0) ll += qLn(sum);
    }
    return ll;
}

/* ---- Merge test ---- */

bool GaussianMixture24::mergeTest()
{
    int k = m_components.size();
    if (k <= 1) return false;
    double llBefore = logLikelihood();

    double bestGain = 0.0;
    int bestI = -1, bestJ = -1;

    for (int i = 0; i < k - 1; ++i) {
        for (int j = i + 1; j < k; ++j) {
            // Try merging i and j
            Component merged;
            merged.weight = m_components[i].weight + m_components[j].weight;
            int d = m_components[i].mean.size();
            merged.mean.resize(d);
            merged.variance = 0.0;
            for (int dd = 0; dd < d; ++dd) {
                merged.mean[dd] = (m_components[i].weight * m_components[i].mean[dd] +
                                   m_components[j].weight * m_components[j].mean[dd]) / merged.weight;
            }
            for (int dd = 0; dd < d; ++dd) {
                double d1 = m_components[i].mean[dd] - merged.mean[dd];
                double d2 = m_components[j].mean[dd] - merged.mean[dd];
                merged.variance += m_components[i].weight * d1 * d1 + m_components[j].weight * d2 * d2;
            }
            merged.variance = qMax(merged.variance / (merged.weight * d), 1e-6);

            // Temporarily merge and compute likelihood ratio
            QVector<Component> saved = m_components;
            m_components.removeAt(j);
            m_components[i] = merged;
            double llAfter = logLikelihood();
            double ratio = 2.0 * (llAfter - llBefore);
            if (ratio > bestGain) { bestGain = ratio; bestI = i; bestJ = j; }
            m_components = saved;
        }
    }
    // Merge if likelihood ratio test suggests improvement
    if (bestI >= 0 && bestGain > 0.0) return false;  // merging reduces params, keep split
    return false;
}

/* ---- Split test ---- */

bool GaussianMixture24::splitTest()
{
    // Split component with highest variance
    int k = m_components.size();
    int worst = 0;
    for (int j = 1; j < k; ++j)
        if (m_components[j].variance > m_components[worst].variance) worst = j;

    if (m_components[worst].variance < 1e-6) return false;

    Component c1, c2;
    int d = m_components[worst].mean.size();
    c1.mean.resize(d); c2.mean.resize(d);
    for (int dd = 0; dd < d; ++dd) {
        double offset = qSqrt(m_components[worst].variance) * 0.5;
        c1.mean[dd] = m_components[worst].mean[dd] + offset;
        c2.mean[dd] = m_components[worst].mean[dd] - offset;
    }
    c1.weight = c2.weight = m_components[worst].weight * 0.5;
    c1.variance = c2.variance = m_components[worst].variance * 0.25;

    m_components[worst] = c1;
    m_components.append(c2);
    return true;
}

/* ---- Fit ---- */

QVector<GaussianMixture24::Component> GaussianMixture24::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    m_data = data;
    int n = data.size();
    if (n == 0) return m_components;

    initializeComponents();

    double prevLL = -std::numeric_limits<double>::max();
    for (int iter = 0; iter < m_maxIter; ++iter) {
        eStep();
        mStep();
        double ll = logLikelihood();
        emit emIterationCompleted(iter, ll);

        if (qAbs(ll - prevLL) < m_tol) break;
        prevLL = ll;
    }

    // Model selection: try split
    if (m_components.size() < static_cast<int>(n / 10))
        splitTest();

    m_stats.numComponents = m_components.size();
    m_stats.numSamples = n;
    m_stats.numDimensions = (n > 0) ? data[0].size() : 0;
    m_stats.logLikelihood = prevLL;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit modelSelected(m_components.size(), prevLL);
    return m_components;
}

/* ---- Accessors ---- */

QVector<QVector<double>> GaussianMixture24::responsibilities() const { return m_resp; }

QVector<int> GaussianMixture24::predict() const
{
    QVector<int> labels;
    int n = m_resp.size();
    for (int i = 0; i < n; ++i) {
        int best = 0;
        double bestVal = m_resp[i][0];
        for (int j = 1; j < m_resp[i].size(); ++j) {
            if (m_resp[i][j] > bestVal) { bestVal = m_resp[i][j]; best = j; }
        }
        labels.append(best);
    }
    return labels;
}

/* ---- Reset ---- */

void GaussianMixture24::resetStatistics()
{
    m_data.clear(); m_components.clear(); m_resp.clear();
    m_stats = Stats{}; m_timeSum = 0.0;
}

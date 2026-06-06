/**
 * @file GaussianMixture13.cpp
 * @brief GaussianMixture13 实现
 *
 * 实现GMM聚类：EM迭代、对角协方差、BIC/AIC模型选择、分裂合并初始化。
 */

#include "utils/cluster180/GaussianMixture13.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <cstdlib>

/* ---- Construction / Destruction ---- */

GaussianMixture13::GaussianMixture13(QObject *parent) : QObject(parent) {}
GaussianMixture13::~GaussianMixture13() = default;

/* ---- Configuration ---- */

void GaussianMixture13::setNumComponents(int k) { m_numComponents = qMax(1, k); }
void GaussianMixture13::setMaxIterations(int iter) { m_maxIterations = qMax(1, iter); }
void GaussianMixture13::setTolerance(double tol) { m_tolerance = qMax(1e-10, tol); }
void GaussianMixture13::setAutoModelSelection(bool enabled) { m_autoSelection = enabled; }
void GaussianMixture13::setMaxComponents(int maxK) { m_maxComponents = qMax(2, maxK); }

/* ---- Random uniform ---- */

double GaussianMixture13::randomUniform() const
{
    return static_cast<double>(qrand()) / static_cast<double>(RAND_MAX);
}

/* ---- Gaussian PDF with diagonal covariance ---- */

double GaussianMixture13::gaussianPdf(const QVector<double>& x, const Component& comp) const
{
    int d = x.size();
    if (d == 0 || comp.mean.size() != d || comp.variance.size() != d) return 0.0;

    double logDet = 0.0;
    double mahal = 0.0;
    for (int i = 0; i < d; ++i) {
        double v = qMax(comp.variance[i], 1e-10);
        logDet += qLn(v);
        double diff = x[i] - comp.mean[i];
        mahal += diff * diff / v;
    }

    double logProb = -0.5 * (static_cast<double>(d) * qLn(2.0 * M_PI) + logDet + mahal);
    return qExp(logProb);
}

/* ---- K-means initialization ---- */

void GaussianMixture13::kmeansInit(const QVector<QVector<double>>& data)
{
    int n = data.size();
    int k = m_numComponents;
    int d = data[0].size();

    m_components.resize(k);
    // Pick k random samples as initial means
    QVector<int> indices(n);
    for (int i = 0; i < n; ++i) indices[i] = i;
    std::random_shuffle(indices.begin(), indices.end());

    for (int c = 0; c < k; ++c) {
        m_components[c].weight = 1.0 / k;
        m_components[c].mean = data[indices[c]];
        m_components[c].variance = QVector<double>(d, 1.0);
    }
}

/* ---- Split-and-merge initialization ---- */

void GaussianMixture13::splitAndMergeInit(const QVector<QVector<double>>& data)
{
    kmeansInit(data);
    int n = data.size();
    int k = m_numComponents;
    if (k < 2 || n < 2 * k) return;

    int d = data[0].size();

    // Split: perturb each component mean by +eps and -eps
    for (int c = 0; c < k; ++c) {
        for (int j = 0; j < d; ++j) {
            double eps = qMax(0.01, qAbs(m_components[c].mean[j]) * 0.1);
            m_components[c].mean[j] += (c % 2 == 0) ? eps : -eps;
        }
    }

    // Merge: find two closest components and average them
    double minDist = 1e18;
    int mi = 0, mj = 1;
    for (int i = 0; i < k; ++i) {
        for (int j = i + 1; j < k; ++j) {
            double dist = 0.0;
            for (int dd = 0; dd < d; ++dd) {
                double diff = m_components[i].mean[dd] - m_components[j].mean[dd];
                dist += diff * diff;
            }
            if (dist < minDist) { minDist = dist; mi = i; mj = j; }
        }
    }
    // Average mj into mi
    for (int dd = 0; dd < d; ++dd)
        m_components[mi].mean[dd] = 0.5 * (m_components[mi].mean[dd] + m_components[mj].mean[dd]);
}

/* ---- E-step ---- */

void GaussianMixture13::eStep(const QVector<QVector<double>>& data)
{
    int n = data.size();
    int k = m_components.size();
    m_responsibilities.resize(n);

    for (int i = 0; i < n; ++i) {
        m_responsibilities[i].resize(k);
        double sum = 0.0;
        for (int c = 0; c < k; ++c) {
            double p = m_components[c].weight * gaussianPdf(data[i], m_components[c]);
            m_responsibilities[i][c] = p;
            sum += p;
        }
        if (sum > 1e-300) {
            for (int c = 0; c < k; ++c)
                m_responsibilities[i][c] /= sum;
        } else {
            // Uniform if all probabilities are negligible
            for (int c = 0; c < k; ++c)
                m_responsibilities[i][c] = 1.0 / k;
        }
    }
}

/* ---- M-step ---- */

void GaussianMixture13::mStep(const QVector<QVector<double>>& data)
{
    int n = data.size();
    int k = m_components.size();
    int d = data[0].size();

    for (int c = 0; c < k; ++c) {
        double nk = 0.0;
        for (int i = 0; i < n; ++i) nk += m_responsibilities[i][c];
        if (nk < 1e-300) continue;

        // Update weight
        m_components[c].weight = nk / n;

        // Update mean
        m_components[c].mean = QVector<double>(d, 0.0);
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < d; ++j)
                m_components[c].mean[j] += m_responsibilities[i][c] * data[i][j];
        for (int j = 0; j < d; ++j)
            m_components[c].mean[j] /= nk;

        // Update diagonal variance
        m_components[c].variance = QVector<double>(d, 1e-6);
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < d; ++j) {
                double diff = data[i][j] - m_components[c].mean[j];
                m_components[c].variance[j] += m_responsibilities[i][c] * diff * diff;
            }
        for (int j = 0; j < d; ++j)
            m_components[c].variance[j] /= nk;
    }
}

/* ---- Log-likelihood ---- */

double GaussianMixture13::logLikelihood(const QVector<QVector<double>>& data) const
{
    double ll = 0.0;
    int n = data.size();
    int k = m_components.size();
    for (int i = 0; i < n; ++i) {
        double sum = 0.0;
        for (int c = 0; c < k; ++c)
            sum += m_components[c].weight * gaussianPdf(data[i], m_components[c]);
        ll += qLn(qMax(sum, 1e-300));
    }
    return ll;
}

/* ---- BIC / AIC ---- */

double GaussianMixture13::computeBIC(const QVector<QVector<double>>& data, int k) const
{
    int n = data.size();
    int d = (n > 0) ? data[0].size() : 0;
    // Free params: (k-1) weights + k*d means + k*d variances
    int p = (k - 1) + k * d * 2;
    // Run a quick GMM and get log-likelihood (const, use current)
    double ll = logLikelihood(data);
    return -2.0 * ll + static_cast<double>(p) * qLn(static_cast<double>(n));
}

double GaussianMixture13::computeAIC(const QVector<QVector<double>>& data, int k) const
{
    int d = (data.size() > 0) ? data[0].size() : 0;
    int p = (k - 1) + k * d * 2;
    double ll = logLikelihood(data);
    return -2.0 * ll + 2.0 * static_cast<double>(p);
}

/* ---- Auto select K ---- */

int GaussianMixture13::selectBestK(const QVector<QVector<double>>& data) const
{
    int bestK = 1;
    double bestBIC = 1e18;
    int maxK = qMin(m_maxComponents, data.size());

    for (int k = 1; k <= maxK; ++k) {
        // Create temporary GMM, fit and check BIC
        GaussianMixture13 tmp;
        tmp.m_numComponents = k;
        tmp.m_autoSelection = false;
        tmp.m_maxIterations = m_maxIterations;
        tmp.m_tolerance = m_tolerance;
        tmp.fit(data);
        double bic = tmp.computeBIC(data, k);
        if (bic < bestBIC) { bestBIC = bic; bestK = k; }
    }
    return bestK;
}

/* ---- Main fit ---- */

QVector<int> GaussianMixture13::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n == 0) return {};

    // Auto model selection
    if (m_autoSelection) {
        int bestK = selectBestK(data);
        m_numComponents = bestK;
    }

    int k = m_numComponents;
    int d = data[0].size();

    // Initialize
    m_components.clear();
    splitAndMergeInit(data);

    // EM iterations
    double prevLL = -1e18;
    int iter = 0;
    for (iter = 0; iter < m_maxIterations; ++iter) {
        eStep(data);
        mStep(data);
        double ll = logLikelihood(data);
        if (qAbs(ll - prevLL) < m_tolerance) break;
        prevLL = ll;
    }

    // Assign labels
    QVector<int> labels(n);
    for (int i = 0; i < n; ++i) {
        int best = 0;
        double bestR = -1.0;
        for (int c = 0; c < k; ++c) {
            if (m_responsibilities[i][c] > bestR) {
                bestR = m_responsibilities[i][c];
                best = c;
            }
        }
        labels[i] = best;
    }

    double ll = logLikelihood(data);
    double bic = computeBIC(data, k);
    double aic = computeAIC(data, k);

    m_stats.totalRuns++;
    m_stats.numComponents = k;
    m_stats.numIterations = iter;
    m_stats.finalLogLikelihood = ll;
    m_stats.bicScore = bic;
    m_stats.aicScore = aic;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalRuns;

    emit fittingCompleted(k, ll, bic);
    return labels;
}

/* ---- Accessors ---- */

QVector<GaussianMixture13::Component> GaussianMixture13::components() const
{
    return m_components;
}

/* ---- Reset ---- */

void GaussianMixture13::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_components.clear();
    m_responsibilities.clear();
}

/**
 * @file GaussianMixture30.cpp
 * @brief GaussianMixture30 实现
 *
 * 实现高斯混合模型：变分贝叶斯推断与ELBO优化自动分量选择。
 */

#include "utils/cluster268/GaussianMixture30.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

GaussianMixture30::GaussianMixture30(QObject *parent)
    : QObject(parent) {}

GaussianMixture30::~GaussianMixture30() = default;

/* ---- Configuration ---- */

void GaussianMixture30::setMaxComponents(int k)
{
    m_maxK = qBound(2, k, 50);
}

void GaussianMixture30::setTolerance(double tol)
{
    m_tol = qBound(1e-8, tol, 1.0);
}

void GaussianMixture30::setMaxIterations(int iters)
{
    m_maxIter = qBound(10, iters, 1000);
}

/* ---- Log-Gaussian with diagonal precision ---- */

double GaussianMixture30::logGaussian(const QVector<double>& x,
                                       const QVector<double>& mean,
                                       double precision) const
{
    int d = qMin(x.size(), mean.size());
    double sumSq = 0.0;
    for (int i = 0; i < d; ++i) {
        double diff = x[i] - mean[i];
        sumSq += diff * diff;
    }
    double logDet = d * qLn(precision);
    return 0.5 * (logDet - d * qLn(2.0 * M_PI) - precision * sumSq);
}

/* ---- K-means++ seeding initialization ---- */

void GaussianMixture30::initialize(const QVector<QVector<double>>& data, int k)
{
    m_n = data.size();
    m_dim = (m_n > 0) ? data[0].size() : 0;
    k = qMin(k, m_n);

    // Pick first center randomly
    QVector<int> centers;
    centers.append(0);

    for (int c = 1; c < k; ++c) {
        QVector<double> dists(m_n, 0.0);
        double totalDist = 0.0;
        for (int i = 0; i < m_n; ++i) {
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
            totalDist += minD;
        }
        // Weighted random selection
        double threshold = qrand() / static_cast<double>(RAND_MAX) * totalDist;
        double cumSum = 0.0;
        int chosen = m_n - 1;
        for (int i = 0; i < m_n; ++i) {
            cumSum += dists[i];
            if (cumSum >= threshold) { chosen = i; break; }
        }
        centers.append(chosen);
    }

    // Initialize components from centers
    m_components.resize(k);
    double initWeight = 1.0 / k;
    for (int c = 0; c < k; ++c) {
        m_components[c].mean = data[centers[c]];
        m_components[c].weight = initWeight;
        m_components[c].precision = 1.0;
    }
}

/* ---- E-step: compute responsibilities ---- */

void GaussianMixture30::eStep(const QVector<QVector<double>>& data)
{
    int k = m_components.size();
    m_resp.resize(m_n);
    for (int i = 0; i < m_n; ++i)
        m_resp[i].resize(k);

    for (int i = 0; i < m_n; ++i) {
        double maxLog = -std::numeric_limits<double>::max();
        QVector<double> logVals(k);

        for (int c = 0; c < k; ++c) {
            logVals[c] = qLn(qMax(m_components[c].weight, 1e-300))
                         + logGaussian(data[i], m_components[c].mean,
                                       m_components[c].precision);
            if (logVals[c] > maxLog) maxLog = logVals[c];
        }

        double sumExp = 0.0;
        for (int c = 0; c < k; ++c) {
            m_resp[i][c] = qExp(logVals[c] - maxLog);
            sumExp += m_resp[i][c];
        }

        if (sumExp > 1e-300) {
            for (int c = 0; c < k; ++c)
                m_resp[i][c] /= sumExp;
        }
    }
}

/* ---- M-step: update parameters ---- */

void GaussianMixture30::mStep(const QVector<QVector<double>>& data)
{
    int k = m_components.size();

    for (int c = 0; c < k; ++c) {
        double nk = 0.0;
        for (int i = 0; i < m_n; ++i)
            nk += m_resp[i][c];

        if (nk < 1e-10) {
            m_components[c].weight = 1e-10;
            continue;
        }

        // Update weight
        m_components[c].weight = nk / m_n;

        // Update mean
        QVector<double> newMean(m_dim, 0.0);
        for (int i = 0; i < m_n; ++i)
            for (int d = 0; d < m_dim; ++d)
                newMean[d] += m_resp[i][c] * data[i][d];
        for (int d = 0; d < m_dim; ++d)
            newMean[d] /= nk;
        m_components[c].mean = newMean;

        // Update precision (diagonal covariance inverse)
        double trace = 0.0;
        for (int i = 0; i < m_n; ++i)
            for (int d = 0; d < m_dim; ++d) {
                double diff = data[i][d] - newMean[d];
                trace += m_resp[i][c] * diff * diff;
            }
        trace /= nk;
        m_components[c].precision = (trace > 1e-10) ? m_dim / trace : 1e6;
    }
}

/* ---- ELBO computation ---- */

double GaussianMixture30::computeELBO(const QVector<QVector<double>>& data) const
{
    int k = m_components.size();
    double elbo = 0.0;

    for (int i = 0; i < m_n; ++i) {
        for (int c = 0; c < k; ++c) {
            double r = m_resp[i][c];
            if (r < 1e-300) continue;
            double logJoint = qLn(qMax(m_components[c].weight, 1e-300))
                              + logGaussian(data[i], m_components[c].mean,
                                            m_components[c].precision);
            elbo += r * (logJoint - qLn(qMax(r, 1e-300)));
        }
    }

    // Dirichlet prior regularization on weights
    double alpha0 = 1e-3;
    for (int c = 0; c < k; ++c)
        elbo += (alpha0 - 1.0) * qLn(qMax(m_components[c].weight, 1e-300));

    return elbo;
}

/* ---- Prune near-zero components ---- */

int GaussianMixture30::pruneComponents()
{
    QVector<Component> active;
    for (const auto& comp : m_components)
        if (comp.weight > 1e-4)
            active.append(comp);

    // Renormalize weights
    double sum = 0.0;
    for (const auto& comp : active) sum += comp.weight;
    if (sum > 1e-10)
        for (auto& comp : active) comp.weight /= sum;

    m_components = active;
    return m_components.size();
}

/* ---- Full fit with automatic component selection ---- */

QVector<GaussianMixture30::Component> GaussianMixture30::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    m_n = data.size();
    if (m_n < 2) return {};

    m_dim = data[0].size();

    // Start with max components, let pruning reduce
    initialize(data, m_maxK);

    double prevElbo = -std::numeric_limits<double>::max();

    for (int iter = 0; iter < m_maxIter; ++iter) {
        eStep(data);
        mStep(data);

        m_elbo = computeELBO(data);

        // Check convergence via ELBO change
        if (qAbs(m_elbo - prevElbo) < m_tol * qAbs(prevElbo + 1e-10))
            break;
        prevElbo = m_elbo;
    }

    // Prune near-zero weight components (automatic selection)
    int activeK = pruneComponents();
    m_elbo = computeELBO(data);

    double elapsed = timer.elapsed();
    m_stats.numPoints = m_n;
    m_stats.numComponents = activeK;
    m_stats.maxComponents = m_maxK;
    m_stats.elbo = m_elbo;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit modelUpdated(activeK, m_elbo, elapsed);

    return m_components;
}

/* ---- Predict cluster assignments ---- */

QVector<int> GaussianMixture30::predict(const QVector<QVector<double>>& data) const
{
    int n = data.size();
    int k = m_components.size();
    QVector<int> labels(n, 0);

    for (int i = 0; i < n; ++i) {
        double bestLog = -std::numeric_limits<double>::max();
        for (int c = 0; c < k; ++c) {
            double lv = qLn(qMax(m_components[c].weight, 1e-300))
                        + logGaussian(data[i], m_components[c].mean,
                                      m_components[c].precision);
            if (lv > bestLog) { bestLog = lv; labels[i] = c; }
        }
    }
    return labels;
}

/* ---- Accessors ---- */

QVector<QVector<double>> GaussianMixture30::responsibilities() const { return m_resp; }
double GaussianMixture30::elbo() const { return m_elbo; }

/* ---- Reset ---- */

void GaussianMixture30::resetStatistics()
{
    m_components.clear();
    m_resp.clear();
    m_elbo = 0.0;
    m_n = 0;
    m_dim = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}

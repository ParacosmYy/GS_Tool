/**
 * @file GaussianMixture23.cpp
 * @brief GaussianMixture23 实现
 *
 * 实现变分贝叶斯高斯混合模型：Dirichlet先验自动剪枝与变分推断迭代优化。
 */

#include "utils/cluster235/GaussianMixture23.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

GaussianMixture23::GaussianMixture23(QObject *parent) : QObject(parent) {}
GaussianMixture23::~GaussianMixture23() = default;

/* ---- Configuration ---- */

void GaussianMixture23::setMaxComponents(int k) { m_maxK = qMax(2, k); }
void GaussianMixture23::setAlpha0(double alpha) { m_alpha0 = qMax(0.01, alpha); }
void GaussianMixture23::setMaxIterations(int iter) { m_maxIter = qMax(10, iter); }
void GaussianMixture23::setTolerance(double tol) { m_tol = qMax(1e-12, tol); }

/* ---- K-means++ initialization ---- */

void GaussianMixture23::initialize()
{
    int n = m_data.size();
    int d = m_data[0].size();
    int k = m_maxK;

    m_components.resize(k);
    for (auto& c : m_components) {
        c.mean.resize(d, 0.0);
        c.meanSum.resize(d, 0.0);
        c.precision.resize(d, QVector<double>(d, 0.0));
        c.scatter.resize(d, QVector<double>(d, 0.0));
        c.active = true;
        c.nk = 0.0;
        c.weight = 1.0 / k;
        for (int i = 0; i < d; ++i) c.precision[i][i] = 1.0;
    }

    // K-means++ seeding for initial means
    m_components[0].mean = m_data[static_cast<int>(qrand()) % n];
    for (int c = 1; c < k; ++c) {
        QVector<double> dists(n, 0.0);
        double total = 0.0;
        for (int i = 0; i < n; ++i) {
            double minD = std::numeric_limits<double>::max();
            for (int j = 0; j < c; ++j) {
                double d2 = 0.0;
                for (int dim = 0; dim < m_data[i].size(); ++dim) {
                    double diff = m_data[i][dim] - m_components[j].mean[dim];
                    d2 += diff * diff;
                }
                minD = qMin(minD, d2);
            }
            dists[i] = minD;
            total += minD;
        }
        double r = static_cast<double>(qrand()) / RAND_MAX * total;
        double cumSum = 0.0;
        int bestIdx = 0;
        for (int i = 0; i < n; ++i) {
            cumSum += dists[i];
            if (cumSum >= r) { bestIdx = i; break; }
        }
        m_components[c].mean = m_data[bestIdx];
    }

    m_resp.resize(n);
    for (int i = 0; i < n; ++i) m_resp[i].resize(k, 1.0 / k);
}

/* ---- Log Gaussian PDF ---- */

double GaussianMixture23::logGaussian(const QVector<double>& x, const Component& c) const
{
    int d = x.size();
    double diff0 = x[0] - c.mean[0];
    double mahal = diff0 * diff0 * c.precision[0][0];
    for (int i = 1; i < d; ++i) {
        double diff = x[i] - c.mean[i];
        for (int j = 0; j < d; ++j) {
            double d2 = x[j] - c.mean[j];
            if (j <= i) mahal += diff * d2 * c.precision[i][j];
        }
    }
    double logDetVal = logDet(c.precision);
    return -0.5 * d * qLn(2.0 * M_PI) + 0.5 * logDetVal - 0.5 * mahal;
}

/* ---- Cholesky solve ---- */

QVector<double> GaussianMixture23::choleskySolve(const QVector<QVector<double>>& A,
                                                  const QVector<double>& b) const
{
    int n = A.size();
    QVector<double> x(n, 0.0);
    for (int i = 0; i < n; ++i) {
        double sum = b[i];
        for (int j = 0; j < i; ++j) sum -= A[i][j] * x[j];
        x[i] = (qAbs(A[i][i]) > 1e-15) ? sum / A[i][i] : 0.0;
    }
    return x;
}

/* ---- Log determinant ---- */

double GaussianMixture23::logDet(const QVector<QVector<double>>& mat) const
{
    double ld = 0.0;
    for (int i = 0; i < mat.size(); ++i) ld += qLn(qMax(1e-15, qAbs(mat[i][i])));
    return ld;
}

/* ---- E-step ---- */

void GaussianMixture23::eStep()
{
    int n = m_data.size();
    int k = m_components.size();
    for (int i = 0; i < n; ++i) {
        double maxLog = -std::numeric_limits<double>::max();
        QVector<double> logProbs(k);
        for (int j = 0; j < k; ++j) {
            if (!m_components[j].active) { logProbs[j] = -1e30; continue; }
            logProbs[j] = qLn(qMax(1e-300, m_components[j].weight)) + logGaussian(m_data[i], m_components[j]);
            maxLog = qMax(maxLog, logProbs[j]);
        }
        double sumExp = 0.0;
        for (int j = 0; j < k; ++j) {
            logProbs[j] = qExp(logProbs[j] - maxLog);
            sumExp += logProbs[j];
        }
        for (int j = 0; j < k; ++j)
            m_resp[i][j] = (sumExp > 1e-300) ? logProbs[j] / sumExp : 0.0;
    }
}

/* ---- M-step ---- */

void GaussianMixture23::mStep()
{
    int n = m_data.size();
    int d = m_data[0].size();
    int k = m_components.size();
    double alphaWeight = m_alpha0 + n;

    for (int j = 0; j < k; ++j) {
        auto& c = m_components[j];
        if (!c.active) continue;

        c.nk = 0.0;
        for (int i = 0; i < n; ++i) c.nk += m_resp[i][j];

        if (c.nk < 1e-6) { c.active = false; continue; }

        c.weight = (c.nk + m_alpha0 - 1.0) / alphaWeight;

        c.meanSum.fill(0.0);
        for (int i = 0; i < n; ++i)
            for (int dim = 0; dim < d; ++dim)
                c.meanSum[dim] += m_resp[i][j] * m_data[i][dim];
        for (int dim = 0; dim < d; ++dim)
            c.mean[dim] = c.meanSum[dim] / c.nk;

        for (int a = 0; a < d; ++a)
            for (int b = 0; b < d; ++b) {
                double s = 0.0;
                for (int i = 0; i < n; ++i)
                    s += m_resp[i][j] * (m_data[i][a] - c.mean[a]) * (m_data[i][b] - c.mean[b]);
                c.scatter[a][b] = s / c.nk;
            }

        for (int a = 0; a < d; ++a)
            for (int b = 0; b < d; ++b) {
                c.precision[a][b] = -c.scatter[a][b];
                if (a == b) c.precision[a][b] += 1e-3;
            }
    }
}

/* ---- Variational lower bound ---- */

double GaussianMixture23::computeLowerBound() const
{
    int n = m_data.size();
    int k = m_components.size();
    double lb = 0.0;
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < k; ++j) {
            if (!m_components[j].active || m_resp[i][j] < 1e-300) continue;
            double lg = logGaussian(m_data[i], m_components[j]);
            lb += m_resp[i][j] * (qLn(qMax(1e-300, m_components[j].weight)) + lg - qLn(qMax(1e-300, m_resp[i][j])));
        }
    return lb;
}

/* ---- Prune inactive ---- */

void GaussianMixture23::pruneComponents()
{
    int k = m_components.size();
    int n = m_data.size();
    for (int j = 0; j < k; ++j) {
        if (!m_components[j].active) {
            for (int i = 0; i < n; ++i) m_resp[i][j] = 0.0;
        }
    }
}

/* ---- Fit ---- */

bool GaussianMixture23::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n < m_maxK) return false;
    int d = data[0].size();
    m_data = data;

    initialize();

    double prevLB = -std::numeric_limits<double>::max();
    for (int iter = 0; iter < m_maxIter; ++iter) {
        eStep();
        mStep();
        pruneComponents();

        double lb = computeLowerBound();
        m_stats.iterations = iter + 1;

        int active = 0;
        for (const auto& c : m_components) if (c.active) active++;

        emit iterationCompleted(iter + 1, lb, active);

        if (qAbs(lb - prevLB) < m_tol) break;
        prevLB = lb;
    }

    m_stats.numPoints = n;
    m_stats.numDimensions = d;
    m_stats.maxComponents = m_maxK;
    m_stats.activeComponents = 0;
    m_stats.lowerBound = prevLB;
    for (const auto& c : m_components) if (c.active) m_stats.activeComponents++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit fitCompleted(m_stats.activeComponents, prevLB, timer.elapsed());
    return true;
}

/* ---- Predict ---- */

int GaussianMixture23::predict(const QVector<double>& point) const
{
    if (m_components.isEmpty()) return -1;
    int bestC = 0;
    double bestProb = -std::numeric_limits<double>::max();
    for (int j = 0; j < m_components.size(); ++j) {
        if (!m_components[j].active) continue;
        double lp = qLn(qMax(1e-300, m_components[j].weight)) + logGaussian(point, m_components[j]);
        if (lp > bestProb) { bestProb = lp; bestC = j; }
    }
    return bestC;
}

/* ---- Accessors ---- */

QVector<QVector<double>> GaussianMixture23::responsibilities() const { return m_resp; }
QVector<GaussianMixture23::Component> GaussianMixture23::components() const { return m_components; }

/* ---- Reset ---- */

void GaussianMixture23::resetStatistics()
{
    m_data.clear(); m_resp.clear(); m_components.clear();
    m_stats = Stats{}; m_timeSum = 0.0;
}

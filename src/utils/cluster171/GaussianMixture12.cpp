/**
 * @file GaussianMixture12.cpp
 * @brief GaussianMixture12 实现
 *
 * 实现GMM聚类：EM算法、对角/满协方差切换、AIC/BIC模型选择。
 */

#include "utils/cluster171/GaussianMixture12.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

GaussianMixture12::GaussianMixture12(QObject *parent)
    : QObject(parent)
{
}

GaussianMixture12::~GaussianMixture12() = default;

/* ---- Configuration ---- */

void GaussianMixture12::setMaxComponents(int k) { m_maxK = qMax(1, k); }
void GaussianMixture12::setCovarianceType(CovarianceType t) { m_covType = t; }
void GaussianMixture12::setMaxIterations(int iter) { m_maxIter = qMax(10, iter); }
void GaussianMixture12::setTolerance(double tol) { m_tol = qMax(1e-10, tol); }

/* ---- K-Means++ initialization ---- */

void GaussianMixture12::initComponents(const QVector<QVector<double>>& data, int k)
{
    int n = data.size();
    int d = data[0].size();
    m_components.resize(k);

    /* Pick first center randomly */
    int idx = 0;
    m_components[0].mean = data[idx];
    m_components[0].weight = 1.0 / k;

    QVector<double> minDist(n, 1e30);
    for (int c = 1; c < k; ++c) {
        /* Update distances */
        double totalDist = 0.0;
        for (int i = 0; i < n; ++i) {
            double dist = 0.0;
            for (int j = 0; j < d; ++j)
                dist += qPow(data[i][j] - m_components[c - 1].mean[j], 2);
            minDist[i] = qMin(minDist[i], dist);
            totalDist += minDist[i];
        }
        /* Weighted random selection */
        double r = totalDist * (static_cast<double>(qrand()) / RAND_MAX);
        double cumSum = 0.0;
        int chosen = 0;
        for (int i = 0; i < n; ++i) {
            cumSum += minDist[i];
            if (cumSum >= r) { chosen = i; break; }
        }
        m_components[c].mean = data[chosen];
        m_components[c].weight = 1.0 / k;
    }

    /* Initialize covariances to identity-scaled */
    for (int c = 0; c < k; ++c) {
        m_components[c].diagVar = QVector<double>(d, 1.0);
        if (m_covType == Full) {
            m_components[c].cov = QVector<QVector<double>>(d, QVector<double>(d, 0.0));
            m_components[c].covInv = QVector<QVector<double>>(d, QVector<double>(d, 0.0));
            for (int j = 0; j < d; ++j)
                m_components[c].cov[j][j] = m_components[c].covInv[j][j] = 1.0;
        }
        m_components[c].logDet = 0.0;
    }
}

/* ---- Update full covariance inverse (diagonal-dominant approximation) ---- */

void GaussianMixture12::updateInverse(int comp, int d)
{
    auto& inv = m_components[comp].covInv;
    auto& cov = m_components[comp].cov;

    /* Gauss-Jordan elimination for small matrices */
    inv = cov;
    double det = 1.0;
    for (int i = 0; i < d; ++i) {
        double pivot = inv[i][i];
        if (qAbs(pivot) < 1e-12) pivot = 1e-12;
        det *= pivot;
        for (int j = 0; j < d; ++j)
            inv[i][j] /= pivot;
        inv[i][i] = 1.0;
        for (int k = 0; k < d; ++k) {
            if (k == i) continue;
            double factor = inv[k][i];
            for (int j = 0; j < d; ++j)
                inv[k][j] -= factor * inv[i][j];
            inv[k][i] = 0.0;
        }
    }
    m_components[comp].logDet = qLn(qAbs(det) + 1e-30);
}

/* ---- Log probability density ---- */

double GaussianMixture12::logPdf(int comp, const QVector<double>& x) const
{
    int d = x.size();
    const auto& c = m_components[comp];
    double mahal = 0.0;

    if (m_covType == Diagonal) {
        for (int j = 0; j < d; ++j) {
            double diff = x[j] - c.mean[j];
            double v = qMax(c.diagVar[j], 1e-12);
            mahal += diff * diff / v;
        }
        double logDetD = 0.0;
        for (int j = 0; j < d; ++j)
            logDetD += qLn(qMax(c.diagVar[j], 1e-12));
        return -0.5 * (d * qLn(2.0 * M_PI) + logDetD + mahal);
    } else {
        /* Full covariance */
        for (int i = 0; i < d; ++i) {
            double sum = 0.0;
            for (int j = 0; j < d; ++j) {
                double diff = x[j] - c.mean[j];
                sum += c.covInv[i][j] * diff;
            }
            double diff_i = x[i] - c.mean[i];
            mahal += diff_i * sum;
        }
        return -0.5 * (d * qLn(2.0 * M_PI) + c.logDet + mahal);
    }
}

/* ---- E-step: compute responsibilities ---- */

void GaussianMixture12::eStep(const QVector<QVector<double>>& data,
                               QVector<QVector<double>>& resp)
{
    int n = data.size();
    int k = m_components.size();
    resp.resize(n);

    for (int i = 0; i < n; ++i) {
        resp[i].resize(k);
        double maxLog = -1e30;
        for (int c = 0; c < k; ++c) {
            resp[i][c] = qLn(qMax(m_components[c].weight, 1e-30)) + logPdf(c, data[i]);
            if (resp[i][c] > maxLog) maxLog = resp[i][c];
        }
        /* Log-sum-exp for numerical stability */
        double sum = 0.0;
        for (int c = 0; c < k; ++c) {
            resp[i][c] = qExp(resp[i][c] - maxLog);
            sum += resp[i][c];
        }
        for (int c = 0; c < k; ++c)
            resp[i][c] /= qMax(sum, 1e-30);
    }
}

/* ---- M-step: update parameters ---- */

void GaussianMixture12::mStep(const QVector<QVector<double>>& data,
                               const QVector<QVector<double>>& resp)
{
    int n = data.size();
    int d = data[0].size();
    int k = m_components.size();

    for (int c = 0; c < k; ++c) {
        double nk = 0.0;
        for (int i = 0; i < n; ++i) nk += resp[i][c];
        nk = qMax(nk, 1e-10);
        m_components[c].weight = nk / n;

        /* Update mean */
        for (int j = 0; j < d; ++j) {
            double sum = 0.0;
            for (int i = 0; i < n; ++i)
                sum += resp[i][c] * data[i][j];
            m_components[c].mean[j] = sum / nk;
        }

        if (m_covType == Diagonal) {
            /* Update diagonal variance */
            for (int j = 0; j < d; ++j) {
                double sum = 0.0;
                for (int i = 0; i < n; ++i) {
                    double diff = data[i][j] - m_components[c].mean[j];
                    sum += resp[i][c] * diff * diff;
                }
                m_components[c].diagVar[j] = qMax(sum / nk, 1e-6);
            }
        } else {
            /* Update full covariance */
            auto& cov = m_components[c].cov;
            for (int a = 0; a < d; ++a) {
                for (int b = 0; b < d; ++b) {
                    double sum = 0.0;
                    for (int i = 0; i < n; ++i) {
                        double da = data[i][a] - m_components[c].mean[a];
                        double db = data[i][b] - m_components[c].mean[b];
                        sum += resp[i][c] * da * db;
                    }
                    cov[a][b] = sum / nk;
                }
            }
            /* Regularize diagonal */
            for (int j = 0; j < d; ++j)
                cov[j][j] = qMax(cov[j][j], 1e-6);
            updateInverse(c, d);
        }
    }
}

/* ---- Compute log-likelihood ---- */

double GaussianMixture12::logLikelihood(const QVector<QVector<double>>& data) const
{
    int n = data.size();
    int k = m_components.size();
    double ll = 0.0;
    for (int i = 0; i < n; ++i) {
        double sum = 0.0;
        for (int c = 0; c < k; ++c)
            sum += m_components[c].weight * qExp(logPdf(c, data[i]));
        ll += qLn(qMax(sum, 1e-30));
    }
    return ll;
}

/* ---- AIC / BIC ---- */

double GaussianMixture12::computeAIC(double ll, int k, int d) const
{
    int p;
    if (m_covType == Diagonal)
        p = k * (1 + d + d) - 1;
    else
        p = k * (1 + d + d * (d + 1) / 2) - 1;
    return -2.0 * ll + 2.0 * p;
}

double GaussianMixture12::computeBIC(double ll, int k, int d, int n) const
{
    int p;
    if (m_covType == Diagonal)
        p = k * (1 + d + d) - 1;
    else
        p = k * (1 + d + d * (d + 1) / 2) - 1;
    return -2.0 * ll + qLn(static_cast<double>(n)) * p;
}

/* ---- Main fit ---- */

QVector<int> GaussianMixture12::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n == 0) return {};
    int d = data[0].size();
    if (d == 0) return {};

    initComponents(data, m_maxK);

    double prevLL = -1e30;
    QVector<QVector<double>> resp;

    for (int iter = 0; iter < m_maxIter; ++iter) {
        eStep(data, resp);
        mStep(data, resp);
        double ll = logLikelihood(data);
        if (qAbs(ll - prevLL) < m_tol * qAbs(prevLL)) break;
        prevLL = ll;
        m_stats.lastLogLikelihood = ll;
    }

    /* Assign labels */
    m_labels.resize(n);
    for (int i = 0; i < n; ++i) {
        int best = 0;
        double bestR = resp[i][0];
        for (int c = 1; c < m_maxK; ++c) {
            if (resp[i][c] > bestR) { bestR = resp[i][c]; best = c; }
        }
        m_labels[i] = best;
    }

    m_stats.lastComponents = m_maxK;
    m_stats.lastAic = computeAIC(m_stats.lastLogLikelihood, m_maxK, d);
    m_stats.lastBic = computeBIC(m_stats.lastLogLikelihood, m_maxK, d, n);
    m_stats.totalRuns++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalRuns;

    emit fittingCompleted(m_maxK, m_stats.lastLogLikelihood);
    return m_labels;
}

/* ---- Auto model selection ---- */

QVector<int> GaussianMixture12::fitAuto(const QVector<QVector<double>>& data, int maxK)
{
    int n = data.size();
    if (n == 0) return {};
    int d = data[0].size();

    int bestK = 1;
    double bestBic = 1e30;
    QVector<int> bestLabels(n, 0);

    int savedK = m_maxK;
    for (int k = 1; k <= qMin(maxK, n); ++k) {
        m_maxK = k;
        QVector<int> labels = fit(data);
        if (m_stats.lastBic < bestBic) {
            bestBic = m_stats.lastBic;
            bestK = k;
            bestLabels = labels;
        }
    }
    m_maxK = savedK;

    emit modelSelected(bestK, m_stats.lastAic, bestBic);
    return bestLabels;
}

/* ---- Accessors ---- */

QVector<double> GaussianMixture12::weights() const
{
    QVector<double> w;
    for (const auto& c : m_components) w.append(c.weight);
    return w;
}

QVector<QVector<double>> GaussianMixture12::means() const
{
    QVector<QVector<double>> m;
    for (const auto& c : m_components) m.append(c.mean);
    return m;
}

/* ---- Statistics ---- */

void GaussianMixture12::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

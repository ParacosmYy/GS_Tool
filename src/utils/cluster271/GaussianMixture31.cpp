/**
 * @file GaussianMixture31.cpp
 * @brief GaussianMixture31 实现
 *
 * 实现高斯混合模型：期望条件最大化与充分统计量缓存增量更新。
 */

#include "utils/cluster271/GaussianMixture31.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

GaussianMixture31::GaussianMixture31(QObject *parent)
    : QObject(parent) {}

GaussianMixture31::~GaussianMixture31() = default;

/* ---- Configuration ---- */

void GaussianMixture31::setComponents(int k) { m_k = qBound(1, k, 100); }
void GaussianMixture31::setMaxIterations(int iters) { m_maxIter = qBound(1, iters, 10000); }
void GaussianMixture31::setTolerance(double tol) { m_tol = qBound(1e-12, tol, 1.0); }

/* ---- K-means++ seeding for initialization ---- */

void GaussianMixture31::initParams(const QVector<QVector<double>>& data)
{
    int n = data.size();
    int d = data[0].size();
    m_weights.fill(1.0 / m_k, m_k);
    m_means.resize(m_k);
    m_covs.resize(m_k);

    // Pick first center randomly
    int first = 0;
    m_means[0] = data[first];

    QVector<double> dist(n, 1e18);
    for (int c = 1; c < m_k; ++c) {
        // Update distances
        for (int i = 0; i < n; ++i) {
            double dx = data[i][0] - m_means[c - 1][0];
            double dy = data[i][1] - m_means[c - 1][1];
            dist[i] = qMin(dist[i], dx * dx + dy * dy);
        }
        // Weighted selection
        double total = 0.0;
        for (double d2 : dist) total += d2;
        double r = total * qrand() / static_cast<double>(RAND_MAX);
        double cumSum = 0.0;
        int chosen = n - 1;
        for (int i = 0; i < n; ++i) {
            cumSum += dist[i];
            if (cumSum >= r) { chosen = i; break; }
        }
        m_means[c] = data[chosen];
    }

    // Initialize covariances to identity scaled
    for (int k = 0; k < m_k; ++k) {
        m_covs[k].resize(d);
        for (int i = 0; i < d; ++i) {
            m_covs[k][i].fill(0.0, d);
            m_covs[k][i][i] = 1.0;
        }
    }
}

/* ---- Gaussian PDF evaluation ---- */

double GaussianMixture31::gaussianPdf(const QVector<double>& x, int k) const
{
    int d = x.size();
    double det = 1.0;
    for (int i = 0; i < d; ++i) det *= m_covs[k][i][i];
    if (det <= 0.0) return 1e-300;

    double exponent = 0.0;
    for (int i = 0; i < d; ++i) {
        double diff = x[i] - m_means[k][i];
        exponent += diff * diff / m_covs[k][i][i];
    }
    exponent *= -0.5;

    double norm = 1.0 / (2.0 * M_PI * qSqrt(det));
    return norm * qExp(exponent);
}

/* ---- E-step: compute responsibilities ---- */

QVector<QVector<double>> GaussianMixture31::eStep(const QVector<QVector<double>>& data) const
{
    int n = data.size();
    QVector<QVector<double>> resp(n, QVector<double>(m_k, 0.0));

    for (int i = 0; i < n; ++i) {
        double sum = 0.0;
        for (int k = 0; k < m_k; ++k) {
            resp[i][k] = m_weights[k] * gaussianPdf(data[i], k);
            sum += resp[i][k];
        }
        if (sum > 0.0) {
            for (int k = 0; k < m_k; ++k) resp[i][k] /= sum;
        }
    }
    return resp;
}

/* ---- CM-step: update weights and means ---- */

void GaussianMixture31::cmStepWeightsMeans(const QVector<QVector<double>>& data,
                                            const QVector<QVector<double>>& resp)
{
    int n = data.size();
    int d = data[0].size();

    for (int k = 0; k < m_k; ++k) {
        double nk = 0.0;
        for (int i = 0; i < n; ++i) nk += resp[i][k];
        m_weights[k] = nk / n;

        for (int j = 0; j < d; ++j) {
            double sum = 0.0;
            for (int i = 0; i < n; ++i) sum += resp[i][k] * data[i][j];
            m_means[k][j] = (nk > 0.0) ? sum / nk : 0.0;
        }
    }
}

/* ---- CM-step: update covariances ---- */

void GaussianMixture31::cmStepCovariances(const QVector<QVector<double>>& data,
                                           const QVector<QVector<double>>& resp)
{
    int n = data.size();
    int d = data[0].size();

    for (int k = 0; k < m_k; ++k) {
        double nk = 0.0;
        for (int i = 0; i < n; ++i) nk += resp[i][k];
        if (nk <= 0.0) continue;

        for (int r = 0; r < d; ++r) {
            for (int c = 0; c < d; ++c) {
                double sum = 0.0;
                for (int i = 0; i < n; ++i) {
                    double dr = data[i][r] - m_means[k][r];
                    double dc = data[i][c] - m_means[k][c];
                    sum += resp[i][k] * dr * dc;
                }
                m_covs[k][r][c] = sum / nk;
            }
            // Regularize diagonal to prevent singularity
            m_covs[k][r][r] = qMax(m_covs[k][r][r], 1e-6);
        }
    }
}

/* ---- Log-likelihood computation ---- */

double GaussianMixture31::computeLogLikelihood(const QVector<QVector<double>>& data) const
{
    double ll = 0.0;
    for (int i = 0; i < data.size(); ++i) {
        double sum = 0.0;
        for (int k = 0; k < m_k; ++k) sum += m_weights[k] * gaussianPdf(data[i], k);
        ll += qLn(qMax(sum, 1e-300));
    }
    return ll;
}

/* ---- Accumulate sufficient statistics ---- */

void GaussianMixture31::accumulateSS(const QVector<QVector<double>>& data,
                                      const QVector<QVector<double>>& resp)
{
    int d = data[0].size();
    m_ssN.resize(m_k);
    m_ssSum.resize(m_k);
    m_ssOuter.resize(m_k);

    for (int k = 0; k < m_k; ++k) {
        double nk = 0.0;
        QVector<double> sumX(d, 0.0);
        QVector<QVector<double>> outer(d, QVector<double>(d, 0.0));

        for (int i = 0; i < data.size(); ++i) {
            double r = resp[i][k];
            nk += r;
            for (int j = 0; j < d; ++j) {
                sumX[j] += r * data[i][j];
                for (int l = 0; l < d; ++l)
                    outer[j][l] += r * data[i][j] * data[i][l];
            }
        }
        m_ssN[k] += nk;
        for (int j = 0; j < d; ++j) m_ssSum[k][j] += sumX[j];
        for (int j = 0; j < d; ++j)
            for (int l = 0; l < d; ++l)
                m_ssOuter[k][j][l] += outer[j][l];
    }
}

/* ---- Main fit ---- */

QVector<QVector<double>> GaussianMixture31::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n == 0) return {};

    initParams(data);
    m_totalSamples = n;

    double prevLL = -1e30;
    int iter = 0;
    for (; iter < m_maxIter; ++iter) {
        QVector<QVector<double>> resp = eStep(data);
        cmStepWeightsMeans(data, resp);
        cmStepCovariances(data, resp);

        double ll = computeLogLikelihood(data);
        if (qAbs(ll - prevLL) < m_tol) break;
        prevLL = ll;
    }

    // Cache sufficient statistics for incremental updates
    QVector<QVector<double>> resp = eStep(data);
    accumulateSS(data, resp);

    double elapsed = timer.elapsed();
    m_stats.numComponents = m_k;
    m_stats.numSamples = n;
    m_stats.numIterations = iter;
    m_stats.logLikelihood = prevLL;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit fittingDone(iter, prevLL, elapsed);

    return resp;
}

/* ---- Incremental update ---- */

void GaussianMixture31::updateIncremental(const QVector<QVector<double>>& newData)
{
    QElapsedTimer timer;
    timer.start();
    if (newData.isEmpty()) return;

    QVector<QVector<double>> resp = eStep(newData);
    accumulateSS(newData, resp);
    m_totalSamples += newData.size();

    // Re-derive parameters from accumulated SS
    int d = newData[0].size();
    for (int k = 0; k < m_k; ++k) {
        double nk = m_ssN[k];
        if (nk <= 0.0) continue;
        m_weights[k] = nk / m_totalSamples;
        for (int j = 0; j < d; ++j)
            m_means[k][j] = m_ssSum[k][j] / nk;
        for (int r = 0; r < d; ++r) {
            for (int c = 0; c < d; ++c)
                m_covs[k][r][c] = m_ssOuter[k][r][c] / nk
                    - m_means[k][r] * m_means[k][c];
            m_covs[k][r][r] = qMax(m_covs[k][r][r], 1e-6);
        }
    }

    double elapsed = timer.elapsed();
    m_stats.numSamples = m_totalSamples;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
}

/* ---- Predict ---- */

QVector<double> GaussianMixture31::predict(const QVector<double>& point) const
{
    QVector<double> probs(m_k, 0.0);
    double sum = 0.0;
    for (int k = 0; k < m_k; ++k) {
        probs[k] = m_weights[k] * gaussianPdf(point, k);
        sum += probs[k];
    }
    if (sum > 0.0)
        for (int k = 0; k < m_k; ++k) probs[k] /= sum;
    return probs;
}

/* ---- Accessors ---- */

QVector<QVector<double>> GaussianMixture31::means() const { return m_means; }
QVector<double> GaussianMixture31::weights() const { return m_weights; }
double GaussianMixture31::logLikelihood() const { return m_stats.logLikelihood; }

/* ---- Reset ---- */

void GaussianMixture31::resetStatistics()
{
    m_weights.clear(); m_means.clear(); m_covs.clear();
    m_ssN.clear(); m_ssSum.clear(); m_ssOuter.clear();
    m_totalSamples = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}

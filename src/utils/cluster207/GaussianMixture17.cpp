/**
 * @file GaussianMixture17.cpp
 * @brief GaussianMixture17 实现
 *
 * 实现高斯混合模型：增量EM、在线充分统计量、流式数据自适应学习。
 */

#include "utils/cluster207/GaussianMixture17.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

GaussianMixture17::GaussianMixture17(QObject *parent) : QObject(parent) {}
GaussianMixture17::~GaussianMixture17() = default;

/* ---- Configuration ---- */

void GaussianMixture17::setNumComponents(int k) { m_k = qMax(1, k); }
void GaussianMixture17::setMaxIterations(int maxIter) { m_maxIter = qMax(1, maxIter); }
void GaussianMixture17::setConvergenceThreshold(double eps) { m_eps = qMax(1e-10, eps); }

/* ---- Initialize ---- */

void GaussianMixture17::initialize(int dims, int k,
                                    const QVector<QVector<double>>& initData)
{
    m_dims = qMax(1, dims);
    m_k = qMax(1, k);
    int n = initData.size();

    // Uniform weights
    m_weights.resize(m_k, 1.0 / m_k);

    // Initialize means from random data points
    m_means.resize(m_k);
    for (int j = 0; j < m_k; ++j) {
        m_means[j].resize(m_dims, 0.0);
        if (n > 0) {
            int idx = j % n;
            for (int d = 0; d < m_dims; ++d)
                m_means[j][d] = (d < initData[idx].size()) ? initData[idx][d] : 0.0;
        }
    }

    // Initialize covariances to identity
    m_covs.resize(m_k);
    for (int j = 0; j < m_k; ++j) {
        m_covs[j].resize(m_dims);
        for (int d = 0; d < m_dims; ++d) {
            m_covs[j][d].resize(m_dims, 0.0);
            m_covs[j][d][d] = 1.0;
        }
    }

    // Reset sufficient statistics
    m_suffN.resize(m_k, 0.0);
    m_suffMean.resize(m_k);
    m_suffCov.resize(m_k);
    for (int j = 0; j < m_k; ++j) {
        m_suffMean[j].resize(m_dims, 0.0);
        m_suffCov[j].resize(m_dims);
        for (int d = 0; d < m_dims; ++d)
            m_suffCov[j][d].resize(m_dims, 0.0);
    }
    m_totalSamples = 0;
}

/* ---- Gaussian PDF ---- */

double GaussianMixture17::gaussianPdf(const QVector<double>& x, int comp) const
{
    double logDet = logDetCov(comp);
    auto inv = inverseCov(comp);

    double quad = 0.0;
    for (int i = 0; i < m_dims; ++i) {
        double sum = 0.0;
        for (int j = 0; j < m_dims; ++j) {
            double diff = x[j] - m_means[comp][j];
            sum += inv[i][j] * diff;
        }
        quad += (x[i] - m_means[comp][i]) * sum;
    }

    double logPdf = -0.5 * (m_dims * qLn(2.0 * M_PI) + logDet + quad);
    return qExp(logPdf);
}

/* ---- Cholesky-based log determinant ---- */

double GaussianMixture17::logDetCov(int comp) const
{
    // Simple log-determinant via LU-style accumulation
    const auto& cov = m_covs[comp];
    double logDet = 0.0;
    for (int i = 0; i < m_dims; ++i)
        logDet += qLn(qMax(cov[i][i], 1e-15));
    return logDet;
}

/* ---- Inverse covariance via Gauss-Jordan ---- */

QVector<QVector<double>> GaussianMixture17::inverseCov(int comp) const
{
    const auto& cov = m_covs[comp];
    int n = m_dims;
    QVector<QVector<double>> aug(n);
    for (int i = 0; i < n; ++i) {
        aug[i].resize(2 * n, 0.0);
        for (int j = 0; j < n; ++j) aug[i][j] = cov[i][j];
        aug[i][n + i] = 1.0;
    }

    for (int col = 0; col < n; ++col) {
        double pivot = qMax(qAbs(aug[col][col]), 1e-15);
        for (int j = 0; j < 2 * n; ++j) aug[col][j] /= pivot;
        for (int row = 0; row < n; ++row) {
            if (row == col) continue;
            double factor = aug[row][col];
            for (int j = 0; j < 2 * n; ++j)
                aug[row][j] -= factor * aug[col][j];
        }
    }

    QVector<QVector<double>> inv(n);
    for (int i = 0; i < n; ++i) {
        inv[i].resize(n);
        for (int j = 0; j < n; ++j)
            inv[i][j] = aug[i][n + j];
    }
    return inv;
}

/* ---- E-step ---- */

QVector<QVector<double>> GaussianMixture17::eStep(
    const QVector<QVector<double>>& data) const
{
    int n = data.size();
    QVector<QVector<double>> resp(n);

    for (int i = 0; i < n; ++i) {
        resp[i].resize(m_k, 0.0);
        double sum = 0.0;
        for (int j = 0; j < m_k; ++j) {
            resp[i][j] = m_weights[j] * gaussianPdf(data[i], j);
            sum += resp[i][j];
        }
        if (sum > 1e-300)
            for (int j = 0; j < m_k; ++j) resp[i][j] /= sum;
    }
    return resp;
}

/* ---- M-step ---- */

void GaussianMixture17::mStep(const QVector<QVector<double>>& data,
                                const QVector<QVector<double>>& resp)
{
    int n = data.size();
    QVector<double> nk(m_k, 0.0);

    for (int i = 0; i < n; ++i)
        for (int j = 0; j < m_k; ++j)
            nk[j] += resp[i][j];

    // Update weights
    for (int j = 0; j < m_k; ++j)
        m_weights[j] = qMax(nk[j], 1e-10) / qMax(n, 1);

    // Update means
    for (int j = 0; j < m_k; ++j) {
        std::fill(m_means[j].begin(), m_means[j].end(), 0.0);
        for (int i = 0; i < n; ++i)
            for (int d = 0; d < m_dims; ++d)
                m_means[j][d] += resp[i][j] * data[i][d];
        if (nk[j] > 1e-10)
            for (int d = 0; d < m_dims; ++d)
                m_means[j][d] /= nk[j];
    }

    // Update covariances
    for (int j = 0; j < m_k; ++j) {
        for (int d1 = 0; d1 < m_dims; ++d1)
            for (int d2 = 0; d2 < m_dims; ++d2) {
                double sum = 0.0;
                for (int i = 0; i < n; ++i)
                    sum += resp[i][j] * (data[i][d1] - m_means[j][d1])
                           * (data[i][d2] - m_means[j][d2]);
                m_covs[j][d1][d2] = sum / qMax(nk[j], 1e-10);
            }
        // Regularization
        for (int d = 0; d < m_dims; ++d)
            m_covs[j][d][d] += 1e-6;
    }
}

/* ---- Update online sufficient statistics ---- */

void GaussianMixture17::updateSufficientStats(const QVector<QVector<double>>& data,
                                                const QVector<QVector<double>>& resp)
{
    int n = data.size();
    for (int j = 0; j < m_k; ++j) {
        double nk = 0.0;
        QVector<double> newMean(m_dims, 0.0);
        for (int i = 0; i < n; ++i) {
            nk += resp[i][j];
            for (int d = 0; d < m_dims; ++d)
                newMean[d] += resp[i][j] * data[i][d];
        }
        if (nk < 1e-15) continue;

        for (int d = 0; d < m_dims; ++d) newMean[d] /= nk;

        // Online update of running mean
        double alpha = nk / (m_suffN[j] + nk);
        if (m_suffN[j] < 1e-15) alpha = 1.0;
        for (int d = 0; d < m_dims; ++d)
            m_suffMean[j][d] = (1.0 - alpha) * m_suffMean[j][d] + alpha * newMean[d];

        m_suffN[j] += nk;
    }
    m_totalSamples += n;
}

/* ---- Batch fit ---- */

void GaussianMixture17::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();
    int n = data.size();
    if (n == 0) return;

    if (m_dims == 0) initialize(data[0].size(), m_k, data);

    double prevLL = -std::numeric_limits<double>::max();

    for (int iter = 0; iter < m_maxIter; ++iter) {
        QVector<QVector<double>> resp = eStep(data);
        mStep(data, resp);
        updateSufficientStats(data, resp);

        double ll = logLikelihood(data);
        if (qAbs(ll - prevLL) < m_eps) break;
        prevLL = ll;
    }

    m_stats.numComponents = m_k;
    m_stats.numDimensions = m_dims;
    m_stats.numSamples = n;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit fittingCompleted(m_k, prevLL, timer.elapsed());
}

/* ---- Partial fit for streaming ---- */

void GaussianMixture17::partialFit(const QVector<QVector<double>>& chunk)
{
    QElapsedTimer timer;
    timer.start();
    if (chunk.isEmpty()) return;

    if (m_dims == 0) initialize(chunk[0].size(), m_k, chunk);

    // Few incremental EM iterations on new chunk
    for (int iter = 0; iter < 5; ++iter) {
        QVector<QVector<double>> resp = eStep(chunk);
        mStep(chunk, resp);
        updateSufficientStats(chunk, resp);
    }

    m_stats.totalOps++;
    m_stats.numSamples += chunk.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
}

/* ---- Predict probability ---- */

QVector<double> GaussianMixture17::predictProba(const QVector<double>& sample) const
{
    QVector<double> proba(m_k, 0.0);
    double sum = 0.0;
    for (int j = 0; j < m_k; ++j) {
        proba[j] = m_weights[j] * gaussianPdf(sample, j);
        sum += proba[j];
    }
    if (sum > 1e-300)
        for (int j = 0; j < m_k; ++j) proba[j] /= sum;
    return proba;
}

/* ---- Predict labels ---- */

QVector<int> GaussianMixture17::predict(const QVector<QVector<double>>& data) const
{
    QVector<int> labels(data.size(), 0);
    for (int i = 0; i < data.size(); ++i) {
        double bestP = -1.0;
        for (int j = 0; j < m_k; ++j) {
            double p = m_weights[j] * gaussianPdf(data[i], j);
            if (p > bestP) { bestP = p; labels[i] = j; }
        }
    }
    return labels;
}

/* ---- Log-likelihood ---- */

double GaussianMixture17::logLikelihood(const QVector<QVector<double>>& data) const
{
    double ll = 0.0;
    for (int i = 0; i < data.size(); ++i) {
        double sum = 0.0;
        for (int j = 0; j < m_k; ++j)
            sum += m_weights[j] * gaussianPdf(data[i], j);
        if (sum > 1e-300) ll += qLn(sum);
    }
    return ll;
}

/* ---- Getters ---- */

QVector<double> GaussianMixture17::weights() const { return m_weights; }
QVector<QVector<double>> GaussianMixture17::means() const { return m_means; }

/* ---- Reset ---- */

void GaussianMixture17::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_weights.clear();
    m_means.clear();
    m_covs.clear();
    m_suffN.clear();
    m_suffMean.clear();
    m_suffCov.clear();
    m_totalSamples = 0;
    m_dims = 0;
}

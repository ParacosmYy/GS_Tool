/**
 * @file GaussianMixture15.cpp
 * @brief GaussianMixture15 实现
 *
 * 实现高斯混合模型：增量EM算法、充分统计遗忘因子、在线模型更新。
 */

#include "utils/cluster192/GaussianMixture15.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

GaussianMixture15::GaussianMixture15(QObject *parent) : QObject(parent) {}
GaussianMixture15::~GaussianMixture15() = default;

/* ---- Configuration ---- */

void GaussianMixture15::setComponents(int k) { m_k = qMax(1, k); }
void GaussianMixture15::setForgettingFactor(double a) { m_alpha = qBound(0.5, a, 1.0); }
void GaussianMixture15::setMaxIterations(int iter) { m_maxIter = qMax(1, iter); }
void GaussianMixture15::setConvergenceThreshold(double e) { m_eps = qMax(1e-12, e); }

/* ---- Multivariate Gaussian log-pdf ---- */

double GaussianMixture15::logDet(const QVector<QVector<double>>& mat) const
{
    int n = mat.size();
    QVector<QVector<double>> L(n, QVector<double>(n, 0.0));
    double ld = 0.0;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j <= i; ++j) {
            double s = mat[i][j];
            for (int k = 0; k < j; ++k) s -= L[i][k] * L[j][k];
            if (i == j) {
                if (s <= 1e-15) s = 1e-15;
                L[i][j] = qSqrt(s);
                ld += qLn(s);
            } else {
                L[i][j] = s / qMax(L[j][j], 1e-15);
            }
        }
    }
    return ld;
}

QVector<double> GaussianMixture15::choleskySolve(
    const QVector<QVector<double>>& A, const QVector<double>& b) const
{
    int n = A.size();
    // Cholesky decomposition
    QVector<QVector<double>> L(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j <= i; ++j) {
            double s = A[i][j];
            for (int k = 0; k < j; ++k) s -= L[i][k] * L[j][k];
            if (i == j) L[i][j] = qSqrt(qMax(s, 1e-15));
            else L[i][j] = s / qMax(L[j][j], 1e-15);
        }
    }
    // Forward substitution
    QVector<double> y(n, 0.0);
    for (int i = 0; i < n; ++i) {
        double s = b[i];
        for (int k = 0; k < i; ++k) s -= L[i][k] * y[k];
        y[i] = s / qMax(L[i][i], 1e-15);
    }
    // Back substitution
    QVector<double> x(n, 0.0);
    for (int i = n - 1; i >= 0; --i) {
        double s = y[i];
        for (int k = i + 1; k < n; ++k) s -= L[k][i] * x[k];
        x[i] = s / qMax(L[i][i], 1e-15);
    }
    return x;
}

double GaussianMixture15::logGaussian(const QVector<double>& x, int comp) const
{
    int d = x.size();
    const auto& mu = m_means[comp];
    const auto& cov = m_covs[comp];

    // Compute diff
    QVector<double> diff(d);
    for (int i = 0; i < d; ++i) diff[i] = x[i] - mu[i];

    // Solve cov * z = diff
    QVector<double> z = choleskySolve(cov, diff);
    double maha = 0.0;
    for (int i = 0; i < d; ++i) maha += diff[i] * z[i];

    double ld = logDet(cov);
    return -0.5 * (d * qLn(2.0 * M_PI) + ld + maha);
}

/* ---- E-step ---- */

QVector<QVector<double>> GaussianMixture15::eStep(
    const QVector<QVector<double>>& data) const
{
    int n = data.size();
    int k = m_weights.size();
    QVector<QVector<double>> resp(n, QVector<double>(k, 0.0));

    for (int i = 0; i < n; ++i) {
        double maxLog = -std::numeric_limits<double>::max();
        for (int j = 0; j < k; ++j) {
            double lp = qLn(qMax(m_weights[j], 1e-300)) + logGaussian(data[i], j);
            resp[i][j] = lp;
            if (lp > maxLog) maxLog = lp;
        }
        // Log-sum-exp normalization
        double sum = 0.0;
        for (int j = 0; j < k; ++j) {
            resp[i][j] = qExp(resp[i][j] - maxLog);
            sum += resp[i][j];
        }
        if (sum > 1e-300)
            for (int j = 0; j < k; ++j) resp[i][j] /= sum;
    }
    return resp;
}

/* ---- M-step ---- */

void GaussianMixture15::mStep(const QVector<QVector<double>>& data,
                               const QVector<QVector<double>>& resp)
{
    int n = data.size();
    int k = m_weights.size();
    int d = m_dim;

    QVector<double> nk(k, 0.0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < k; ++j)
            nk[j] += resp[i][j];

    for (int j = 0; j < k; ++j) {
        // Weight
        m_weights[j] = qMax(nk[j], 1e-10) / qMax(n, 1);
        // Mean
        for (int dd = 0; dd < d; ++dd) {
            double s = 0.0;
            for (int i = 0; i < n; ++i) s += resp[i][j] * data[i][dd];
            m_means[j][dd] = s / qMax(nk[j], 1e-10);
        }
        // Covariance
        for (int a = 0; a < d; ++a) {
            for (int b = 0; b < d; ++b) {
                double s = 0.0;
                for (int i = 0; i < n; ++i) {
                    double da = data[i][a] - m_means[j][a];
                    double db = data[i][b] - m_means[j][b];
                    s += resp[i][j] * da * db;
                }
                m_covs[j][a][b] = s / qMax(nk[j], 1e-10) + (a == b ? 1e-6 : 0.0);
            }
        }
    }
}

/* ---- Batch fit ---- */

QVector<int> GaussianMixture15::fit(const QVector<QVector<double>>& data, int k)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n == 0 || k <= 0) return {};
    k = qMin(k, n);
    m_k = k;
    m_dim = data[0].size();

    // Initialize with random responsibilities
    m_weights.resize(k);
    m_means.resize(k);
    m_covs.resize(k);
    for (int j = 0; j < k; ++j) {
        m_weights[j] = 1.0 / k;
        m_means[j] = data[j * n / k];
        m_covs[j] = QVector<QVector<double>>(m_dim, QVector<double>(m_dim, 0.0));
        for (int d = 0; d < m_dim; ++d) m_covs[j][d][d] = 1.0;
    }

    // EM iterations
    double prevLL = -std::numeric_limits<double>::max();
    for (int it = 0; it < m_maxIter; ++it) {
        QVector<QVector<double>> resp = eStep(data);
        mStep(data, resp);
        double ll = logLikelihood(data);
        if (qAbs(ll - prevLL) < m_eps) break;
        prevLL = ll;
    }

    m_totalN = n;
    QVector<int> labels = predict(data);

    m_stats.totalUpdates++;
    m_stats.numComponents = k;
    m_stats.numDimensions = m_dim;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalUpdates;

    emit modelUpdated(k, prevLL, timer.elapsed());
    return labels;
}

/* ---- Incremental partial fit ---- */

void GaussianMixture15::partialFit(const QVector<QVector<double>>& batch)
{
    if (batch.isEmpty() || m_weights.isEmpty()) return;

    QElapsedTimer timer;
    timer.start();

    int bn = batch.size();
    double gamma = 1.0 - m_alpha;  // learning rate

    // E-step on new batch
    QVector<QVector<double>> resp = eStep(batch);

    // Update sufficient statistics with forgetting
    int k = m_weights.size();
    int d = m_dim;
    QVector<double> nk(k, 0.0);
    for (int i = 0; i < bn; ++i)
        for (int j = 0; j < k; ++j) nk[j] += resp[i][j];

    m_totalN = m_alpha * m_totalN + bn;
    for (int j = 0; j < k; ++j) {
        double oldNk = m_weights[j] * (m_totalN - bn + bn * gamma);
        double newNk = m_alpha * oldNk + nk[j];
        m_weights[j] = newNk / qMax(m_totalN, 1.0);

        // Update mean
        QVector<double> newMean(d, 0.0);
        for (int i = 0; i < bn; ++i)
            for (int dd = 0; dd < d; ++dd)
                newMean[dd] += resp[i][j] * batch[i][dd];
        for (int dd = 0; dd < d; ++dd) {
            newMean[dd] /= qMax(nk[j], 1e-10);
            m_means[j][dd] = m_alpha * m_means[j][dd] + gamma * newMean[dd];
        }

        // Update covariance (sufficient statistics)
        for (int a = 0; a < d; ++a) {
            for (int b = 0; b < d; ++b) {
                double s = 0.0;
                for (int i = 0; i < bn; ++i) {
                    double da = batch[i][a] - m_means[j][a];
                    double db = batch[i][b] - m_means[j][b];
                    s += resp[i][j] * da * db;
                }
                s /= qMax(nk[j], 1e-10);
                m_covs[j][a][b] = m_alpha * m_covs[j][a][b] + gamma * s
                                  + (a == b ? 1e-6 : 0.0);
            }
        }
    }

    m_stats.streamingBatches++;
    double ll = logLikelihood(batch);
    m_llSum += ll;
    m_stats.avgLogLikelihood = m_llSum / m_stats.streamingBatches;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.streamingBatches;

    emit modelUpdated(k, ll, timer.elapsed());
}

/* ---- Predict ---- */

QVector<int> GaussianMixture15::predict(const QVector<QVector<double>>& data) const
{
    QVector<QVector<double>> resp = eStep(data);
    QVector<int> labels(data.size(), 0);
    for (int i = 0; i < data.size(); ++i) {
        double best = -1.0;
        for (int j = 0; j < m_weights.size(); ++j) {
            if (resp[i][j] > best) { best = resp[i][j]; labels[i] = j; }
        }
    }
    return labels;
}

/* ---- Log-likelihood ---- */

double GaussianMixture15::logLikelihood(const QVector<QVector<double>>& data) const
{
    int n = data.size();
    int k = m_weights.size();
    double ll = 0.0;
    for (int i = 0; i < n; ++i) {
        double sum = 0.0;
        for (int j = 0; j < k; ++j)
            sum += m_weights[j] * qExp(logGaussian(data[i], j));
        ll += qLn(qMax(sum, 1e-300));
    }
    return ll;
}

/* ---- Accessors ---- */

QVector<double> GaussianMixture15::weights() const { return m_weights; }
QVector<QVector<double>> GaussianMixture15::means() const { return m_means; }

/* ---- Reset ---- */

void GaussianMixture15::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_llSum = 0.0;
    m_weights.clear();
    m_means.clear();
    m_covs.clear();
    m_totalN = 0.0;
}

/**
 * @file GaussianMixture14.cpp
 * @brief GaussianMixture14 实现
 *
 * 实现变分贝叶斯高斯混合模型：Dirichlet先验、自动剪枝、ELBO收敛。
 */

#include "utils/cluster185/GaussianMixture14.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>
#include <random>

/* ---- Construction / Destruction ---- */

GaussianMixture14::GaussianMixture14(QObject *parent) : QObject(parent) {}
GaussianMixture14::~GaussianMixture14() = default;

/* ---- Configuration ---- */

void GaussianMixture14::setMaxComponents(int k) { m_maxComponents = qMax(2, k); }
void GaussianMixture14::setDirichletAlpha(double a) { m_dirichletAlpha = qMax(0.01, a); }
void GaussianMixture14::setMaxIterations(int iter) { m_maxIterations = qMax(10, iter); }
void GaussianMixture14::setTolerance(double tol) { m_tolerance = qMax(1e-10, tol); }

/* ---- Log-sum-exp ---- */

double GaussianMixture14::logSumExp(const QVector<double>& vals) const
{
    if (vals.isEmpty()) return -std::numeric_limits<double>::infinity();
    double maxV = *std::max_element(vals.begin(), vals.end());
    double sum = 0.0;
    for (double v : vals)
        sum += qExp(v - maxV);
    return maxV + qLn(sum);
}

/* ---- Determinant (2x2 or larger via cofactor) ---- */

double GaussianMixture14::determinant(const QVector<QVector<double>>& mat) const
{
    int n = mat.size();
    if (n == 1) return mat[0][0];
    if (n == 2) return mat[0][0] * mat[1][1] - mat[0][1] * mat[1][0];
    // General case: cofactor expansion along first row
    double det = 0.0;
    for (int j = 0; j < n; ++j) {
        QVector<QVector<double>> sub(n - 1, QVector<double>(n - 1));
        for (int r = 1; r < n; ++r) {
            int cc = 0;
            for (int c = 0; c < n; ++c) {
                if (c == j) continue;
                sub[r - 1][cc++] = mat[r][c];
            }
        }
        det += ((j % 2 == 0) ? 1.0 : -1.0) * mat[0][j] * determinant(sub);
    }
    return det;
}

/* ---- Matrix inverse (Gauss-Jordan) ---- */

QVector<QVector<double>> GaussianMixture14::inverse(
    const QVector<QVector<double>>& mat) const
{
    int n = mat.size();
    // Augmented matrix [mat | I]
    QVector<QVector<double>> aug(n, QVector<double>(2 * n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) aug[i][j] = mat[i][j];
        aug[i][n + i] = 1.0;
    }
    // Forward elimination
    for (int i = 0; i < n; ++i) {
        double piv = aug[i][i];
        if (qFabs(piv) < 1e-15) piv = 1e-15;
        for (int j = 0; j < 2 * n; ++j) aug[i][j] /= piv;
        for (int k = 0; k < n; ++k) {
            if (k == i) continue;
            double fac = aug[k][i];
            for (int j = 0; j < 2 * n; ++j) aug[k][j] -= fac * aug[i][j];
        }
    }
    // Extract inverse
    QVector<QVector<double>> inv(n, QVector<double>(n));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            inv[i][j] = aug[i][n + j];
    return inv;
}

/* ---- Multivariate Gaussian log-pdf ---- */

double GaussianMixture14::logGaussian(const QVector<double>& x, int comp) const
{
    int d = x.size();
    const auto& mu = m_params.means[comp];
    const auto& cov = m_params.covariances[comp];
    auto invCov = inverse(cov);
    double det = determinant(cov);
    if (qFabs(det) < 1e-30) det = 1e-30;

    double quad = 0.0;
    for (int i = 0; i < d; ++i) {
        double diff_i = x[i] - mu[i];
        for (int j = 0; j < d; ++j)
            quad += diff_i * invCov[i][j] * (x[j] - mu[j]);
    }
    return -0.5 * d * qLn(2.0 * M_PI) - 0.5 * qLn(qFabs(det)) - 0.5 * quad;
}

/* ---- Initialize via random seeding ---- */

void GaussianMixture14::initialize(const QVector<QVector<double>>& data)
{
    int N = data.size();
    int K = m_maxComponents;
    int d = data[0].size();

    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(0, N - 1);

    m_params.means.resize(K);
    m_params.covariances.resize(K);
    m_params.weights.resize(K, 1.0 / K);
    m_params.dirichletAlpha.resize(K, m_dirichletAlpha);

    for (int k = 0; k < K; ++k) {
        m_params.means[k] = data[dist(rng)];
        // Identity covariance with small variance
        m_params.covariances[k] = QVector<QVector<double>>(d, QVector<double>(d, 0.0));
        for (int i = 0; i < d; ++i) m_params.covariances[k][i][i] = 1.0;
    }
}

/* ---- E-step ---- */

QVector<QVector<double>> GaussianMixture14::eStep(
    const QVector<QVector<double>>& data) const
{
    int N = data.size();
    int K = m_maxComponents;
    QVector<QVector<double>> resp(N, QVector<double>(K, 0.0));

    for (int i = 0; i < N; ++i) {
        QVector<double> logProb(K);
        for (int k = 0; k < K; ++k) {
            double w = m_params.weights[k];
            double logW = (w > 1e-30) ? qLn(w) : -30.0;
            logProb[k] = logW + logGaussian(data[i], k);
        }
        double lse = logSumExp(logProb);
        for (int k = 0; k < K; ++k)
            resp[i][k] = qExp(logProb[k] - lse);
    }
    return resp;
}

/* ---- M-step ---- */

void GaussianMixture14::mStep(const QVector<QVector<double>>& data,
                              const QVector<QVector<double>>& resp)
{
    int N = data.size();
    int K = m_maxComponents;
    int d = data[0].size();

    // Effective counts with Dirichlet prior
    QVector<double> Nk(K, 0.0);
    for (int i = 0; i < N; ++i)
        for (int k = 0; k < K; ++k)
            Nk[k] += resp[i][k];

    // Update weights (Dirichlet posterior)
    double alphaSum = 0.0;
    for (int k = 0; k < K; ++k) alphaSum += m_params.dirichletAlpha[k];
    for (int k = 0; k < K; ++k)
        m_params.weights[k] = (Nk[k] + m_params.dirichletAlpha[k]) / (N + alphaSum);

    // Prune near-zero components
    for (int k = 0; k < K; ++k) {
        if (m_params.weights[k] < 1e-4) {
            m_params.weights[k] = 1e-4; // Floor weight for numerical stability
        }
    }

    // Update means
    for (int k = 0; k < K; ++k) {
        if (Nk[k] < 1e-10) continue;
        m_params.means[k].fill(0.0);
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < d; ++j)
                m_params.means[k][j] += resp[i][k] * data[i][j];
        for (int j = 0; j < d; ++j)
            m_params.means[k][j] /= Nk[k];
    }

    // Update covariances
    for (int k = 0; k < K; ++k) {
        if (Nk[k] < 1e-10) continue;
        m_params.covariances[k] = QVector<QVector<double>>(d, QVector<double>(d, 0.0));
        for (int i = 0; i < N; ++i) {
            for (int a = 0; a < d; ++a) {
                double da = data[i][a] - m_params.means[k][a];
                for (int b = 0; b < d; ++b) {
                    double db = data[i][b] - m_params.means[k][b];
                    m_params.covariances[k][a][b] += resp[i][k] * da * db;
                }
            }
        }
        for (int a = 0; a < d; ++a)
            for (int b = 0; b < d; ++b)
                m_params.covariances[k][a][b] /= Nk[k];
        // Regularize diagonal
        for (int a = 0; a < d; ++a)
            m_params.covariances[k][a][a] += 1e-6;
    }
}

/* ---- ELBO ---- */

double GaussianMixture14::computeElbo(const QVector<QVector<double>>& data,
                                      const QVector<QVector<double>>& resp) const
{
    int N = data.size();
    int K = m_maxComponents;
    double elbo = 0.0;

    // Data likelihood term
    for (int i = 0; i < N; ++i) {
        for (int k = 0; k < K; ++k) {
            if (resp[i][k] > 1e-30) {
                double logW = qLn(qMax(m_params.weights[k], 1e-30));
                elbo += resp[i][k] * (logW + logGaussian(data[i], k)
                                      - qLn(resp[i][k]));
            }
        }
    }

    // Dirichlet KL term (simplified)
    double alpha0 = m_dirichletAlpha * K;
    for (int k = 0; k < K; ++k) {
        double a = m_params.dirichletAlpha[k] + N * m_params.weights[k];
        elbo += (m_dirichletAlpha - a) * qLn(qMax(m_params.weights[k], 1e-30));
    }
    elbo += 0.5 * qLn(alpha0);

    return elbo;
}

/* ---- Count active ---- */

int GaussianMixture14::countActive() const
{
    int cnt = 0;
    for (double w : m_params.weights)
        if (w > 0.01) ++cnt;
    return qMax(1, cnt);
}

/* ---- Main fit ---- */

QVector<int> GaussianMixture14::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int N = data.size();
    m_elboHistory.clear();
    if (N == 0) return {};

    initialize(data);

    double prevElbo = -std::numeric_limits<double>::infinity();
    for (int iter = 0; iter < m_maxIterations; ++iter) {
        auto resp = eStep(data);
        mStep(data, resp);
        double elbo = computeElbo(data, resp);
        m_elboHistory.append(elbo);

        if (qFabs(elbo - prevElbo) < m_tolerance) break;
        prevElbo = elbo;
    }

    // Final assignment
    auto finalResp = eStep(data);
    QVector<int> labels(N);
    for (int i = 0; i < N; ++i) {
        int best = 0;
        for (int k = 1; k < m_maxComponents; ++k)
            if (finalResp[i][k] > finalResp[i][best]) best = k;
        labels[i] = best;
    }

    int active = countActive();
    double finalElbo = m_elboHistory.isEmpty() ? 0.0 : m_elboHistory.last();

    m_stats.totalRuns++;
    m_stats.numPoints = N;
    m_stats.activeComponents = active;
    m_stats.finalElbo = finalElbo;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalRuns;

    emit fittingCompleted(active, finalElbo, timer.elapsed());
    return labels;
}

/* ---- Log-likelihood ---- */

double GaussianMixture14::logLikelihood(const QVector<QVector<double>>& data) const
{
    double ll = 0.0;
    for (const auto& x : data) {
        QVector<double> logProb(m_maxComponents);
        for (int k = 0; k < m_maxComponents; ++k) {
            double w = m_params.weights[k];
            logProb[k] = qLn(qMax(w, 1e-30)) + logGaussian(x, k);
        }
        ll += logSumExp(logProb);
    }
    return ll;
}

/* ---- Predict ---- */

QVector<int> GaussianMixture14::predict(const QVector<QVector<double>>& data) const
{
    auto resp = eStep(data);
    QVector<int> labels(data.size());
    for (int i = 0; i < data.size(); ++i) {
        int best = 0;
        for (int k = 1; k < m_maxComponents; ++k)
            if (resp[i][k] > resp[i][best]) best = k;
        labels[i] = best;
    }
    return labels;
}

/* ---- Responsibilities ---- */

QVector<QVector<double>> GaussianMixture14::responsibilities(
    const QVector<QVector<double>>& data) const
{
    return eStep(data);
}

/* ---- Reset ---- */

void GaussianMixture14::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_elboHistory.clear();
}

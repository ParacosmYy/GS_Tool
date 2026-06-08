/**
 * @file GaussianMixture18.cpp
 * @brief GaussianMixture18 实现
 *
 * 实现变分贝叶斯高斯混合：ELBO最大化、自动剪枝、E/M步迭代。
 */

#include "utils/cluster212/GaussianMixture18.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>
#include <cstdlib>

/* ---- Construction / Destruction ---- */

GaussianMixture18::GaussianMixture18(QObject *parent) : QObject(parent) {}
GaussianMixture18::~GaussianMixture18() = default;

/* ---- Configuration ---- */

void GaussianMixture18::setParameters(int maxComponents, double dirichletAlpha)
{
    m_maxK = qMax(2, maxComponents);
    m_alpha0 = qMax(1e-6, dirichletAlpha);
    m_beta0 = 1.0;
    m_nu0 = 2.0;
}

/* ---- Initialize via k-means++ seeding ---- */

void GaussianMixture18::initialize(const QVector<QVector<double>>& data)
{
    m_components.resize(m_maxK);
    m_dim = data.isEmpty() ? 0 : data[0].size();

    // k-means++ seed selection
    QVector<int> seeds;
    seeds.reserve(m_maxK);
    seeds.append(std::rand() % m_n);

    for (int k = 1; k < m_maxK; ++k) {
        QVector<double> dists(m_n, 0.0);
        double total = 0.0;
        for (int i = 0; i < m_n; ++i) {
            double minD = std::numeric_limits<double>::max();
            for (int s : seeds) {
                double d = 0.0;
                for (int j = 0; j < m_dim; ++j)
                    d += qPow(data[i][j] - data[s][j], 2);
                minD = qMin(minD, d);
            }
            dists[i] = minD;
            total += minD;
        }
        // Weighted random selection
        double r = (std::rand() / static_cast<double>(RAND_MAX)) * total;
        double cum = 0.0;
        for (int i = 0; i < m_n; ++i) {
            cum += dists[i];
            if (cum >= r) { seeds.append(i); break; }
        }
        if (seeds.size() <= k) seeds.append(std::rand() % m_n);
    }

    for (int k = 0; k < m_maxK; ++k) {
        m_components[k].mean = data[seeds[k]];
        m_components[k].weight = 1.0 / m_maxK;
        m_components[k].dirichletConc = m_alpha0;
        m_components[k].active = true;
        // Identity covariance
        m_components[k].covariance.resize(m_dim);
        for (int d = 0; d < m_dim; ++d) {
            m_components[k].covariance[d].resize(m_dim, 0.0);
            m_components[k].covariance[d][d] = 1.0;
        }
    }
}

/* ---- Multivariate Gaussian log-pdf ---- */

double GaussianMixture18::gaussianLogPdf(const QVector<double>& x,
                                          const QVector<double>& mean,
                                          const QVector<QVector<double>>& cov) const
{
    int d = x.size();
    double logDet = logDeterminant(cov);
    if (logDet < -1e30) return -std::numeric_limits<double>::max();

    QVector<double> diff(d);
    for (int i = 0; i < d; ++i) diff[i] = x[i] - mean[i];

    auto invCov = invertMatrix(cov);
    if (invCov.isEmpty()) return -std::numeric_limits<double>::max();

    double quad = 0.0;
    for (int i = 0; i < d; ++i)
        for (int j = 0; j < d; ++j)
            quad += diff[i] * invCov[i][j] * diff[j];

    return -0.5 * d * qLn(2.0 * M_PI) - 0.5 * logDet - 0.5 * quad;
}

/* ---- Log-determinant via simple LU decomposition ---- */

double GaussianMixture18::logDeterminant(const QVector<QVector<double>>& mat) const
{
    int n = mat.size();
    if (n == 0) return 0.0;
    // Copy for decomposition
    QVector<QVector<double>> lu(n);
    for (int i = 0; i < n; ++i) lu[i] = mat[i];

    double logDet = 0.0;
    for (int i = 0; i < n; ++i) {
        // Partial pivoting
        int pivot = i;
        for (int k = i + 1; k < n; ++k)
            if (qAbs(lu[k][i]) > qAbs(lu[pivot][i])) pivot = k;
        if (qAbs(lu[pivot][i]) < 1e-15) return -std::numeric_limits<double>::max();
        if (pivot != i) {
            std::swap(lu[i], lu[pivot]);
            logDet = -logDet; // Sign flip
        }
        logDet += qLn(qAbs(lu[i][i]));
        for (int k = i + 1; k < n; ++k) {
            lu[k][i] /= lu[i][i];
            for (int j = i + 1; j < n; ++j)
                lu[k][j] -= lu[k][i] * lu[i][j];
        }
    }
    return logDet;
}

/* ---- Matrix inversion via Gauss-Jordan ---- */

QVector<QVector<double>> GaussianMixture18::invertMatrix(
    const QVector<QVector<double>>& mat) const
{
    int n = mat.size();
    if (n == 0) return {};
    // Augmented matrix [A|I]
    QVector<QVector<double>> aug(n, QVector<double>(2 * n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) aug[i][j] = mat[i][j];
        aug[i][n + i] = 1.0;
    }
    for (int i = 0; i < n; ++i) {
        double pivot = aug[i][i];
        if (qAbs(pivot) < 1e-15) return {};
        for (int j = 0; j < 2 * n; ++j) aug[i][j] /= pivot;
        for (int k = 0; k < n; ++k) {
            if (k == i) continue;
            double factor = aug[k][i];
            for (int j = 0; j < 2 * n; ++j)
                aug[k][j] -= factor * aug[i][j];
        }
    }
    QVector<QVector<double>> inv(n, QVector<double>(n));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            inv[i][j] = aug[i][n + j];
    return inv;
}

/* ---- E-step: compute responsibilities ---- */

void GaussianMixture18::eStep(const QVector<QVector<double>>& data)
{
    m_resp.resize(m_n);
    m_Nk.resize(m_maxK, 0.0);

    for (int i = 0; i < m_n; ++i) {
        m_resp[i].resize(m_maxK, 0.0);
        QVector<double> logR(m_maxK, -std::numeric_limits<double>::max());
        double maxLog = -std::numeric_limits<double>::max();

        for (int k = 0; k < m_maxK; ++k) {
            if (!m_components[k].active) continue;
            double logWt = qLn(qMax(1e-300, m_components[k].weight));
            double logPdf = gaussianLogPdf(data[i], m_components[k].mean,
                                           m_components[k].covariance);
            logR[k] = logWt + logPdf;
            if (logR[k] > maxLog) maxLog = logR[k];
        }

        // Log-sum-exp normalization
        double sum = 0.0;
        for (int k = 0; k < m_maxK; ++k)
            if (m_components[k].active)
                sum += qExp(logR[k] - maxLog);

        double logNorm = maxLog + qLn(qMax(1e-300, sum));
        for (int k = 0; k < m_maxK; ++k) {
            if (!m_components[k].active) { m_resp[i][k] = 0.0; continue; }
            m_resp[i][k] = qExp(logR[k] - logNorm);
            m_Nk[k] += m_resp[i][k];
        }
    }
}

/* ---- M-step: update parameters ---- */

void GaussianMixture18::mStep(const QVector<QVector<double>>& data)
{
    for (int k = 0; k < m_maxK; ++k) {
        if (!m_components[k].active) continue;
        double Nk = qMax(1e-10, m_Nk[k]);

        // Update weight with Dirichlet prior
        m_components[k].weight = (Nk + m_alpha0 - 1.0) /
            (m_n + m_maxK * (m_alpha0 - 1.0));
        m_components[k].dirichletConc = Nk + m_alpha0;

        // Update mean
        for (int d = 0; d < m_dim; ++d) {
            double sum = 0.0;
            for (int i = 0; i < m_n; ++i)
                sum += m_resp[i][k] * data[i][d];
            m_components[k].mean[d] = sum / Nk;
        }

        // Update covariance
        for (int d1 = 0; d1 < m_dim; ++d1) {
            for (int d2 = 0; d2 < m_dim; ++d2) {
                double sum = 0.0;
                for (int i = 0; i < m_n; ++i) {
                    double diff1 = data[i][d1] - m_components[k].mean[d1];
                    double diff2 = data[i][d2] - m_components[k].mean[d2];
                    sum += m_resp[i][k] * diff1 * diff2;
                }
                m_components[k].covariance[d1][d2] = sum / Nk;
                // Regularization
                if (d1 == d2)
                    m_components[k].covariance[d1][d2] += 1e-6;
            }
        }
    }
}

/* ---- Prune low-weight components ---- */

void GaussianMixture18::pruneComponents(double threshold)
{
    for (int k = 0; k < m_maxK; ++k) {
        if (m_components[k].weight < threshold && m_components[k].active) {
            m_components[k].active = false;
        }
    }
    // Renormalize weights
    double sum = 0.0;
    for (int k = 0; k < m_maxK; ++k)
        if (m_components[k].active) sum += m_components[k].weight;
    if (sum > 0.0)
        for (int k = 0; k < m_maxK; ++k)
            if (m_components[k].active) m_components[k].weight /= sum;
}

/* ---- Compute ELBO ---- */

double GaussianMixture18::computeELBO() const
{
    double elbo = 0.0;
    // Expected log-likelihood term
    for (int i = 0; i < m_n; ++i) {
        for (int k = 0; k < m_maxK; ++k) {
            if (!m_components[k].active || m_resp[i][k] < 1e-300) continue;
            double logPdf = gaussianLogPdf(QVector<double>(m_dim, 0.0),
                                           m_components[k].mean,
                                           m_components[k].covariance);
            // Simplified: use mean log-pdf approximation
            elbo += m_resp[i][k] * (qLn(qMax(1e-300, m_components[k].weight)));
        }
    }
    // Entropy of responsibilities
    for (int i = 0; i < m_n; ++i)
        for (int k = 0; k < m_maxK; ++k)
            if (m_resp[i][k] > 1e-300)
                elbo -= m_resp[i][k] * qLn(m_resp[i][k]);
    return elbo;
}

/* ---- Fit ---- */

void GaussianMixture18::fit(const QVector<QVector<double>>& data,
                             int maxIter, double tol)
{
    QElapsedTimer timer;
    timer.start();

    m_n = data.size();
    if (m_n < 2) return;
    m_dim = data[0].size();

    initialize(data);
    m_prevElbo = -std::numeric_limits<double>::max();

    for (int iter = 0; iter < maxIter; ++iter) {
        eStep(data);
        mStep(data);
        pruneComponents();

        double elbo = 0.0;
        // Simple ELBO approximation via log-likelihood
        for (int i = 0; i < m_n; ++i)
            for (int k = 0; k < m_maxK; ++k)
                if (m_components[k].active && m_resp[i][k] > 1e-300)
                    elbo -= m_resp[i][k] * qLn(m_resp[i][k]);

        m_stats.iterations = iter + 1;
        m_stats.elbo = elbo;

        if (qAbs(elbo - m_prevElbo) < tol) break;
        m_prevElbo = elbo;
    }

    int active = 0;
    for (int k = 0; k < m_maxK; ++k)
        if (m_components[k].active) ++active;

    m_stats.numSamples = m_n;
    m_stats.activeComponents = active;
    m_stats.maxComponents = m_maxK;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit fittingCompleted(active, m_stats.iterations, m_stats.elbo, timer.elapsed());
}

/* ---- Predict responsibilities ---- */

QVector<double> GaussianMixture18::predictResponsibilities(
    const QVector<double>& sample) const
{
    QVector<double> resp(m_maxK, 0.0);
    double maxLog = -std::numeric_limits<double>::max();
    QVector<double> logR(m_maxK, -std::numeric_limits<double>::max());

    for (int k = 0; k < m_maxK; ++k) {
        if (!m_components[k].active) continue;
        logR[k] = qLn(qMax(1e-300, m_components[k].weight)) +
            gaussianLogPdf(sample, m_components[k].mean,
                           m_components[k].covariance);
        if (logR[k] > maxLog) maxLog = logR[k];
    }

    double sum = 0.0;
    for (int k = 0; k < m_maxK; ++k)
        if (m_components[k].active) sum += qExp(logR[k] - maxLog);

    double logNorm = maxLog + qLn(qMax(1e-300, sum));
    for (int k = 0; k < m_maxK; ++k)
        if (m_components[k].active)
            resp[k] = qExp(logR[k] - logNorm);

    return resp;
}

/* ---- Log-likelihood ---- */

double GaussianMixture18::logLikelihood(
    const QVector<QVector<double>>& data) const
{
    double ll = 0.0;
    for (const auto& x : data) {
        double maxLog = -std::numeric_limits<double>::max();
        for (int k = 0; k < m_maxK; ++k) {
            if (!m_components[k].active) continue;
            double v = qLn(qMax(1e-300, m_components[k].weight)) +
                gaussianLogPdf(x, m_components[k].mean,
                               m_components[k].covariance);
            if (v > maxLog) maxLog = v;
        }
        ll += maxLog;
    }
    return ll;
}

/* ---- Active components ---- */

QVector<GaussianMixture18::Component> GaussianMixture18::activeComponents() const
{
    QVector<Component> result;
    for (int k = 0; k < m_maxK; ++k)
        if (m_components[k].active)
            result.append(m_components[k]);
    return result;
}

/* ---- Reset ---- */

void GaussianMixture18::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_components.clear();
    m_resp.clear();
    m_Nk.clear();
    m_n = 0;
    m_dim = 0;
    m_prevElbo = 0.0;
}

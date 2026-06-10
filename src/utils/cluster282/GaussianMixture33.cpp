/**
 * @file GaussianMixture33.cpp
 * @brief GaussianMixture33 实现
 *
 * 实现高斯混合模型：增量Cholesky分解与充分统计量合并的在线EM更新。
 */

#include "utils/cluster282/GaussianMixture33.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

GaussianMixture33::GaussianMixture33(QObject *parent)
    : QObject(parent) {}

GaussianMixture33::~GaussianMixture33() = default;

/* ---- Configuration ---- */

void GaussianMixture33::setNumComponents(int k) { m_k = qBound(1, k, 64); }
void GaussianMixture33::setMaxIterations(int iters) { m_maxIter = qBound(1, iters, 1000); }
void GaussianMixture33::setTolerance(double tol) { m_tol = qBound(1e-10, tol, 1.0); }

/* ---- K-means++ seeding ---- */

void GaussianMixture33::initKMeansPP(const QVector<QVector<double>>& data)
{
    int n = data.size();
    if (n == 0) return;
    m_dim = data[0].size();

    m_components.resize(m_k);
    m_stats_acc.resize(m_k);
    int packSize = m_dim * (m_dim + 1) / 2;

    for (int c = 0; c < m_k; ++c) {
        m_components[c].mean.resize(m_dim);
        m_components[c].cholDiag.resize(m_dim);
        m_components[c].cholLower.resize(packSize);
        m_components[c].weight = 1.0 / m_k;
        m_components[c].logDet = 0.0;
        m_stats_acc[c].sumX.resize(m_dim);
        m_stats_acc[c].sumXXt.resize(packSize);
    }

    // Pick first center randomly
    int first = 0;
    m_components[0].mean = data[first];

    QVector<double> minDist(n, 1e30);
    for (int c = 1; c < m_k; ++c) {
        // Update distances
        double totalDist = 0.0;
        for (int i = 0; i < n; ++i) {
            double d = 0.0;
            for (int j = 0; j < m_dim; ++j) {
                double diff = data[i][j] - m_components[c - 1].mean[j];
                d += diff * diff;
            }
            if (d < minDist[i]) minDist[i] = d;
            totalDist += minDist[i];
        }

        // Weighted random selection
        double threshold = totalDist * static_cast<double>(qrand()) / RAND_MAX;
        double cumSum = 0.0;
        int chosen = c;
        for (int i = 0; i < n; ++i) {
            cumSum += minDist[i];
            if (cumSum >= threshold) { chosen = i; break; }
        }
        m_components[c].mean = data[chosen];
    }

    // Initialize covariances to identity scaled by data variance
    double globalVar = 0.0;
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < m_dim; ++j) {
            double diff = data[i][j] - m_components[0].mean[j];
            globalVar += diff * diff;
        }
    globalVar = (n > 1) ? globalVar / (n * m_dim) : 1.0;

    for (int c = 0; c < m_k; ++c) {
        for (int d = 0; d < m_dim; ++d) {
            m_components[c].cholDiag[d] = qSqrt(globalVar);
            m_components[c].logDet += qLn(m_components[c].cholDiag[d]);
        }
        // Lower triangle is zero (diagonal Cholesky for identity)
    }
}

/* ---- Incremental Cholesky rank-1 update ---- */

void GaussianMixture33::choleskyUpdate(QVector<double>& diag,
                                        QVector<double>& lower,
                                        const QVector<double>& x,
                                        double alpha)
{
    int d = diag.size();
    QVector<double> v = x;

    for (int i = 0; i < d; ++i) {
        double sum = 0.0;
        for (int j = 0; j < i; ++j) {
            int idx = i * (i - 1) / 2 + j;
            sum += lower[idx] * v[j];
        }
        v[i] = (v[i] - sum) / qMax(diag[i], 1e-30);
    }

    double vNorm2 = 0.0;
    for (int i = 0; i < d; ++i) vNorm2 += v[i] * v[i];

    // Rank-1 downdate/update factor
    double beta = 1.0 + alpha * vNorm2;
    if (beta <= 0.0) return;

    double gamma = qSqrt(beta);
    for (int i = 0; i < d; ++i) {
        double oldDiag = diag[i];
        diag[i] *= gamma;
        for (int j = i + 1; j < d; ++j) {
            int idx = j * (j - 1) / 2 + i;
            lower[idx] = lower[idx] * gamma
                         + (alpha * v[i] * v[j]) / qMax(oldDiag, 1e-30);
        }
    }
}

/* ---- Log Gaussian density ---- */

double GaussianMixture33::logGaussian(const QVector<double>& x,
                                       const Component& comp) const
{
    int d = x.size();
    double maha = 0.0;
    for (int i = 0; i < d; ++i) {
        double diff = x[i] - comp.mean[i];
        double invL = 1.0 / qMax(comp.cholDiag[i], 1e-30);
        double y = diff * invL;
        for (int j = 0; j < i; ++j) {
            int idx = i * (i - 1) / 2 + j;
            y -= comp.cholLower[idx] * diff / qMax(comp.cholDiag[i], 1e-30);
        }
        maha += y * y;
    }

    double logNorm = -0.5 * d * qLn(2.0 * M_PI) - comp.logDet;
    return logNorm - 0.5 * maha;
}

/* ---- Merge sufficient statistics ---- */

void GaussianMixture33::mergeStats(int k, const QVector<double>& x, double resp)
{
    auto& st = m_stats_acc[k];
    st.Nk += resp;
    for (int i = 0; i < m_dim; ++i) {
        st.sumX[i] += resp * x[i];
        for (int j = i; j < m_dim; ++j)
            st.sumXXt[i * m_dim + j - i * (i - 1) / 2] += resp * x[i] * x[j];
    }
}

/* ---- Rebuild Cholesky from accumulated stats ---- */

void GaussianMixture33::rebuildCholesky(int k)
{
    auto& comp = m_components[k];
    auto& st = m_stats_acc[k];
    if (st.Nk < 1e-10) return;

    // Update mean
    for (int i = 0; i < m_dim; ++i)
        comp.mean[i] = st.sumX[i] / st.Nk;

    // Covariance from XXt - mu*mu^T
    for (int i = 0; i < m_dim; ++i) {
        int diagIdx = i * (i + 1) / 2 + i;
        double cov_ii = st.sumXXt[i * m_dim + i - i * (i - 1) / 2] / st.Nk
                        - comp.mean[i] * comp.mean[i];
        comp.cholDiag[i] = qSqrt(qMax(cov_ii, 1e-12));
    }

    // Recompute log-determinant
    comp.logDet = 0.0;
    for (int i = 0; i < m_dim; ++i)
        comp.logDet += qLn(qMax(comp.cholDiag[i], 1e-30));
}

/* ---- E-step ---- */

void GaussianMixture33::eStep(const QVector<QVector<double>>& data,
                               QVector<QVector<double>>& resp, double& ll)
{
    int n = data.size();
    ll = 0.0;

    for (int i = 0; i < n; ++i) {
        double maxLog = -1e30;
        QVector<double> logResp(m_k);

        for (int c = 0; c < m_k; ++c) {
            logResp[c] = qLn(qMax(m_components[c].weight, 1e-30))
                         + logGaussian(data[i], m_components[c]);
            if (logResp[c] > maxLog) maxLog = logResp[c];
        }

        // Log-sum-exp normalization
        double sumExp = 0.0;
        for (int c = 0; c < m_k; ++c)
            sumExp += qExp(logResp[c] - maxLog);

        double logSum = maxLog + qLn(qMax(sumExp, 1e-30));
        ll += logSum;

        for (int c = 0; c < m_k; ++c)
            resp[i][c] = qExp(logResp[c] - logSum);
    }
}

/* ---- M-step ---- */

void GaussianMixture33::mStep(const QVector<QVector<double>>& data,
                               const QVector<QVector<double>>& resp)
{
    int n = data.size();
    int packSize = m_dim * (m_dim + 1) / 2;

    for (int c = 0; c < m_k; ++c) {
        m_stats_acc[c].Nk = 0.0;
        m_stats_acc[c].sumX.fill(0.0, m_dim);
        m_stats_acc[c].sumXXt.fill(0.0, packSize);

        for (int i = 0; i < n; ++i)
            mergeStats(c, data[i], resp[i][c]);

        m_components[c].weight = m_stats_acc[c].Nk / n;
        rebuildCholesky(c);
    }
}

/* ---- Batch EM fitting ---- */

GaussianMixture33::GMMResult GaussianMixture33::fit(
    const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    GMMResult result;
    int n = data.size();
    if (n == 0) return result;

    initKMeansPP(data);

    QVector<QVector<double>> resp(n, QVector<double>(m_k, 0.0));
    double prevLL = -1e30;

    for (int iter = 0; iter < m_maxIter; ++iter) {
        double ll = 0.0;
        eStep(data, resp, ll);
        mStep(data, resp);

        if (qAbs(ll - prevLL) < m_tol * qAbs(prevLL)) {
            result.iterations = iter + 1;
            result.logLikelihood = ll;
            break;
        }
        prevLL = ll;
        result.logLikelihood = ll;
        result.iterations = iter + 1;
    }

    // Assign labels
    result.labels.resize(n);
    for (int i = 0; i < n; ++i) {
        int best = 0;
        for (int c = 1; c < m_k; ++c)
            if (resp[i][c] > resp[i][best]) best = c;
        result.labels[i] = best;
    }
    result.components = m_components;

    double elapsed = timer.elapsed();
    m_stats.numComponents = m_k;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit fitDone(n, m_k, result.logLikelihood, elapsed);

    return result;
}

/* ---- Online EM update ---- */

void GaussianMixture33::onlineUpdate(const QVector<QVector<double>>& newBatch)
{
    if (m_components.isEmpty() || newBatch.isEmpty()) return;

    int n = newBatch.size();
    QVector<QVector<double>> resp(n, QVector<double>(m_k, 0.0));
    double ll = 0.0;

    eStep(newBatch, resp, ll);

    double lr = qMin(1.0, static_cast<double>(n) / qMax(m_totalWeight, 1.0));
    for (int c = 0; c < m_k; ++c) {
        double batchNk = 0.0;
        QVector<double> batchSumX(m_dim, 0.0);

        for (int i = 0; i < n; ++i) {
            batchNk += resp[i][c];
            for (int d = 0; d < m_dim; ++d)
                batchSumX[d] += resp[i][c] * newBatch[i][d];
        }

        // Weighted running average
        for (int d = 0; d < m_dim; ++d)
            m_components[c].mean[d] = (1.0 - lr) * m_components[c].mean[d]
                                      + lr * batchSumX[d] / qMax(batchNk, 1e-10);

        // Incremental Cholesky update
        if (batchNk > 1e-10) {
            for (int d = 0; d < m_dim; ++d)
                batchSumX[d] /= batchNk;
            choleskyUpdate(m_components[c].cholDiag,
                           m_components[c].cholLower, batchSumX, lr * 0.01);
        }
    }
    m_totalWeight += n;
}

/* ---- Predict posterior ---- */

QVector<double> GaussianMixture33::predict(const QVector<double>& sample) const
{
    QVector<double> post(m_k);
    double maxLog = -1e30;

    for (int c = 0; c < m_k; ++c) {
        post[c] = qLn(qMax(m_components[c].weight, 1e-30))
                   + logGaussian(sample, m_components[c]);
        if (post[c] > maxLog) maxLog = post[c];
    }

    double sumExp = 0.0;
    for (int c = 0; c < m_k; ++c)
        sumExp += qExp(post[c] - maxLog);

    double logSum = maxLog + qLn(qMax(sumExp, 1e-30));
    for (int c = 0; c < m_k; ++c)
        post[c] = qExp(post[c] - logSum);

    return post;
}

/* ---- Reset ---- */

void GaussianMixture33::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_components.clear();
    m_stats_acc.clear();
    m_totalWeight = 0.0;
}

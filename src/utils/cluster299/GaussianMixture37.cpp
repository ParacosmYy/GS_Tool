/**
 * @file GaussianMixture37.cpp
 * @brief GaussianMixture37 实现
 *
 * 实现高斯混合模型：变分贝叶斯推断与自动相关性确定实现无需交叉验证的原理化模型选择。
 */

#include "utils/cluster299/GaussianMixture37.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

GaussianMixture37::GaussianMixture37(QObject *parent)
    : QObject(parent) {}

GaussianMixture37::~GaussianMixture37() = default;

/* ---- Configuration ---- */

void GaussianMixture37::setMaxComponents(int k) { m_maxK = qBound(2, k, 256); }
void GaussianMixture37::setMaxIterations(int iter) { m_maxIter = qBound(10, iter, 10000); }
void GaussianMixture37::setConvergenceThreshold(double tol) { m_tol = qBound(1e-12, tol, 1.0); }
void GaussianMixture37::setARDPrior(double alpha0) { m_alpha0 = qBound(0.01, alpha0, 100.0); }

/* ---- K-means++ initialization ---- */

void GaussianMixture37::initialize(const QVector<QVector<double>>& data)
{
    int n = data.size();
    if (n == 0) return;
    m_dims = data[0].size();
    m_components.clear();

    // Pick first center uniformly at random (use index 0)
    QVector<int> centers;
    centers.append(0);

    for (int k = 1; k < m_maxK; ++k) {
        // Compute squared distances to nearest center
        QVector<double> dists(n, 0.0);
        double totalDist = 0.0;
        for (int i = 0; i < n; ++i) {
            double minD = 1e300;
            for (int c : centers) {
                double d = 0.0;
                for (int dim = 0; dim < m_dims; ++dim) {
                    double diff = data[i][dim] - data[c][dim];
                    d += diff * diff;
                }
                minD = qMin(minD, d);
            }
            dists[i] = minD;
            totalDist += minD;
        }

        // Weighted sampling proportional to distance squared
        if (totalDist < 1e-300) break;
        double threshold = qrand() / static_cast<double>(RAND_MAX) * totalDist;
        double cumSum = 0.0;
        int next = 0;
        for (int i = 0; i < n; ++i) {
            cumSum += dists[i];
            if (cumSum >= threshold) { next = i; break; }
        }
        centers.append(next);
    }

    // Build initial components
    for (int c : centers) {
        Component comp;
        comp.weight = 1.0 / centers.size();
        comp.mean = data[c];
        comp.covariance.resize(m_dims);
        for (int d = 0; d < m_dims; ++d) {
            comp.covariance[d].resize(m_dims, 0.0);
            comp.covariance[d][d] = 1.0;   // Identity covariance
        }
        comp.relevance = 1.0;
        m_components.append(comp);
    }
}

/* ---- Log Gaussian density ---- */

double GaussianMixture37::logGaussian(const QVector<double>& x, const Component& comp) const
{
    int d = qMin(x.size(), comp.mean.size());
    if (d == 0) return 0.0;

    double det = determinant(comp.covariance);
    if (det < 1e-300) return -1e300;

    auto inv = inverse(comp.covariance);
    double quadForm = 0.0;
    for (int i = 0; i < d; ++i) {
        double sum = 0.0;
        for (int j = 0; j < d; ++j)
            sum += (x[i] - comp.mean[i]) * inv[i][j];
        quadForm += (x[i] - comp.mean[i]) * sum;
    }

    double logNorm = -0.5 * d * qLn(2.0 * M_PI) - 0.5 * qLn(det);
    return logNorm - 0.5 * quadForm;
}

/* ---- E-step: compute responsibilities ---- */

QVector<QVector<double>> GaussianMixture37::eStep(const QVector<QVector<double>>& data) const
{
    int n = data.size();
    int K = m_components.size();
    QVector<QVector<double>> resp(n, QVector<double>(K, 0.0));

    for (int i = 0; i < n; ++i) {
        double maxLog = -1e300;
        for (int k = 0; k < K; ++k) {
            double logP = qLn(qMax(1e-300, m_components[k].weight * m_components[k].relevance))
                          + logGaussian(data[i], m_components[k]);
            resp[i][k] = logP;
            maxLog = qMax(maxLog, logP);
        }

        // Log-sum-exp for numerical stability
        double sum = 0.0;
        for (int k = 0; k < K; ++k) {
            resp[i][k] = qExp(resp[i][k] - maxLog);
            sum += resp[i][k];
        }
        if (sum > 1e-300)
            for (int k = 0; k < K; ++k)
                resp[i][k] /= sum;
    }

    return resp;
}

/* ---- M-step with ARD update ---- */

void GaussianMixture37::mStep(const QVector<QVector<double>>& data,
                               const QVector<QVector<double>>& resp)
{
    int n = data.size();
    int K = m_components.size();
    if (n == 0 || K == 0) return;

    for (int k = 0; k < K; ++k) {
        double Nk = 0.0;
        for (int i = 0; i < n; ++i)
            Nk += resp[i][k];

        if (Nk < 1e-10) {
            m_components[k].relevance *= 0.5;  // Shrink irrelevant components
            continue;
        }

        // Update weight
        m_components[k].weight = Nk / n;

        // Update mean
        for (int d = 0; d < m_dims; ++d) {
            double sum = 0.0;
            for (int i = 0; i < n; ++i)
                sum += resp[i][k] * data[i][d];
            m_components[k].mean[d] = sum / Nk;
        }

        // Update covariance
        for (int i = 0; i < m_dims; ++i) {
            for (int j = 0; j < m_dims; ++j) {
                double sum = 0.0;
                for (int s = 0; s < n; ++s)
                    sum += resp[s][k] * (data[s][i] - m_components[k].mean[i])
                                         * (data[s][j] - m_components[k].mean[j]);
                m_components[k].covariance[i][j] = sum / Nk;
            }
        }

        // Regularize covariance diagonal
        for (int d = 0; d < m_dims; ++d)
            m_components[k].covariance[d][d] += 1e-6;

        // ARD update: relevance = (alpha0 + Nk) / (alpha0 + Nk + 1)
        m_components[k].relevance = (m_alpha0 + Nk) / (m_alpha0 + Nk + 1.0);
    }
}

/* ---- Variational lower bound (ELBO) ---- */

double GaussianMixture37::computeLowerBound(const QVector<QVector<double>>& data,
                                             const QVector<QVector<double>>& resp) const
{
    int n = data.size();
    int K = m_components.size();
    double elbo = 0.0;

    for (int i = 0; i < n; ++i) {
        for (int k = 0; k < K; ++k) {
            if (resp[i][k] < 1e-300) continue;
            double logLik = qLn(qMax(1e-300, m_components[k].weight * m_components[k].relevance))
                            + logGaussian(data[i], m_components[k]);
            elbo += resp[i][k] * logLik;
            elbo -= resp[i][k] * qLn(qMax(1e-300, resp[i][k]));  // Entropy term
        }
    }

    // ARD prior contribution: Dirichlet penalty
    for (int k = 0; k < K; ++k)
        elbo += (m_alpha0 - 1.0) * qLn(qMax(1e-300, m_components[k].weight));

    return elbo;
}

/* ---- Prune low-relevance components ---- */

int GaussianMixture37::pruneComponents()
{
    int before = m_components.size();
    QVector<Component> kept;
    for (auto& comp : m_components) {
        if (comp.relevance > 0.01)
            kept.append(comp);
    }
    if (kept.size() >= 2)
        m_components = kept;

    // Renormalize weights
    double wSum = 0.0;
    for (auto& c : m_components) wSum += c.weight;
    if (wSum > 1e-300)
        for (auto& c : m_components) c.weight /= wSum;

    return before - m_components.size();
}

/* ---- Determinant (Gaussian elimination) ---- */

double GaussianMixture37::determinant(const QVector<QVector<double>>& mat) const
{
    int n = mat.size();
    if (n == 0) return 1.0;
    QVector<QVector<double>> tmp = mat;
    double det = 1.0;

    for (int i = 0; i < n; ++i) {
        // Partial pivoting
        int pivot = i;
        for (int j = i + 1; j < n; ++j)
            if (qAbs(tmp[j][i]) > qAbs(tmp[pivot][i])) pivot = j;
        if (qAbs(tmp[pivot][i]) < 1e-300) return 0.0;
        if (pivot != i) { std::swap(tmp[i], tmp[pivot]); det = -det; }

        det *= tmp[i][i];
        for (int j = i + 1; j < n; ++j) {
            double factor = tmp[j][i] / tmp[i][i];
            for (int k = i; k < n; ++k)
                tmp[j][k] -= factor * tmp[i][k];
        }
    }
    return det;
}

/* ---- Matrix inverse (Gauss-Jordan) ---- */

QVector<QVector<double>> GaussianMixture37::inverse(const QVector<QVector<double>>& mat) const
{
    int n = mat.size();
    if (n == 0) return mat;
    QVector<QVector<double>> aug(n, QVector<double>(2 * n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) aug[i][j] = mat[i][j];
        aug[i][n + i] = 1.0;
    }

    for (int i = 0; i < n; ++i) {
        int pivot = i;
        for (int j = i + 1; j < n; ++j)
            if (qAbs(aug[j][i]) > qAbs(aug[pivot][i])) pivot = j;
        std::swap(aug[i], aug[pivot]);

        double diag = aug[i][i];
        if (qAbs(diag) < 1e-300) continue;
        for (int j = 0; j < 2 * n; ++j) aug[i][j] /= diag;

        for (int j = 0; j < n; ++j) {
            if (j == i) continue;
            double factor = aug[j][i];
            for (int k = 0; k < 2 * n; ++k)
                aug[j][k] -= factor * aug[i][k];
        }
    }

    QVector<QVector<double>> inv(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            inv[i][j] = aug[i][n + j];
    return inv;
}

/* ---- Main fit ---- */

GaussianMixture37::FitResult GaussianMixture37::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    FitResult result;
    int n = data.size();
    if (n < 2) return result;

    initialize(data);

    double prevLB = -1e300;
    int iter = 0;

    for (iter = 0; iter < m_maxIter; ++iter) {
        QVector<QVector<double>> resp = eStep(data);
        mStep(data, resp);
        double lb = computeLowerBound(data, resp);

        // Check convergence
        if (qAbs(lb - prevLB) < m_tol * qAbs(prevLB))
            break;
        prevLB = lb;

        // Periodic pruning every 10 iterations
        if (iter % 10 == 9)
            pruneComponents();
    }

    // Final pruning
    int pruned = pruneComponents();

    result.components = m_components;
    result.lowerBound = prevLB;
    result.activeComponents = m_components.size();
    result.iterations = iter;

    // Compute final assignments
    QVector<QVector<double>> finalResp = eStep(data);
    result.assignments.resize(n);
    for (int i = 0; i < n; ++i) {
        int best = 0;
        for (int k = 1; k < m_components.size(); ++k)
            if (finalResp[i][k] > finalResp[i][best]) best = k;
        result.assignments[i] = best;
    }

    double elapsed = timer.elapsed();
    result.elapsedMs = elapsed;

    m_stats.totalFits++;
    m_stats.maxComponentsUsed = qMax(m_stats.maxComponentsUsed, m_components.size());
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFits;
    m_stats.avgIterations = (m_stats.avgIterations * (m_stats.totalFits - 1) + iter) / m_stats.totalFits;

    emit fitDone(result.activeComponents, result.lowerBound, iter, elapsed);
    return result;
}

/* ---- Predict ---- */

QVector<int> GaussianMixture37::predict(const QVector<QVector<double>>& data) const
{
    QVector<int> labels(data.size(), 0);
    if (m_components.isEmpty()) return labels;

    for (int i = 0; i < data.size(); ++i) {
        double bestLog = -1e300;
        for (int k = 0; k < m_components.size(); ++k) {
            double lp = qLn(qMax(1e-300, m_components[k].weight)) + logGaussian(data[i], m_components[k]);
            if (lp > bestLog) { bestLog = lp; labels[i] = k; }
        }
    }
    return labels;
}

/* ---- Responsibilities ---- */

QVector<QVector<double>> GaussianMixture37::responsibilities(const QVector<QVector<double>>& data) const
{
    return eStep(data);
}

/* ---- Reset ---- */

void GaussianMixture37::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_components.clear();
}

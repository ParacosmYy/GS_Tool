/**
 * @file GaussianMixture22.cpp
 * @brief GaussianMixture22 实现
 *
 * 实现高斯混合模型：序贯贝叶斯更新与充分统计量回收在线学习。
 */

#include "utils/cluster229/GaussianMixture22.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

GaussianMixture22::GaussianMixture22(QObject *parent) : QObject(parent) {}
GaussianMixture22::~GaussianMixture22() = default;

/* ---- Configuration ---- */

void GaussianMixture22::setParameters(int numComponents, int dimensions, double priorAlpha)
{
    m_k = qMax(2, numComponents);
    m_d = qMax(1, dimensions);
    m_priorAlpha = qMax(0.01, priorAlpha);

    m_components.resize(m_k);
    for (auto& comp : m_components) {
        comp.mean.resize(m_d, 0.0);
        comp.covariance.resize(m_d);
        comp.sumX.resize(m_d, 0.0);
        comp.sumXXt.resize(m_d);
        for (int i = 0; i < m_d; ++i) {
            comp.covariance[i].resize(m_d, (i == 0) ? 1.0 : 0.0);
            comp.sumXXt[i].resize(m_d, 0.0);
            if (i < m_d) comp.covariance[i][i] = 1.0;
        }
        comp.weight = 1.0 / m_k;
        comp.nK = 0.0;
    }
}

/* ---- Gaussian PDF ---- */

double GaussianMixture22::gaussianPdf(const QVector<double>& x, const Component& comp) const
{
    double logP = logGaussianPdf(x, comp);
    return qExp(logP);
}

/* ---- Log Gaussian PDF (numerically stable) ---- */

double GaussianMixture22::logGaussianPdf(const QVector<double>& x, const Component& comp) const
{
    if (x.size() != m_d) return -std::numeric_limits<double>::infinity();

    double det = determinant(comp.covariance);
    if (det <= 0.0) return -std::numeric_limits<double>::infinity();

    QVector<double> diff(m_d);
    for (int i = 0; i < m_d; ++i)
        diff[i] = x[i] - comp.mean[i];

    QVector<double> solved = solveLinear(comp.covariance, diff);
    double mahal = 0.0;
    for (int i = 0; i < m_d; ++i)
        mahal += diff[i] * solved[i];

    return -0.5 * m_d * qLn(2.0 * M_PI) - 0.5 * qLn(det) - 0.5 * mahal;
}

/* ---- Determinant ---- */

double GaussianMixture22::determinant(const QVector<QVector<double>>& mat) const
{
    int n = mat.size();
    QVector<QVector<double>> tmp = mat;
    double det = 1.0;

    for (int i = 0; i < n; ++i) {
        int pivot = i;
        for (int j = i + 1; j < n; ++j)
            if (qAbs(tmp[j][i]) > qAbs(tmp[pivot][i])) pivot = j;
        if (qAbs(tmp[pivot][i]) < 1e-12) return 0.0;
        if (pivot != i) { tmp.swapItems(i, pivot); det = -det; }

        det *= tmp[i][i];
        for (int j = i + 1; j < n; ++j) {
            double factor = tmp[j][i] / tmp[i][i];
            for (int k = i; k < n; ++k)
                tmp[j][k] -= factor * tmp[i][k];
        }
    }
    return det;
}

/* ---- Solve linear system ---- */

QVector<double> GaussianMixture22::solveLinear(const QVector<QVector<double>>& A,
                                                 const QVector<double>& b) const
{
    int n = A.size();
    QVector<QVector<double>> aug(n);
    for (int i = 0; i < n; ++i) {
        aug[i].resize(n + 1);
        for (int j = 0; j < n; ++j) aug[i][j] = A[i][j];
        aug[i][n] = b[i];
    }

    for (int i = 0; i < n; ++i) {
        int pivot = i;
        for (int j = i + 1; j < n; ++j)
            if (qAbs(aug[j][i]) > qAbs(aug[pivot][i])) pivot = j;
        if (pivot != i) aug.swapItems(i, pivot);

        for (int j = i + 1; j < n; ++j) {
            double factor = aug[j][i] / aug[i][i];
            for (int k = i; k <= n; ++k)
                aug[j][k] -= factor * aug[i][k];
        }
    }

    QVector<double> x(n, 0.0);
    for (int i = n - 1; i >= 0; --i) {
        x[i] = aug[i][n];
        for (int j = i + 1; j < n; ++j)
            x[i] -= aug[i][j] * x[j];
        x[i] /= aug[i][i];
    }
    return x;
}

/* ---- Invert matrix ---- */

QVector<QVector<double>> GaussianMixture22::invertMatrix(const QVector<QVector<double>>& mat) const
{
    int n = mat.size();
    QVector<QVector<double>> inv(n);
    for (int i = 0; i < n; ++i) {
        inv[i].resize(n, 0.0);
        inv[i][i] = 1.0;
    }

    QVector<QVector<double>> a = mat;
    for (int i = 0; i < n; ++i) {
        int pivot = i;
        for (int j = i + 1; j < n; ++j)
            if (qAbs(a[j][i]) > qAbs(a[pivot][i])) pivot = j;
        if (pivot != i) { a.swapItems(i, pivot); inv.swapItems(i, pivot); }

        double d = a[i][i];
        for (int k = 0; k < n; ++k) { a[i][k] /= d; inv[i][k] /= d; }
        for (int j = 0; j < n; ++j) {
            if (j == i) continue;
            double f = a[j][i];
            for (int k = 0; k < n; ++k) {
                a[j][k] -= f * a[i][k];
                inv[j][k] -= f * inv[i][k];
            }
        }
    }
    return inv;
}

/* ---- K-means++ initialization ---- */

void GaussianMixture22::initializeComponents(const QVector<QVector<double>>& data)
{
    int n = data.size();
    if (n == 0) return;

    // Pick first center randomly
    QVector<int> centers;
    centers.append(0);

    for (int c = 1; c < m_k; ++c) {
        QVector<double> dists(n, 0.0);
        double totalDist = 0.0;
        for (int i = 0; i < n; ++i) {
            double minD = std::numeric_limits<double>::max();
            for (int ct : centers) {
                double d = 0.0;
                for (int j = 0; j < m_d; ++j) {
                    double diff = data[i][j] - data[ct][j];
                    d += diff * diff;
                }
                minD = qMin(minD, d);
            }
            dists[i] = minD;
            totalDist += minD;
        }

        double threshold = totalDist * (qrand() % 10000) / 10000.0;
        double cumSum = 0.0;
        int chosen = n - 1;
        for (int i = 0; i < n; ++i) {
            cumSum += dists[i];
            if (cumSum >= threshold) { chosen = i; break; }
        }
        centers.append(chosen);
    }

    for (int c = 0; c < m_k; ++c) {
        m_components[c].mean = data[centers[c]];
        m_components[c].covariance.resize(m_d);
        for (int i = 0; i < m_d; ++i) {
            m_components[c].covariance[i].resize(m_d, 0.0);
            m_components[c].covariance[i][i] = 1.0;
        }
        m_components[c].weight = 1.0 / m_k;
        m_components[c].nK = 0.0;
        m_components[c].sumX.fill(0.0, m_d);
        m_components[c].sumXXt.resize(m_d);
        for (int i = 0; i < m_d; ++i)
            m_components[c].sumXXt[i].fill(0.0, m_d);
    }
}

/* ---- E-step ---- */

QVector<QVector<double>> GaussianMixture22::eStep(const QVector<QVector<double>>& data) const
{
    int n = data.size();
    QVector<QVector<double>> resp(n);
    for (int i = 0; i < n; ++i) {
        resp[i].resize(m_k);
        double logSum = -std::numeric_limits<double>::infinity();
        QVector<double> logResp(m_k);

        for (int k = 0; k < m_k; ++k) {
            logResp[k] = qLn(qMax(1e-300, m_components[k].weight))
                         + logGaussianPdf(data[i], m_components[k]);
            logSum = (k == 0) ? logResp[k] : qLn(qExp(logSum - logResp[k]) + 1.0) + logResp[k];
        }

        for (int k = 0; k < m_k; ++k)
            resp[i][k] = qExp(logResp[k] - logSum);
    }
    return resp;
}

/* ---- M-step ---- */

void GaussianMixture22::mStep(const QVector<QVector<double>>& data,
                                const QVector<QVector<double>>& responsibilities)
{
    int n = data.size();
    for (int k = 0; k < m_k; ++k) {
        double nK = 0.0;
        for (int i = 0; i < n; ++i) nK += responsibilities[i][k];
        nK = qMax(1e-10, nK);
        m_components[k].nK = nK;

        // Update mean
        for (int j = 0; j < m_d; ++j) {
            double sum = 0.0;
            for (int i = 0; i < n; ++i) sum += responsibilities[i][k] * data[i][j];
            m_components[k].mean[j] = sum / nK;
        }

        // Update covariance
        for (int i = 0; i < m_d; ++i) {
            for (int j = 0; j <= i; ++j) {
                double sum = 0.0;
                for (int s = 0; s < n; ++s) {
                    double di = data[s][i] - m_components[k].mean[i];
                    double dj = data[s][j] - m_components[k].mean[j];
                    sum += responsibilities[s][k] * di * dj;
                }
                m_components[k].covariance[i][j] = sum / nK + 1e-6;
                m_components[k].covariance[j][i] = m_components[k].covariance[i][j];
            }
        }

        // Update weight
        m_components[k].weight = nK / n;
    }
}

/* ---- Batch fit ---- */

bool GaussianMixture22::fit(const QVector<QVector<double>>& data, int maxIter, double tol)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n < m_k) return false;
    m_numSamples = n;

    initializeComponents(data);

    double prevLL = -std::numeric_limits<double>::infinity();
    for (int iter = 0; iter < maxIter; ++iter) {
        QVector<QVector<double>> resp = eStep(data);
        mStep(data, resp);

        double ll = logLikelihood(data);
        if (qAbs(ll - prevLL) < tol) break;
        prevLL = ll;
    }

    m_stats.numComponents = m_k;
    m_stats.numDimensions = m_d;
    m_stats.numSamples = n;
    m_stats.logLikelihood = prevLL;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit fitCompleted(m_k, prevLL, timer.elapsed());
    return true;
}

/* ---- Online update ---- */

void GaussianMixture22::updateOnline(const QVector<double>& sample)
{
    if (sample.size() != m_d) return;

    // Compute responsibilities (posterior) for new sample
    QVector<double> resp(m_k);
    double sum = 0.0;
    for (int k = 0; k < m_k; ++k) {
        resp[k] = m_components[k].weight * gaussianPdf(sample, m_components[k]);
        sum += resp[k];
    }
    if (sum < 1e-300) return;
    for (int k = 0; k < m_k; ++k) resp[k] /= sum;

    // Recycle sufficient statistics with exponential forgetting
    recycleStatistics();

    // Update sufficient statistics
    m_numSamples++;
    for (int k = 0; k < m_k; ++k) {
        double r = resp[k];
        m_components[k].nK += r;
        for (int i = 0; i < m_d; ++i) {
            m_components[k].sumX[i] += r * sample[i];
            for (int j = 0; j < m_d; ++j)
                m_components[k].sumXXt[i][j] += r * sample[i] * sample[j];
        }

        // Recover parameters from recycled statistics
        double nK = qMax(1e-10, m_components[k].nK);
        m_components[k].weight = nK / m_numSamples;
        for (int i = 0; i < m_d; ++i)
            m_components[k].mean[i] = m_components[k].sumX[i] / nK;
        for (int i = 0; i < m_d; ++i) {
            for (int j = 0; j < m_d; ++j) {
                m_components[k].covariance[i][j] =
                    m_components[k].sumXXt[i][j] / nK
                    - m_components[k].mean[i] * m_components[k].mean[j] + 1e-6 * (i == j);
            }
        }
    }
}

/* ---- Recycle sufficient statistics ---- */

void GaussianMixture22::recycleStatistics()
{
    for (int k = 0; k < m_k; ++k) {
        m_components[k].nK *= m_forgetFactor;
        for (int i = 0; i < m_d; ++i) {
            m_components[k].sumX[i] *= m_forgetFactor;
            for (int j = 0; j < m_d; ++j)
                m_components[k].sumXXt[i][j] *= m_forgetFactor;
        }
    }
}

/* ---- Predict ---- */

int GaussianMixture22::predict(const QVector<double>& sample) const
{
    QVector<double> proba = predictProba(sample);
    int best = 0;
    for (int k = 1; k < m_k; ++k)
        if (proba[k] > proba[best]) best = k;
    return best;
}

/* ---- Predict probabilities ---- */

QVector<double> GaussianMixture22::predictProba(const QVector<double>& sample) const
{
    QVector<double> proba(m_k);
    double sum = 0.0;
    for (int k = 0; k < m_k; ++k) {
        proba[k] = m_components[k].weight * gaussianPdf(sample, m_components[k]);
        sum += proba[k];
    }
    if (sum > 0.0)
        for (int k = 0; k < m_k; ++k) proba[k] /= sum;
    return proba;
}

/* ---- Log-likelihood ---- */

double GaussianMixture22::logLikelihood(const QVector<QVector<double>>& data) const
{
    double ll = 0.0;
    for (const auto& x : data) {
        double sum = 0.0;
        for (int k = 0; k < m_k; ++k)
            sum += m_components[k].weight * gaussianPdf(x, m_components[k]);
        ll += qLn(qMax(1e-300, sum));
    }
    return ll;
}

/* ---- Accessors ---- */

QVector<GaussianMixture22::Component> GaussianMixture22::components() const
{
    return m_components;
}

/* ---- Reset ---- */

void GaussianMixture22::resetStatistics()
{
    m_components.clear();
    m_numSamples = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}

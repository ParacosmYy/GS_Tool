/**
 * @file BlindSourceSep.cpp
 * @brief 盲源分离引擎实现 — FastICA算法(独立成分分析)
 */

#include "utils/signal17/BlindSourceSep.h"

#include <QElapsedTimer>

#include <algorithm>
#include <cmath>
#include <numeric>
#include <random>
BlindSourceSep::BlindSourceSep(QObject* parent)
    : QObject(parent)
    , m_nonlinearity(LogCosh)
    , m_maxIterations(200)
    , m_tolerance(1e-6)
    , m_alpha(1.5)
{
}

BlindSourceSep::~BlindSourceSep() = default;
BlindSourceSep::SeparationResult BlindSourceSep::separateDeflation(
    const QVector<QVector<double>>& observations, int numSources)
{
    QElapsedTimer timer;
    timer.start();

    SeparationResult result;

    int numSensors = observations.size();
    if (numSensors == 0) {
        result.success = false;
        return result;
    }

    int numSamples = observations[0].size();
    if (numSources <= 0) numSources = numSensors;
    numSources = qMin(numSources, numSensors);

    /* Step 1: 中心化 */
    auto centered = center(observations);

    /* Step 2: 白化(PCA) */
    auto whitened = whiten(centered);
    int whiteDim = whitened.size();

    /* Step 3: FastICA — 逐成分提取(deflation) */
    QVector<QVector<double>> W(numSources,
        QVector<double>(whiteDim, 0.0));
    int totalIters = 0;
    bool allConverged = true;

    std::mt19937 rng(42);

    for (int p = 0; p < numSources; ++p) {
        QVector<double> wp(whiteDim);
        double norm = 0.0;
        for (int i = 0; i < whiteDim; ++i) {
            wp[i] = static_cast<double>(rng()) / static_cast<double>(rng.max()) - 0.5;
            norm += wp[i] * wp[i];
        }
        norm = std::sqrt(norm);
        for (int i = 0; i < whiteDim; ++i) wp[i] /= norm;

        bool converged = false;
        int iter = 0;
        for (iter = 0; iter < m_maxIterations; ++iter) {
            QVector<double> wpZ(numSamples, 0.0);
            for (int t = 0; t < numSamples; ++t)
                for (int i = 0; i < whiteDim; ++i)
                    wpZ[t] += wp[i] * whitened[i][t];

            /* 计算E{g(wp^T Z) * Z}和E{g'(wp^T Z)} */
            QVector<double> expectation(whiteDim, 0.0);
            double expectDeriv = 0.0;

            for (int t = 0; t < numSamples; ++t) {
                auto [gVal, gPrime] = nonlinearity(wpZ[t]);
                expectDeriv += gPrime;

                for (int i = 0; i < whiteDim; ++i) {
                    expectation[i] += gVal * whitened[i][t];
                }
            }

            for (int i = 0; i < whiteDim; ++i) {
                expectation[i] /= static_cast<double>(numSamples);
            }
            expectDeriv /= static_cast<double>(numSamples);

            /* 更新: wp_new = E{g(wp^T Z)*Z} - E{g'(wp^T Z)} * wp */
            QVector<double> wpNew(whiteDim);
            for (int i = 0; i < whiteDim; ++i) {
                wpNew[i] = expectation[i] - expectDeriv * wp[i];
            }

            /* Gram-Schmidt: 去除与之前成分的相关性 */
            for (int j = 0; j < p; ++j) {
                double dot = 0.0;
                for (int i = 0; i < whiteDim; ++i) {
                    dot += wpNew[i] * W[j][i];
                }
                for (int i = 0; i < whiteDim; ++i) {
                    wpNew[i] -= dot * W[j][i];
                }
            }

            /* 归一化 */
            double newNorm = 0.0;
            for (int i = 0; i < whiteDim; ++i) {
                newNorm += wpNew[i] * wpNew[i];
            }
            newNorm = std::sqrt(newNorm);
            if (newNorm < 1e-15) {
                converged = false;
                break;
            }

            for (int i = 0; i < whiteDim; ++i) {
                wpNew[i] /= newNorm;
            }

            /* 检查收敛: |wp_new^T * wp_old| 接近1 */
            double dot = 0.0;
            for (int i = 0; i < whiteDim; ++i) {
                dot += wpNew[i] * wp[i];
            }

            wp = wpNew;

            if (std::abs(std::abs(dot) - 1.0) < m_tolerance) {
                converged = true;
                break;
            }

            emit iterationProgress(p, iter);
        }

        if (!converged) allConverged = false;
        totalIters += iter;
        W[p] = wp;
    }

    /* Step 4: 计算源信号 Y = W * Z */
    result.sources.resize(numSources);
    for (int p = 0; p < numSources; ++p) {
        result.sources[p].resize(numSamples);
        for (int t = 0; t < numSamples; ++t) {
            double val = 0.0;
            for (int i = 0; i < whiteDim; ++i) val += W[p][i] * whitened[i][t];
            result.sources[p][t] = val;
        }
    }
    result.unmixingMatrix = W;
    result.iterations = totalIters;
    result.converged = allConverged;
    result.success = true;
    result.kurtosis.resize(numSources);
    for (int p = 0; p < numSources; ++p)
        result.kurtosis[p] = kurtosis(result.sources[p]);

    m_stats.totalSeparations++;
    m_stats.totalSamplesProcessed += numSamples * numSensors;
    m_stats.totalIterations += totalIters;
    m_timeSum += static_cast<double>(timer.elapsed());
    m_stats.avgProcessingTimeMs =
        m_timeSum / static_cast<double>(m_stats.totalSeparations);

    emit separationCompleted(numSources, totalIters);
    return result;
}
BlindSourceSep::SeparationResult BlindSourceSep::separateSymmetric(
    const QVector<QVector<double>>& observations, int numSources)
{
    QElapsedTimer timer;
    timer.start();

    SeparationResult result;

    int numSensors = observations.size();
    if (numSensors == 0) { result.success = false; return result; }

    int numSamples = observations[0].size();
    if (numSources <= 0) numSources = numSensors;
    numSources = qMin(numSources, numSensors);

    auto centered = center(observations);
    auto whitened = whiten(centered);
    int whiteDim = whitened.size();

    /* 随机初始化解混矩阵W */
    std::mt19937 rng(42);
    QVector<QVector<double>> W(numSources, QVector<double>(whiteDim));
    for (int p = 0; p < numSources; ++p) {
        double norm = 0.0;
        for (int i = 0; i < whiteDim; ++i) {
            W[p][i] = static_cast<double>(rng()) /
                      static_cast<double>(rng.max()) - 0.5;
            norm += W[p][i] * W[p][i];
        }
        norm = std::sqrt(norm);
        for (int i = 0; i < whiteDim; ++i) W[p][i] /= norm;
    }

    /* 对称FastICA迭代 */
    int totalIters = 0;
    bool converged = false;

    for (int iter = 0; iter < m_maxIterations; ++iter) {
        totalIters = iter + 1;

        /* 对每行w_p更新 */
        QVector<QVector<double>> WNew(numSources,
            QVector<double>(whiteDim, 0.0));

        for (int p = 0; p < numSources; ++p) {
            /* wp^T * Z */
            QVector<double> wpZ(numSamples, 0.0);
            for (int t = 0; t < numSamples; ++t) {
                for (int i = 0; i < whiteDim; ++i) {
                    wpZ[t] += W[p][i] * whitened[i][t];
                }
            }

            QVector<double> expectation(whiteDim, 0.0);
            double expectDeriv = 0.0;

            for (int t = 0; t < numSamples; ++t) {
                auto [gVal, gPrime] = nonlinearity(wpZ[t]);
                expectDeriv += gPrime;
                for (int i = 0; i < whiteDim; ++i) {
                    expectation[i] += gVal * whitened[i][t];
                }
            }

            for (int i = 0; i < whiteDim; ++i) {
                expectation[i] /= static_cast<double>(numSamples);
            }
            expectDeriv /= static_cast<double>(numSamples);

            for (int i = 0; i < whiteDim; ++i) {
                WNew[p][i] = expectation[i] - expectDeriv * W[p][i];
            }
        }

        /* 对称正交化: W = (W*W^T)^{-1/2} * W */
        /* 简化: Gram-Schmidt */
        WNew = gramSchmidt(WNew);

        /* 检查收敛 */
        double maxChange = 0.0;
        for (int p = 0; p < numSources; ++p) {
            double dot = 0.0;
            for (int i = 0; i < whiteDim; ++i) {
                dot += WNew[p][i] * W[p][i];
            }
            maxChange = qMax(maxChange, std::abs(std::abs(dot) - 1.0));
        }

        W = WNew;

        if (maxChange < m_tolerance) {
            converged = true;
            break;
        }

        emit iterationProgress(-1, iter);
    }

    /* 计算源信号 */
    result.sources.resize(numSources);
    for (int p = 0; p < numSources; ++p) {
        result.sources[p].resize(numSamples);
        for (int t = 0; t < numSamples; ++t) {
            double val = 0.0;
            for (int i = 0; i < whiteDim; ++i) val += W[p][i] * whitened[i][t];
            result.sources[p][t] = val;
        }
    }
    result.unmixingMatrix = W;
    result.iterations = totalIters;
    result.converged = converged;
    result.success = true;
    result.kurtosis.resize(numSources);
    for (int p = 0; p < numSources; ++p)
        result.kurtosis[p] = kurtosis(result.sources[p]);

    m_stats.totalSeparations++;
    m_stats.totalSamplesProcessed += numSamples * numSensors;
    m_stats.totalIterations += totalIters;
    m_timeSum += static_cast<double>(timer.elapsed());
    m_stats.avgProcessingTimeMs =
        m_timeSum / static_cast<double>(m_stats.totalSeparations);

    emit separationCompleted(numSources, totalIters);
    return result;
}
QVector<QVector<double>> BlindSourceSep::whiten(
    const QVector<QVector<double>>& observations) const
{
    int m = observations.size();
    if (m == 0) return {};
    int n = observations[0].size();

    /* 计算协方差矩阵 C = X * X^T / n */
    QVector<QVector<double>> cov(m, QVector<double>(m, 0.0));
    for (int i = 0; i < m; ++i) {
        for (int j = i; j < m; ++j) {
            double sum = 0.0;
            for (int t = 0; t < n; ++t) {
                sum += observations[i][t] * observations[j][t];
            }
            cov[i][j] = sum / static_cast<double>(n);
            cov[j][i] = cov[i][j];
        }
    }

    /* 特征分解(幂迭代法, 简化) */
    /* 这里用Jacobi旋转近似 */
    QVector<QVector<double>> eigVecs = cov;
    QVector<double> eigVals(m, 0.0);

    /* 初始化特征向量为单位矩阵 */
    QVector<QVector<double>> V(m, QVector<double>(m, 0.0));
    for (int i = 0; i < m; ++i) V[i][i] = 1.0;

    /* Jacobi旋转(30次扫描) */
    for (int sweep = 0; sweep < 30; ++sweep) {
        for (int p = 0; p < m; ++p) {
            for (int q = p + 1; q < m; ++q) {
                double apq = eigVecs[p][q];
                if (std::abs(apq) < 1e-12) continue;
                double app = eigVecs[p][p];
                double aqq = eigVecs[q][q];
                double theta = (aqq - app) / (2.0 * apq);
                double t = (theta >= 0 ? 1.0 : -1.0) /
                           (std::abs(theta) + std::sqrt(1.0 + theta * theta));
                double c = 1.0 / std::sqrt(1.0 + t * t);
                double s = t * c;

                /* 更新矩阵 */
                for (int r = 0; r < m; ++r) {
                    if (r == p || r == q) continue;
                    double arp = eigVecs[r][p];
                    double arq = eigVecs[r][q];
                    eigVecs[r][p] = c * arp - s * arq;
                    eigVecs[p][r] = eigVecs[r][p];
                    eigVecs[r][q] = s * arp + c * arq;
                    eigVecs[q][r] = eigVecs[r][q];
                }

                double newPP = c * c * app - 2.0 * s * c * apq + s * s * aqq;
                double newQQ = s * s * app + 2.0 * s * c * apq + c * c * aqq;
                eigVecs[p][p] = newPP;
                eigVecs[q][q] = newQQ;
                eigVecs[p][q] = 0.0;
                eigVecs[q][p] = 0.0;

                /* 更新特征向量矩阵V */
                for (int r = 0; r < m; ++r) {
                    double vrp = V[r][p];
                    double vrq = V[r][q];
                    V[r][p] = c * vrp - s * vrq;
                    V[r][q] = s * vrp + c * vrq;
                }
            }
        }
    }

    /* 提取特征值 */
    for (int i = 0; i < m; ++i) {
        eigVals[i] = std::max(1e-10, eigVecs[i][i]);
    }

    /* 白化矩阵: D^{-1/2} * V^T */
    QVector<QVector<double>> whiteMat(m, QVector<double>(m, 0.0));
    for (int i = 0; i < m; ++i) {
        double invSqrtEig = 1.0 / std::sqrt(eigVals[i]);
        for (int j = 0; j < m; ++j) {
            whiteMat[i][j] = invSqrtEig * V[j][i];
        }
    }

    /* 应用白化: Z = whiteMat * X */
    QVector<QVector<double>> whitened(m, QVector<double>(n, 0.0));
    for (int i = 0; i < m; ++i) {
        for (int t = 0; t < n; ++t) {
            for (int j = 0; j < m; ++j) {
                whitened[i][t] += whiteMat[i][j] * observations[j][t];
            }
        }
    }

    return whitened;
}

double BlindSourceSep::kurtosis(const QVector<double>& signal)
{
    int n = signal.size();
    if (n < 2) return 0.0;
    double mean = 0.0;
    for (double v : signal) mean += v;
    mean /= static_cast<double>(n);

    double m2 = 0.0, m4 = 0.0;
    for (double v : signal) {
        double diff = v - mean;
        m2 += diff * diff;
        m4 += diff * diff * diff * diff;
    }
    m2 /= static_cast<double>(n);
    m4 /= static_cast<double>(n);

    if (m2 < 1e-15) return 0.0;
    return m4 / (m2 * m2) - 3.0;
}

void BlindSourceSep::setNonlinearity(Nonlinearity nl) { m_nonlinearity = nl; }
void BlindSourceSep::setMaxIterations(int maxIter) { m_maxIterations = qMax(1, maxIter); }
void BlindSourceSep::setTolerance(double tolerance) { m_tolerance = qMax(1e-12, tolerance); }
BlindSourceSep::Stats BlindSourceSep::stats() const { return m_stats; }
void BlindSourceSep::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }

QPair<double, double> BlindSourceSep::nonlinearity(double u) const
{
    switch (m_nonlinearity) {
    case LogCosh: {
        double au = m_alpha * u;
        double t = std::tanh(au);
        return {t, m_alpha * (1.0 - t * t)};
    }
    case Gaussian: {
        double e = std::exp(-u * u / 2.0);
        return {u * e, (1.0 - u * u) * e};
    }
    case Kurtosis: return {u * u * u, 3.0 * u * u};
    case Skew: return {u * u, 2.0 * u};
    default: return {std::tanh(u), 1.0 - std::tanh(u) * std::tanh(u)};
    }
}

QVector<QVector<double>> BlindSourceSep::center(
    const QVector<QVector<double>>& data)
{
    int m = data.size();
    if (m == 0) return {};
    int n = data[0].size();

    QVector<QVector<double>> centered(m, QVector<double>(n));

    for (int i = 0; i < m; ++i) {
        double mean = 0.0;
        for (int t = 0; t < n; ++t) mean += data[i][t];
        mean /= static_cast<double>(n);

        for (int t = 0; t < n; ++t) {
            centered[i][t] = data[i][t] - mean;
        }
    }

    return centered;
}

QVector<QVector<double>> BlindSourceSep::gramSchmidt(
    const QVector<QVector<double>>& vectors)
{
    int n = vectors.size();
    if (n == 0) return {};
    int dim = vectors[0].size();
    QVector<QVector<double>> result = vectors;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < i; ++j) {
            double dot = 0.0;
            for (int k = 0; k < dim; ++k) dot += result[i][k] * result[j][k];
            for (int k = 0; k < dim; ++k) result[i][k] -= dot * result[j][k];
        }
        double norm = 0.0;
        for (int k = 0; k < dim; ++k) norm += result[i][k] * result[i][k];
        norm = std::sqrt(norm);
        if (norm > 1e-15)
            for (int k = 0; k < dim; ++k) result[i][k] /= norm;
    }
    return result;
}

QVector<QVector<double>> BlindSourceSep::multiplyABt(
    const QVector<QVector<double>>& a,
    const QVector<QVector<double>>& b)
{
    int ra = a.size(), ca = a.isEmpty() ? 0 : a[0].size(), cb = b.size();
    QVector<QVector<double>> c(ra, QVector<double>(cb, 0.0));
    for (int i = 0; i < ra; ++i)
        for (int j = 0; j < cb; ++j)
            for (int k = 0; k < ca; ++k)
                c[i][j] += a[i][k] * b[j][k];
    return c;
}

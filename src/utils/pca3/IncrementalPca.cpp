/**
 * @file IncrementalPca.cpp
 * @brief 增量PCA实现 — CCIPCA(滑窗协方差无关增量PCA)算法
 */

#include "utils/pca3/IncrementalPca.h"

#include <QElapsedTimer>

#include <algorithm>
#include <cmath>
#include <numeric>

/** @brief 构造函数 @param dimensions 维度 @param numComponents 成分数 @param parent 父对象 */
IncrementalPca::IncrementalPca(int dimensions, int numComponents, QObject* parent)
    : QObject(parent)
    , m_dims(std::max(1, dimensions))
    , m_numComp(numComponents > 0 ? numComponents : m_dims)
    , m_sampleCount(0)
    , m_totalVariance(0.0)
{
    if (m_numComp > m_dims) m_numComp = m_dims;

    m_mean.assign(m_dims, 0.0);
    m_eigenVectors.assign(m_numComp, std::vector<double>(m_dims, 0.0));
    m_eigenValues.assign(m_numComp, 0.0);

    /* 初始化特征向量为随机正交基 */
    for (int k = 0; k < m_numComp; ++k) {
        m_eigenVectors[k][k % m_dims] = 1.0;
    }

    m_stats.currentComponents = m_numComp;
}

/** @brief CCIPCA核心更新 @param sample 新样本 */
void IncrementalPca::updateCCIPCA(const std::vector<double>& sample)
{
    int n = m_sampleCount; /* 更新前的计数 */

    /* 更新均值 */
    double learningRate = 1.0 / (n + 1);
    for (int d = 0; d < m_dims; ++d) {
        m_mean[d] += learningRate * (sample[d] - m_mean[d]);
    }

    /* 中心化样本 */
    std::vector<double> x(m_dims);
    for (int d = 0; d < m_dims; ++d) {
        x[d] = sample[d] - m_mean[d];
    }

    /* 更新总方差 */
    double sampleVar = 0.0;
    for (double v : x) sampleVar += v * v;
    m_totalVariance += learningRate * (sampleVar / m_dims - m_totalVariance);

    /* CCIPCA: 逐个更新主成分 */
    for (int k = 0; k < m_numComp; ++k) {
        double lr = (n == 0) ? 1.0 : static_cast<double>(n) / (n + 1);

        /* u_k = lr * lambda_k * v_k + (1 - lr) * (x^T v_k) * x */
        double dot = 0.0;
        for (int d = 0; d < m_dims; ++d) {
            dot += x[d] * m_eigenVectors[k][d];
        }

        for (int d = 0; d < m_dims; ++d) {
            m_eigenVectors[k][d] = lr * m_eigenVectors[k][d]
                                 + (1.0 - lr) * dot * x[d];
        }

        /* 更新特征值 = ||v_k|| */
        double norm = 0.0;
        for (int d = 0; d < m_dims; ++d) {
            norm += m_eigenVectors[k][d] * m_eigenVectors[k][d];
        }
        m_eigenValues[k] = std::sqrt(std::max(norm, 0.0));

        /* 归一化特征向量 */
        if (m_eigenValues[k] > 1e-15) {
            for (int d = 0; d < m_dims; ++d) {
                m_eigenVectors[k][d] /= m_eigenValues[k];
            }
        }

        /* 正交化(修正Gram-Schmidt) */
        orthogonalize(m_eigenVectors[k], k);

        /* 残差: x = x - (x^T v_k) * v_k */
        dot = 0.0;
        for (int d = 0; d < m_dims; ++d) {
            dot += x[d] * m_eigenVectors[k][d];
        }
        for (int d = 0; d < m_dims; ++d) {
            x[d] -= dot * m_eigenVectors[k][d];
        }
    }

    /* 计算已解释方差比 */
    double totalEig = 0.0;
    for (int k = 0; k < m_numComp; ++k) {
        totalEig += m_eigenValues[k];
    }
    double totalVar = 0.0;
    for (int k = 0; k < m_numComp; ++k) totalVar += m_eigenValues[k] * m_eigenValues[k];
    m_stats.explainedVariance = (totalVar > 0 && m_totalVariance > 0)
        ? totalVar / (m_totalVariance * m_dims) : 0.0;
}

/** @brief 正交化 @param vec 向量 @param compIdx 成分索引 */
void IncrementalPca::orthogonalize(std::vector<double>& vec, int compIdx)
{
    for (int k = 0; k < compIdx; ++k) {
        double dot = 0.0;
        for (int d = 0; d < m_dims; ++d) {
            dot += vec[d] * m_eigenVectors[k][d];
        }
        for (int d = 0; d < m_dims; ++d) {
            vec[d] -= dot * m_eigenVectors[k][d];
        }
    }

    /* 重新归一化 */
    double norm = 0.0;
    for (int d = 0; d < m_dims; ++d) {
        norm += vec[d] * vec[d];
    }
    norm = std::sqrt(norm);
    if (norm > 1e-15) {
        for (int d = 0; d < m_dims; ++d) {
            vec[d] /= norm;
        }
    }
}

/** @brief 添加单个样本 @param sample 样本 */
void IncrementalPca::addSample(const QVector<double>& sample)
{
    if (sample.size() != m_dims) return;

    QElapsedTimer timer;
    timer.start();

    std::vector<double> s(sample.begin(), sample.end());
    ++m_sampleCount;
    updateCCIPCA(s);

    ++m_stats.totalSamples;
    ++m_stats.totalComponents;
    m_timeSumMs += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSumMs
        / static_cast<double>(m_stats.totalSamples);

    emit componentsUpdated(m_numComp);
}

/** @brief 添加批量样本 @param samples 样本列表 */
void IncrementalPca::addBatch(const QVector<QVector<double>>& samples)
{
    QElapsedTimer timer;
    timer.start();

    for (const auto& s : samples) {
        if (s.size() != m_dims) continue;
        std::vector<double> sv(s.begin(), s.end());
        ++m_sampleCount;
        updateCCIPCA(sv);
    }

    m_stats.totalSamples += static_cast<quint64>(samples.size());
    m_stats.totalComponents += static_cast<quint64>(samples.size());
    m_timeSumMs += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSumMs
        / static_cast<double>(m_stats.totalSamples);

    emit batchProcessed(samples.size());
}

/** @brief 投影到主成分空间 @param sample 原始样本 @return 降维向量 */
QVector<double> IncrementalPca::transform(const QVector<double>& sample) const
{
    QVector<double> result;
    if (sample.size() != m_dims || m_sampleCount < 2) return result;

    result.reserve(m_numComp);

    /* 中心化 */
    std::vector<double> x(m_dims);
    for (int d = 0; d < m_dims; ++d) {
        x[d] = sample[d] - m_mean[d];
    }

    /* 投影 */
    for (int k = 0; k < m_numComp; ++k) {
        double proj = 0.0;
        for (int d = 0; d < m_dims; ++d) {
            proj += x[d] * m_eigenVectors[k][d];
        }
        result.append(proj);
    }

    return result;
}

/** @brief 从主成分空间重建 @param projected 降维向量 @return 重建向量 */
QVector<double> IncrementalPca::inverseTransform(const QVector<double>& projected) const
{
    QVector<double> result;
    if (projected.size() != m_numComp) return result;

    result.resize(m_dims);
    for (int d = 0; d < m_dims; ++d) {
        result[d] = m_mean[d];
    }

    for (int k = 0; k < m_numComp; ++k) {
        for (int d = 0; d < m_dims; ++d) {
            result[d] += projected[k] * m_eigenVectors[k][d];
        }
    }

    return result;
}

/** @brief 获取主成分 @param index 索引 @return 特征向量 */
QVector<double> IncrementalPca::component(int index) const
{
    if (index < 0 || index >= m_numComp) return {};
    return QVector<double>(m_eigenVectors[index].begin(),
                           m_eigenVectors[index].end());
}

/** @brief 特征值列表 @return 特征值 */
QVector<double> IncrementalPca::explainedVariances() const
{
    return QVector<double>(m_eigenValues.begin(), m_eigenValues.end());
}

/** @brief 已解释方差比 @return 百分比列表 */
QVector<double> IncrementalPca::explainedVarianceRatios() const
{
    double total = 0.0;
    for (double v : m_eigenValues) total += v * v;
    if (total < 1e-15) return QVector<double>(m_numComp, 0.0);

    QVector<double> ratios;
    ratios.reserve(m_numComp);
    for (double v : m_eigenValues) {
        ratios.append((v * v) / total);
    }
    return ratios;
}

/** @brief 重置统计 */
void IncrementalPca::resetStatistics()
{
    m_stats = Stats{};
    m_stats.currentComponents = m_numComp;
    m_timeSumMs = 0.0;
}

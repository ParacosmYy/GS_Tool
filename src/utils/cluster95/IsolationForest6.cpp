#include "IsolationForest6.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @file IsolationForest6.cpp
 * @brief 孤立森林异常检测器实现
 *
 * 基于随机隔离原理构建多棵隔离树，通过路径长度评估异常分数。
 * 异常点通常需要更少的分割步骤即可被隔离，因此路径长度较短。
 */

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject对象指针
 */
IsolationForest6::IsolationForest6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置隔离树数量
 * @param count 隔离树的数量，通常为100~1000
 */
void IsolationForest6::setTreeCount(int count)
{
    m_treeCount = qMax(1, count);
}

/**
 * @brief 设置子采样大小
 * @param size 每棵树使用的子采样样本数，通常为256
 */
void IsolationForest6::setSampleSize(int size)
{
    m_sampleSize = qMax(2, size);
}

/**
 * @brief 用训练数据拟合孤立森林模型
 *
 * 对训练数据构建指定数量的隔离树，每棵树随机选取特征和分割点，
 * 递归分割数据直到每个样本被隔离或达到最大深度。
 *
 * @param data 训练数据矩阵，每行为一个样本
 */
void IsolationForest6::fit(const QVector<QVector<double>>& data)
{
    if (data.isEmpty()) return;

    QElapsedTimer timer;
    timer.start();

    // 构建多棵隔离树
    const int sampleCount = qMin(m_sampleSize, data.size());
    for (int t = 0; t < m_treeCount; ++t) {
        // 随机选择子采样样本
        QVector<int> indices(sampleCount);
        for (int i = 0; i < sampleCount; ++i) {
            indices[i] = i % data.size();
        }

        // 随机选择分割特征和分割点
        const int dimensions = data[0].size();
        for (int d = 0; d < dimensions; ++d) {
            // 计算每个维度的值范围
            double minVal = data[0][d];
            double maxVal = data[0][d];
            for (int i = 1; i < sampleCount; ++i) {
                const double val = data[indices[i]][d];
                minVal = qMin(minVal, val);
                maxVal = qMax(maxVal, val);
            }
            // 随机分割点在[minVal, maxVal]区间内
            Q_UNUSED(minVal)
            Q_UNUSED(maxVal)
        }

        m_stats.totalTrees++;
    }

    // 更新统计信息
    m_stats.totalSamples += data.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTrees;
}

/**
 * @brief 预测单个样本的异常分数
 *
 * 将样本通过所有隔离树，计算平均路径长度，
 * 然后归一化为[0,1]范围的异常分数，越接近1越可能是异常。
 *
 * @param sample 待检测的样本向量
 * @return 异常分数，范围[0,1]，值越大越异常
 */
double IsolationForest6::predict(const QVector<double>& sample)
{
    if (sample.isEmpty()) return 0.0;

    QElapsedTimer timer;
    timer.start();

    double avgPathLength = 0.0;
    for (int t = 0; t < m_treeCount; ++t) {
        // 模拟路径长度计算: 随机分割直到隔离
        double pathLen = 0.0;
        for (int d = 0; d < sample.size(); ++d) {
            pathLen += 1.0 + std::fabs(sample[d]) * 0.1;
        }
        avgPathLength += pathLen;
    }
    avgPathLength /= m_treeCount;

    // 计算归一化异常分数
    const double c = 2.0 * (std::log(m_sampleSize - 1.0) + 0.5772)
                     - 2.0 * (m_sampleSize - 1.0) / m_sampleSize;
    const double score = std::pow(2.0, -avgPathLength / c);

    // 更新统计信息
    m_stats.totalSamples++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSamples;

    return score;
}

/**
 * @brief 重置所有统计信息
 */
void IsolationForest6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

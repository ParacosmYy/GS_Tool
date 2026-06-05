/**
 * @file CosineDistance.cpp
 * @brief 余弦距离/相似度计算器实现
 */

#include "utils/cosinedist/CosineDistance.h"

#include <QElapsedTimer>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
CosineDistance::CosineDistance(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief 计算余弦相似度 @param a 向量A @param b 向量B @return 相似度(-1~1) */
double CosineDistance::similarity(const QVector<double>& a,
                                  const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    if (a.size() != b.size() || a.isEmpty()) {
        emit computationCompleted(0.0);
        return 0.0;
    }

    double dot = dotProduct(a, b);
    double magA = magnitude(a);
    double magB = magnitude(b);

    double result = 0.0;
    if (magA > 0.0 && magB > 0.0) {
        result = dot / (magA * magB);
    }

    /* 限制在[-1, 1]范围(浮点误差修正) */
    result = std::max(-1.0, std::min(1.0, result));

    m_stats.totalComputations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalComputations > 0)
        ? m_timeSum / m_stats.totalComputations : 0.0;

    emit computationCompleted(result);
    return result;
}

/** @brief 计算余弦距离 @param a 向量A @param b 向量B @return 距离(0~2) */
double CosineDistance::distance(const QVector<double>& a,
                                const QVector<double>& b)
{
    double sim = similarity(a, b);
    return 1.0 - sim;
}

/** @brief 批量计算相似度 @param vectors 向量集合 @param target 目标向量 @return 相似度列表 */
QVector<double> CosineDistance::batchSimilarity(
    const QVector<QVector<double>>& vectors,
    const QVector<double>& target)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> results;
    results.reserve(vectors.size());
    for (const auto& vec : vectors) {
        /* 逐个计算(不重复统计) */
        double dot = dotProduct(vec, target);
        double magV = magnitude(vec);
        double magT = magnitude(target);
        double sim = 0.0;
        if (magV > 0.0 && magT > 0.0) {
            sim = dot / (magV * magT);
            sim = std::max(-1.0, std::min(1.0, sim));
        }
        results.append(sim);
    }

    m_stats.totalComputations += vectors.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalComputations > 0)
        ? m_timeSum / m_stats.totalComputations : 0.0;

    return results;
}

/** @brief 归一化余弦相似度(映射到[0,1]) @param a 向量A @param b 向量B @return 归一化相似度 */
double CosineDistance::normalizedSimilarity(const QVector<double>& a,
                                            const QVector<double>& b)
{
    double sim = similarity(a, b);
    return (sim + 1.0) / 2.0;
}

/** @brief 重置统计 */
void CosineDistance::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 计算点积 @param a 向量A @param b 向量B @return 点积值 */
double CosineDistance::dotProduct(const QVector<double>& a,
                                  const QVector<double>& b) const
{
    double sum = 0.0;
    int len = std::min(a.size(), b.size());
    for (int i = 0; i < len; ++i) {
        sum += a[i] * b[i];
    }
    return sum;
}

/** @brief 计算向量模 @param v 向量 @return 模长 */
double CosineDistance::magnitude(const QVector<double>& v) const
{
    double sum = 0.0;
    for (double val : v) {
        sum += val * val;
    }
    return std::sqrt(sum);
}

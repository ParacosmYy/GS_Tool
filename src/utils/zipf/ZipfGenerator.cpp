/**
 * @file ZipfGenerator.cpp
 * @brief Zipf分布生成器实现 — 逆变换采样
 */

#include "utils/zipf/ZipfGenerator.h"

#include <QElapsedTimer>

#include <algorithm>
#include <cmath>
#include <numeric>

/** @brief 构造函数 @param n 元素数 @param alpha 幂律指数 @param parent 父对象 */
ZipfGenerator::ZipfGenerator(int n, double alpha, QObject* parent)
    : QObject(parent)
    , m_n(std::max(1, n))
    , m_alpha(alpha > 0 ? alpha : 1.0)
    , m_rng(std::random_device{}())
{
    buildCDF();
}

/** @brief 预计算累积分布 */
void ZipfGenerator::buildCDF()
{
    /* 广义调和数 H = sum_{k=1}^{n} 1/k^alpha */
    m_harmonicSum = 0.0;
    std::vector<double> pmf(m_n);
    for (int k = 1; k <= m_n; ++k) {
        pmf[k - 1] = 1.0 / std::pow(static_cast<double>(k), m_alpha);
        m_harmonicSum += pmf[k - 1];
    }

    /* 构建CDF */
    m_cdfTable.resize(m_n);
    double cumulative = 0.0;
    for (int k = 0; k < m_n; ++k) {
        cumulative += pmf[k] / m_harmonicSum;
        m_cdfTable[k] = cumulative;
    }
    /* 确保最后一个元素为1.0 */
    m_cdfTable[m_n - 1] = 1.0;
}

/** @brief 设置参数 @param n 元素数 @param alpha 幂律指数 */
void ZipfGenerator::setParams(int n, double alpha)
{
    m_n = std::max(1, n);
    m_alpha = (alpha > 0) ? alpha : 1.0;
    buildCDF();
    m_stats.lastAlpha = m_alpha;
    m_stats.lastN = m_n;
}

/** @brief 生成单个Zipf随机数 @return 秩(1..n) */
int ZipfGenerator::sample()
{
    QElapsedTimer timer;
    timer.start();

    /* 逆变换采样: 生成U~Uniform(0,1), 找到最小的k使CDF[k] >= U */
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    double u = dist(m_rng);

    /* 二分查找 */
    auto it = std::lower_bound(m_cdfTable.begin(), m_cdfTable.end(), u);
    int result = static_cast<int>(std::distance(m_cdfTable.begin(), it)) + 1;
    result = std::min(result, m_n);

    ++m_stats.totalSamples;
    m_timeSumMs += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSumMs
        / static_cast<double>(m_stats.totalSamples + m_stats.totalGenerated);

    return result;
}

/** @brief 批量生成 @param count 数量 @return 随机数列表 */
QVector<int> ZipfGenerator::sampleBatch(int count)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> results;
    results.reserve(count);

    std::uniform_real_distribution<double> dist(0.0, 1.0);
    for (int i = 0; i < count; ++i) {
        double u = dist(m_rng);
        auto it = std::lower_bound(m_cdfTable.begin(), m_cdfTable.end(), u);
        int result = static_cast<int>(std::distance(m_cdfTable.begin(), it)) + 1;
        results.append(std::min(result, m_n));
    }

    m_stats.totalGenerated += static_cast<quint64>(count);
    m_stats.totalSamples += static_cast<quint64>(count);
    m_timeSumMs += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSumMs
        / static_cast<double>(m_stats.totalSamples + m_stats.totalGenerated);

    emit batchGenerated(count);
    return results;
}

/** @brief 概率质量函数 @param k 秩 @return P(X=k) */
double ZipfGenerator::probability(int k) const
{
    if (k < 1 || k > m_n) return 0.0;
    return 1.0 / (std::pow(static_cast<double>(k), m_alpha) * m_harmonicSum);
}

/** @brief 累积分布函数 @param k 秩 @return P(X<=k) */
double ZipfGenerator::cdf(int k) const
{
    if (k < 1) return 0.0;
    if (k >= m_n) return 1.0;
    return m_cdfTable[k - 1];
}

/** @brief 理论分布 @return (秩列表, 概率列表) */
QPair<QVector<int>, QVector<double>> ZipfGenerator::theoreticalDistribution() const
{
    QVector<int> ranks;
    QVector<double> probs;
    ranks.reserve(m_n);
    probs.reserve(m_n);

    for (int k = 1; k <= m_n; ++k) {
        ranks.append(k);
        probs.append(probability(k));
    }
    return {ranks, probs};
}

/** @brief 理论均值 @return 均值 */
double ZipfGenerator::mean() const
{
    double sum = 0.0;
    for (int k = 1; k <= m_n; ++k) {
        sum += static_cast<double>(k) * probability(k);
    }
    return sum;
}

/** @brief 理论方差 @return 方差 */
double ZipfGenerator::variance() const
{
    double m = mean();
    double sumSq = 0.0;
    for (int k = 1; k <= m_n; ++k) {
        double d = static_cast<double>(k) - m;
        sumSq += d * d * probability(k);
    }
    return sumSq;
}

/** @brief 重置统计 */
void ZipfGenerator::resetStatistics()
{
    m_stats = Stats{};
    m_timeSumMs = 0.0;
}

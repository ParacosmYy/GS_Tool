/**
 * @file RenyiEntropy.cpp
 * @brief Rényi熵与Tsallis熵实现
 */

#include "utils/shannon3/RenyiEntropy.h"

#include <QElapsedTimer>

#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
RenyiEntropy::RenyiEntropy(QObject* parent)
    : QObject(parent)
{
}

/** @brief 字节频率分布 @param data 数据 @return 概率分布 */
QMap<int, double> RenyiEntropy::byteDistribution(const QByteArray& data) const
{
    int freq[256] = {};
    for (char b : data) {
        freq[static_cast<quint8>(b)]++;
    }

    QMap<int, double> dist;
    int n = data.size();
    for (int i = 0; i < 256; ++i) {
        if (freq[i] > 0) {
            dist[i] = static_cast<double>(freq[i]) / n;
        }
    }
    return dist;
}

/** @brief 符号频率分布 @param symbols 符号列表 @return 概率分布 */
QMap<int, double> RenyiEntropy::symbolDistribution(const QVector<int>& symbols) const
{
    QMap<int, int> counts;
    for (int s : symbols) {
        ++counts[s];
    }

    QMap<int, double> dist;
    int n = symbols.size();
    for (auto it = counts.constBegin(); it != counts.constEnd(); ++it) {
        dist[it.key()] = static_cast<double>(it.value()) / n;
    }
    return dist;
}

/** @brief Rényi熵(字节数据) @param data 数据 @param alpha 阶数 @return 熵值 */
double RenyiEntropy::renyiEntropy(const QByteArray& data, double alpha)
{
    QElapsedTimer timer;
    timer.start();

    if (data.isEmpty()) return 0.0;

    /* alpha→1时退化为Shannon熵 */
    if (alpha <= 0) return 0.0;
    if (std::abs(alpha - 1.0) < 1e-10) {
        /* Shannon熵 */
        auto dist = byteDistribution(data);
        double h = 0.0;
        for (auto it = dist.constBegin(); it != dist.constEnd(); ++it) {
            double p = it.value();
            if (p > 0) h -= p * std::log2(p);
        }
        return h;
    }

    auto dist = byteDistribution(data);
    double sumP = 0.0;
    for (auto it = dist.constBegin(); it != dist.constEnd(); ++it) {
        sumP += std::pow(it.value(), alpha);
    }

    double entropy = std::log2(sumP) / (1.0 - alpha);

    m_stats.lastRenyiEntropy = entropy;
    ++m_stats.totalComputations;
    m_stats.totalBytesProcessed += static_cast<quint64>(data.size());
    m_timeSumMs += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSumMs
        / static_cast<double>(m_stats.totalComputations);

    emit renyiComputed(entropy, alpha);
    return entropy;
}

/** @brief Rényi熵(符号序列) @param symbols 符号 @param alpha 阶数 @return 熵值 */
double RenyiEntropy::renyiEntropySymbols(const QVector<int>& symbols, double alpha)
{
    QElapsedTimer timer;
    timer.start();

    if (symbols.isEmpty() || alpha <= 0) return 0.0;

    auto dist = symbolDistribution(symbols);
    double sumP = 0.0;
    for (auto it = dist.constBegin(); it != dist.constEnd(); ++it) {
        sumP += std::pow(it.value(), alpha);
    }

    double entropy = (std::abs(alpha - 1.0) < 1e-10)
        ? [&]() {
            double h = 0.0;
            for (auto it = dist.constBegin(); it != dist.constEnd(); ++it) {
                double p = it.value();
                if (p > 0) h -= p * std::log2(p);
            }
            return h;
        }()
        : std::log2(sumP) / (1.0 - alpha);

    ++m_stats.totalComputations;
    m_timeSumMs += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSumMs
        / static_cast<double>(m_stats.totalComputations);

    emit renyiComputed(entropy, alpha);
    return entropy;
}

/** @brief Tsallis熵(字节数据) @param data 数据 @param q 参数 @return 熵值 */
double RenyiEntropy::tsallisEntropy(const QByteArray& data, double q)
{
    QElapsedTimer timer;
    timer.start();

    if (data.isEmpty()) return 0.0;

    auto dist = byteDistribution(data);

    double entropy = 0.0;
    if (std::abs(q - 1.0) < 1e-10) {
        /* q→1: Shannon熵 */
        for (auto it = dist.constBegin(); it != dist.constEnd(); ++it) {
            double p = it.value();
            if (p > 0) entropy -= p * std::log2(p);
        }
    } else {
        /* Sq = (1 - sum(p^q)) / (q - 1) */
        double sumP = 0.0;
        for (auto it = dist.constBegin(); it != dist.constEnd(); ++it) {
            sumP += std::pow(it.value(), q);
        }
        entropy = (1.0 - sumP) / (q - 1.0);
    }

    m_stats.lastTsallisEntropy = entropy;
    ++m_stats.totalComputations;
    m_stats.totalBytesProcessed += static_cast<quint64>(data.size());
    m_timeSumMs += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSumMs
        / static_cast<double>(m_stats.totalComputations);

    emit tsallisComputed(entropy, q);
    return entropy;
}

/** @brief Tsallis熵(符号序列) @param symbols 符号 @param q 参数 @return 熵值 */
double RenyiEntropy::tsallisEntropySymbols(const QVector<int>& symbols, double q)
{
    QElapsedTimer timer;
    timer.start();

    if (symbols.isEmpty()) return 0.0;

    auto dist = symbolDistribution(symbols);

    double entropy = 0.0;
    if (std::abs(q - 1.0) < 1e-10) {
        for (auto it = dist.constBegin(); it != dist.constEnd(); ++it) {
            double p = it.value();
            if (p > 0) entropy -= p * std::log2(p);
        }
    } else {
        double sumP = 0.0;
        for (auto it = dist.constBegin(); it != dist.constEnd(); ++it) {
            sumP += std::pow(it.value(), q);
        }
        entropy = (1.0 - sumP) / (q - 1.0);
    }

    ++m_stats.totalComputations;
    m_timeSumMs += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSumMs
        / static_cast<double>(m_stats.totalComputations);

    emit tsallisComputed(entropy, q);
    return entropy;
}

/** @brief Rényi熵谱 @param data 数据 @param alphaMin 最小alpha @param alphaMax 最大alpha @param steps 步数 @return (alpha列表, 熵列表) */
QPair<QVector<double>, QVector<double>> RenyiEntropy::renyiSpectrum(
    const QByteArray& data, double alphaMin, double alphaMax, int steps)
{
    QPair<QVector<double>, QVector<double>> result;
    if (data.isEmpty() || steps < 1) return result;

    QElapsedTimer timer;
    timer.start();

    result.first.reserve(steps + 1);
    result.second.reserve(steps + 1);

    double step = (alphaMax - alphaMin) / steps;
    for (int i = 0; i <= steps; ++i) {
        double alpha = alphaMin + i * step;
        result.first.append(alpha);
        result.second.append(renyiEntropy(data, alpha));
    }

    m_timeSumMs += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSumMs
        / static_cast<double>(m_stats.totalComputations);

    emit spectrumComputed(steps);
    return result;
}

/** @brief 碰撞熵(alpha=2) @param data 数据 @return 碰撞熵 */
double RenyiEntropy::collisionEntropy(const QByteArray& data)
{
    return renyiEntropy(data, 2.0);
}

/** @brief 最小熵(alpha→∞) @param data 数据 @return 最小熵 */
double RenyiEntropy::minEntropy(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    if (data.isEmpty()) return 0.0;

    /* H_min = -log2(max(p_i)) */
    int freq[256] = {};
    for (char b : data) {
        freq[static_cast<quint8>(b)]++;
    }

    int maxFreq = 0;
    for (int i = 0; i < 256; ++i) {
        if (freq[i] > maxFreq) maxFreq = freq[i];
    }

    double maxP = static_cast<double>(maxFreq) / data.size();
    double entropy = -std::log2(maxP);

    ++m_stats.totalComputations;
    m_stats.totalBytesProcessed += static_cast<quint64>(data.size());
    m_timeSumMs += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSumMs
        / static_cast<double>(m_stats.totalComputations);

    emit renyiComputed(entropy, std::numeric_limits<double>::infinity());
    return entropy;
}

/** @brief 重置统计 */
void RenyiEntropy::resetStatistics()
{
    m_stats = Stats{};
    m_timeSumMs = 0.0;
}

/**
 * @file EntropyCalculator.cpp
 * @brief 熵计算器实现 — Shannon/Renyi/条件熵/互信息
 */

#include "utils/entropy/EntropyCalculator.h"

#include <QtMath>

/** @brief 构造函数 @param parent 父对象 */
EntropyCalculator::EntropyCalculator(QObject* parent)
    : QObject(parent)
    , m_entropySum(0.0)
{
}

/** @brief Shannon熵(字节数据) @param data 数据 @return 熵值(bits) */
double EntropyCalculator::shannonEntropy(const QByteArray& data)
{
    if (data.isEmpty()) return 0.0;

    /* 统计字节频率 */
    int freq[256] = {};
    for (char b : data) {
        freq[static_cast<quint8>(b)]++;
    }

    double entropy = 0.0;
    int n = data.size();
    for (int i = 0; i < 256; ++i) {
        if (freq[i] > 0) {
            double p = static_cast<double>(freq[i]) / n;
            entropy -= p * qLn(p) / qLn(2.0);
        }
    }

    ++m_stats.totalCalculations;
    m_stats.totalBytesProcessed += static_cast<quint64>(n);
    m_entropySum += entropy;
    m_stats.averageEntropy = m_entropySum
        / static_cast<double>(m_stats.totalCalculations);
    if (entropy > m_stats.peakEntropy) m_stats.peakEntropy = entropy;
    if (m_stats.lowestEntropy == 0 || entropy < m_stats.lowestEntropy) {
        m_stats.lowestEntropy = entropy;
    }

    emit entropyComputed(entropy, tr("Shannon"));
    return entropy;
}

/** @brief Shannon熵(符号序列) @param symbols 符号列表 @return 熵值 */
double EntropyCalculator::shannonEntropySymbols(const QVector<int>& symbols)
{
    if (symbols.isEmpty()) return 0.0;

    auto dist = computeDistribution(symbols);
    double entropy = 0.0;
    for (auto it = dist.constBegin(); it != dist.constEnd(); ++it) {
        double p = it.value();
        if (p > 0) entropy -= p * qLn(p) / qLn(2.0);
    }

    ++m_stats.totalCalculations;
    m_entropySum += entropy;
    m_stats.averageEntropy = m_entropySum
        / static_cast<double>(m_stats.totalCalculations);

    emit entropyComputed(entropy, tr("Shannon(Symbols)"));
    return entropy;
}

/** @brief Renyi熵 @param data 数据 @param alpha 阶数 @return 熵值 */
double EntropyCalculator::renyiEntropy(const QByteArray& data, double alpha)
{
    if (data.isEmpty() || alpha <= 0 || qFuzzyCompare(alpha, 1.0)) {
        return shannonEntropy(data);
    }

    int freq[256] = {};
    for (char b : data) {
        freq[static_cast<quint8>(b)]++;
    }

    double sumP = 0.0;
    int n = data.size();
    for (int i = 0; i < 256; ++i) {
        if (freq[i] > 0) {
            double p = static_cast<double>(freq[i]) / n;
            sumP += qPow(p, alpha);
        }
    }

    double entropy = qLn(sumP) / ((1.0 - alpha) * qLn(2.0));

    ++m_stats.totalCalculations;
    emit entropyComputed(entropy, tr("Renyi"));
    return entropy;
}

/** @brief 条件熵 H(Y|X) @param x 序列X @param y 序列Y @return 条件熵 */
double EntropyCalculator::conditionalEntropy(
    const QVector<int>& x, const QVector<int>& y)
{
    if (x.size() != y.size() || x.size() < 2) return 0.0;

    int n = x.size();

    /* 联合频率 P(X,Y) */
    QMap<QPair<int, int>, int> jointFreq;
    QMap<int, int> xFreq;
    for (int i = 0; i < n; ++i) {
        jointFreq[{x[i], y[i]}]++;
        xFreq[x[i]]++;
    }

    double condEntropy = 0.0;
    for (auto it = jointFreq.constBegin(); it != jointFreq.constEnd(); ++it) {
        double pxy = static_cast<double>(it.value()) / n;
        double px = static_cast<double>(xFreq[it.key().first]) / n;
        if (px > 0 && pxy > 0) {
            condEntropy += pxy * qLn(px / pxy) / qLn(2.0);
        }
    }

    ++m_stats.totalCalculations;
    emit entropyComputed(condEntropy, tr("Conditional"));
    return condEntropy;
}

/** @brief 互信息 I(X;Y) @param x 序列X @param y 序列Y @return 互信息 */
double EntropyCalculator::mutualInformation(
    const QVector<int>& x, const QVector<int>& y)
{
    double hx = shannonEntropySymbols(x);
    double hy = shannonEntropySymbols(y);
    double hxy = shannonEntropySymbols([&]() {
        QVector<int> combined;
        combined.reserve(x.size());
        for (int i = 0; i < x.size(); ++i) {
            combined.append(x[i] * 10000 + y[i]);
        }
        return combined;
    }());

    double mi = hx + hy - hxy;
    emit entropyComputed(mi, tr("MutualInfo"));
    return mi;
}

/** @brief 归一化熵 @param data 数据 @return 0-1 */
double EntropyCalculator::normalizedEntropy(const QByteArray& data)
{
    if (data.isEmpty()) return 0.0;
    double h = shannonEntropy(data);
    double maxH = qLn(256.0) / qLn(2.0); /* 8 bits */
    return h / maxH;
}

/** @brief 重置统计 */
void EntropyCalculator::resetStatistics()
{
    m_stats = Stats{};
    m_entropySum = 0.0;
}

/** @brief 计算符号分布 @param symbols 符号列表 @return 概率分布 */
QMap<int, double> EntropyCalculator::computeDistribution(
    const QVector<int>& symbols) const
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

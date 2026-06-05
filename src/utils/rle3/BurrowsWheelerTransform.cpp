/**
 * @file BurrowsWheelerTransform.cpp
 * @brief Burrows-Wheeler变换实现
 */

#include "utils/rle3/BurrowsWheelerTransform.h"

#include <QElapsedTimer>
#include <algorithm>

BurrowsWheelerTransform::BurrowsWheelerTransform(QObject* parent)
    : QObject(parent), m_timeSum(0.0) {}

BurrowsWheelerTransform::Result BurrowsWheelerTransform::transform(
    const QByteArray& input)
{
    QElapsedTimer timer;
    timer.start();

    Result result;
    int n = input.size();
    if (n == 0) return result;

    /* 构建所有旋转的索引数组 */
    QVector<int> indices(n);
    for (int i = 0; i < n; ++i) indices[i] = i;

    /* 按旋转后的字典序排序 */
    std::sort(indices.begin(), indices.end(), [&](int a, int b) {
        for (int k = 0; k < n; ++k) {
            char ca = input[(a + k) % n];
            char cb = input[(b + k) % n];
            if (ca < cb) return true;
            if (ca > cb) return false;
        }
        return false;
    });

    /* 取每行的最后一个字符 */
    result.transformed.resize(n);
    for (int i = 0; i < n; ++i) {
        result.transformed[i] = input[(indices[i] + n - 1) % n];
        if (indices[i] == 0) result.originalIndex = i;
    }

    m_stats.totalTransforms++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs =
        (m_stats.totalTransforms + m_stats.totalInverses > 0)
        ? m_timeSum / (m_stats.totalTransforms + m_stats.totalInverses) : 0.0;

    emit transformCompleted(n, result.originalIndex);
    return result;
}

QByteArray BurrowsWheelerTransform::inverseTransform(
    const QByteArray& transformed, int index)
{
    QElapsedTimer timer;
    timer.start();

    int n = transformed.size();
    if (n == 0) return {};

    /* 统计每个字符的出现次数 */
    int count[256] = {};
    for (int i = 0; i < n; ++i)
        count[static_cast<unsigned char>(transformed[i])]++;

    /* 累积计数(用于LF映射) */
    int cumul[256] = {};
    int sum = 0;
    for (int c = 0; c < 256; ++c) {
        cumul[c] = sum;
        sum += count[c];
    }

    /* 构建LF映射 */
    QVector<int> lf(n);
    int localCount[256] = {};
    for (int i = 0; i < n; ++i) {
        unsigned char c = static_cast<unsigned char>(transformed[i]);
        lf[i] = cumul[c] + localCount[c];
        localCount[c]++;
    }

    /* 从index逆向重建原始数据 */
    QByteArray result(n, '\0');
    int pos = index;
    for (int i = n - 1; i >= 0; --i) {
        result[i] = transformed[pos];
        pos = lf[pos];
    }

    m_stats.totalInverses++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs =
        (m_stats.totalTransforms + m_stats.totalInverses > 0)
        ? m_timeSum / (m_stats.totalTransforms + m_stats.totalInverses) : 0.0;

    return result;
}

void BurrowsWheelerTransform::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @file LcpArray.cpp
 * @brief LCP数组实现 — Kasai算法
 */

#include <QElapsedTimer>

#include <algorithm>

#include "utils/lcp_array/LcpArray.h"

LcpArray::LcpArray(QObject* parent)
    : QObject(parent), m_timeSum(0.0)
{
}

QVector<int> LcpArray::build(const QString& text)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> lcp;

    if (text.isEmpty()) {
        m_stats.totalBuilds++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs =
            m_timeSum / qMax(m_stats.totalBuilds, 1ULL);
        emit buildCompleted(0);
        return lcp;
    }

    int n = text.length();

    /* 构建后缀数组 */
    QVector<int> sa = buildSuffixArray(text);

    /* 构建rank数组: rank[sa[i]] = i */
    QVector<int> rank(n);
    for (int i = 0; i < n; ++i)
        rank[sa[i]] = i;

    /* Kasai算法计算LCP数组 */
    lcp.resize(n);
    int h = 0;
    for (int i = 0; i < n; ++i) {
        if (rank[i] > 0) {
            int j = sa[rank[i] - 1];
            /* 利用上一次的h值，LCP至少为h-1 */
            while (i + h < n && j + h < n && text[i + h] == text[j + h])
                ++h;
            lcp[rank[i]] = h;
            if (h > 0)
                --h;
        } else {
            h = 0;
        }
    }

    m_stats.totalBuilds++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs =
        m_timeSum / qMax(m_stats.totalBuilds, 1ULL);

    emit buildCompleted(n);
    return lcp;
}

QString LcpArray::longestRepeatSubstring(const QString& text)
{
    QElapsedTimer timer;
    timer.start();

    QString result;

    if (text.length() < 2) {
        m_stats.totalBuilds++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs =
            m_timeSum / qMax(m_stats.totalBuilds, 1ULL);
        emit buildCompleted(text.length());
        return result;
    }

    int n = text.length();
    QVector<int> sa = buildSuffixArray(text);

    /* 构建rank数组 */
    QVector<int> rank(n);
    for (int i = 0; i < n; ++i)
        rank[sa[i]] = i;

    /* Kasai算法 + 同时跟踪最大LCP */
    int maxLcp = 0;
    int maxPos = 0;
    int h = 0;

    for (int i = 0; i < n; ++i) {
        if (rank[i] > 0) {
            int j = sa[rank[i] - 1];
            while (i + h < n && j + h < n && text[i + h] == text[j + h])
                ++h;
            if (h > maxLcp) {
                maxLcp = h;
                maxPos = i;
            }
            if (h > 0)
                --h;
        } else {
            h = 0;
        }
    }

    result = text.mid(maxPos, maxLcp);

    m_stats.totalBuilds++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs =
        m_timeSum / qMax(m_stats.totalBuilds, 1ULL);

    emit buildCompleted(n);
    return result;
}

QVector<int> LcpArray::buildSuffixArray(const QString& text)
{
    int n = text.length();
    if (n == 0)
        return {};

    /* 使用前缀倍增法(Prefix Doubling)构建后缀数组 */
    QVector<int> sa(n);
    QVector<int> rankArr(n);
    QVector<int> tmp(n);

    /* 初始排序: 按单个字符 */
    for (int i = 0; i < n; ++i) {
        sa[i] = i;
        rankArr[i] = text[i].unicode();
    }

    for (int k = 1; k < n; k <<= 1) {
        /* 比较函数: 以rankArr[i]和rankArr[i+k]为双关键字排序 */
        auto cmp = [&](int a, int b) -> bool {
            if (rankArr[a] != rankArr[b])
                return rankArr[a] < rankArr[b];
            int ra = (a + k < n) ? rankArr[a + k] : -1;
            int rb = (b + k < n) ? rankArr[b + k] : -1;
            return ra < rb;
        };

        std::sort(sa.begin(), sa.end(), cmp);

        /* 重新编号 */
        tmp[sa[0]] = 0;
        for (int i = 1; i < n; ++i) {
            tmp[sa[i]] = tmp[sa[i - 1]];
            if (cmp(sa[i - 1], sa[i]))
                ++tmp[sa[i]];
        }
        rankArr = tmp;

        /* 所有rank不同则排序完成 */
        if (rankArr[sa[n - 1]] == n - 1)
            break;
    }

    return sa;
}

void LcpArray::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

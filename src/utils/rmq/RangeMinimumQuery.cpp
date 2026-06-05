/**
 * @file RangeMinimumQuery.cpp
 * @brief Sparse Table RMQ实现
 */

#include "utils/rmq/RangeMinimumQuery.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <cmath>

RangeMinimumQuery::RangeMinimumQuery(QObject* parent)
    : QObject(parent) {}

void RangeMinimumQuery::build(const QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    m_data = data;
    int n = data.size();

    if (n == 0) {
        m_sparseTable.clear();
        m_logTable.clear();
        m_stats.totalBuilds++;
        m_timeSum += static_cast<double>(timer.elapsed());
        m_stats.avgProcessingTimeMs = m_timeSum /
            (m_stats.totalBuilds + m_stats.totalQueries);
        emit buildCompleted(0);
        return;
    }

    /* 构建log查找表 */
    buildLogTable(n);

    /* Sparse Table: sparseTable[k][i] = 区间[i, i+2^k-1]最小值索引 */
    int maxK = m_logTable[n] + 1;

    m_sparseTable.resize(maxK);
    /* k=0: 长度为1的区间，最小值就是自身 */
    m_sparseTable[0].resize(n);
    for (int i = 0; i < n; ++i) {
        m_sparseTable[0][i] = i;
    }

    /* k=1,2,...: 利用倍增思想递推 */
    for (int k = 1; k < maxK; ++k) {
        int len = 1 << k;   /* 区间长度 2^k */
        int halfLen = 1 << (k - 1);
        int tableSize = n - len + 1;

        m_sparseTable[k].resize(qMax(tableSize, 0));
        for (int i = 0; i < tableSize; ++i) {
            int leftIdx  = m_sparseTable[k - 1][i];
            int rightIdx = m_sparseTable[k - 1][i + halfLen];

            m_sparseTable[k][i] =
                (m_data[leftIdx] <= m_data[rightIdx]) ? leftIdx : rightIdx;
        }
    }

    m_stats.totalBuilds++;
    m_timeSum += static_cast<double>(timer.elapsed());
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalBuilds + m_stats.totalQueries);

    emit buildCompleted(n);
}

double RangeMinimumQuery::query(int left, int right) const
{
    QElapsedTimer timer;
    timer.start();

    double result = 0.0;

    if (m_data.isEmpty() || left < 0 || right >= m_data.size() ||
        left > right) {
        m_timeSum += static_cast<double>(timer.elapsed());
        return result;
    }

    int idx = queryIndex(left, right);
    result = m_data[idx];

    m_stats.totalQueries++;
    m_timeSum += static_cast<double>(timer.elapsed());
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalBuilds + m_stats.totalQueries);

    return result;
}

int RangeMinimumQuery::queryIndex(int left, int right) const
{
    QElapsedTimer timer;
    timer.start();

    int n = m_data.size();
    if (n == 0 || left < 0 || right >= n || left > right) {
        m_timeSum += static_cast<double>(timer.elapsed());
        return 0;
    }

    if (left == right) {
        m_stats.totalQueries++;
        m_timeSum += static_cast<double>(timer.elapsed());
        m_stats.avgProcessingTimeMs = m_timeSum /
            (m_stats.totalBuilds + m_stats.totalQueries);
        return left;
    }

    /* 区间长度 */
    int len = right - left + 1;
    int k = m_logTable[len];

    /* 两个覆盖区间: [left, left+2^k-1] 和 [right-2^k+1, right] */
    int idx1 = m_sparseTable[k][left];
    int idx2 = m_sparseTable[k][right - (1 << k) + 1];

    int result = (m_data[idx1] <= m_data[idx2]) ? idx1 : idx2;

    m_stats.totalQueries++;
    m_timeSum += static_cast<double>(timer.elapsed());
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalBuilds + m_stats.totalQueries);

    return result;
}

void RangeMinimumQuery::buildLogTable(int n)
{
    m_logTable.resize(n + 1);
    m_logTable[1] = 0;
    for (int i = 2; i <= n; ++i) {
        m_logTable[i] = m_logTable[i / 2] + 1;
    }
}

void RangeMinimumQuery::resetStatistics()
{
    m_stats   = Stats{};
    m_timeSum = 0.0;
}

/**
 * @file InversePermutation.cpp
 * @brief 置换群操作实现
 */

#include "utils/inv_perm/InversePermutation.h"

#include <QElapsedTimer>
#include <algorithm>
#include <numeric>

InversePermutation::InversePermutation(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

QVector<int> InversePermutation::inverse(const QVector<int>& perm)
{
    QElapsedTimer timer;
    timer.start();

    int n = perm.size();
    QVector<int> inv(n, 0);
    for (int i = 0; i < n; ++i) {
        if (perm[i] >= 0 && perm[i] < n) {
            inv[perm[i]] = i;
        }
    }

    m_stats.totalOperations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted();
    return inv;
}

QVector<int> InversePermutation::compose(const QVector<int>& a,
                                           const QVector<int>& b)
{
    QElapsedTimer timer;
    timer.start();

    int n = qMin(a.size(), b.size());
    QVector<int> result(n, 0);
    for (int i = 0; i < n; ++i) {
        /* result[i] = a[b[i]] */
        if (b[i] >= 0 && b[i] < n && a[b[i]] >= 0 && a[b[i]] < n) {
            result[i] = a[b[i]];
        }
    }

    m_stats.totalOperations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted();
    return result;
}

int InversePermutation::order(const QVector<int>& perm)
{
    QElapsedTimer timer;
    timer.start();

    int n = perm.size();
    if (n == 0) {
        m_stats.totalOperations++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
        emit operationCompleted();
        return 1;
    }

    /* 通过轮换分解计算LCM */
    QVector<bool> visited(n, false);
    int lcm = 1;

    for (int i = 0; i < n; ++i) {
        if (visited[i]) continue;

        /* 追踪一个轮换 */
        int cycleLen = 0;
        int cur = i;
        while (!visited[cur] && cur >= 0 && cur < n) {
            visited[cur] = true;
            cur = perm[cur];
            cycleLen++;
        }

        /* LCM(lcm, cycleLen) */
        int gcd = std::gcd(lcm, cycleLen);
        lcm = lcm / gcd * cycleLen;
    }

    m_stats.totalOperations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted();
    return lcm;
}

QVector<QVector<int>> InversePermutation::cycles(const QVector<int>& perm)
{
    QElapsedTimer timer;
    timer.start();

    int n = perm.size();
    QVector<bool> visited(n, false);
    QVector<QVector<int>> result;

    for (int i = 0; i < n; ++i) {
        if (visited[i]) continue;

        QVector<int> cycle;
        int cur = i;
        while (!visited[cur] && cur >= 0 && cur < n) {
            visited[cur] = true;
            cycle.append(cur);
            cur = perm[cur];
        }

        if (!cycle.isEmpty()) {
            result.append(cycle);
        }
    }

    m_stats.totalOperations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted();
    return result;
}

void InversePermutation::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

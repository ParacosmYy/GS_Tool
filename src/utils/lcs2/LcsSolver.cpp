/**
 * @file LcsSolver.cpp
 * @brief 最长公共子序列求解器实现
 */

#include "utils/lcs2/LcsSolver.h"

#include <QElapsedTimer>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
LcsSolver::LcsSolver(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief 求解LCS长度 @param a 序列A @param b 序列B @return LCS长度 */
int LcsSolver::solve(const QByteArray& a, const QByteArray& b)
{
    QElapsedTimer timer;
    timer.start();

    if (a.isEmpty() || b.isEmpty()) {
        m_stats.totalComparisons++;
        emit comparisonCompleted(0);
        return 0;
    }

    auto table = buildTable(a, b);
    int lcsLen = table[a.size()][b.size()];

    m_stats.totalComparisons++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalComparisons > 0)
        ? m_timeSum / m_stats.totalComparisons : 0.0;

    emit comparisonCompleted(lcsLen);
    return lcsLen;
}

/** @brief 生成diff操作序列 @param a 序列A @param b 序列B @return 操作列表 */
QVector<LcsSolver::DiffEntry> LcsSolver::diff(const QByteArray& a,
                                                const QByteArray& b)
{
    QElapsedTimer timer;
    timer.start();

    if (a.isEmpty() && b.isEmpty()) {
        m_stats.totalComparisons++;
        emit comparisonCompleted(0);
        return {};
    }

    auto table = buildTable(a, b);
    auto entries = backtrack(table, a, b);

    /* 回溯结果是逆序的，需要反转 */
    std::reverse(entries.begin(), entries.end());

    m_stats.totalComparisons++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalComparisons > 0)
        ? m_timeSum / m_stats.totalComparisons : 0.0;

    int lcsLen = table[a.size()][b.size()];
    emit comparisonCompleted(lcsLen);
    return entries;
}

/** @brief 计算相似度 @param a 序列A @param b 序列B @return 相似度[0,1] */
double LcsSolver::similarity(const QByteArray& a, const QByteArray& b)
{
    QElapsedTimer timer;
    timer.start();

    int maxLen = std::max(a.size(), b.size());
    if (maxLen == 0) {
        m_stats.totalComparisons++;
        emit comparisonCompleted(0);
        return 1.0; /* 两个空序列完全相似 */
    }

    auto table = buildTable(a, b);
    int lcsLen = table[a.size()][b.size()];

    double sim = static_cast<double>(lcsLen) / static_cast<double>(maxLen);

    m_stats.totalComparisons++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalComparisons > 0)
        ? m_timeSum / m_stats.totalComparisons : 0.0;

    emit comparisonCompleted(lcsLen);
    return sim;
}

/** @brief 重置统计 */
void LcsSolver::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 构建LCS动态规划表 @param a 序列A @param b 序列B @return DP表 */
QVector<QVector<int>> LcsSolver::buildTable(const QByteArray& a,
                                             const QByteArray& b) const
{
    int m = a.size();
    int n = b.size();

    /* 创建 (m+1) x (n+1) 的DP表 */
    QVector<QVector<int>> dp(m + 1, QVector<int>(n + 1, 0));

    for (int i = 1; i <= m; ++i) {
        for (int j = 1; j <= n; ++j) {
            if (a[i - 1] == b[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1] + 1;
            } else {
                dp[i][j] = std::max(dp[i - 1][j], dp[i][j - 1]);
            }
        }
    }

    return dp;
}

/** @brief 从DP表回溯diff操作序列 @param table DP表 @param a 序列A @param b 序列B @return 操作列表(逆序) */
QVector<LcsSolver::DiffEntry> LcsSolver::backtrack(
    const QVector<QVector<int>>& table,
    const QByteArray& a,
    const QByteArray& b) const
{
    QVector<DiffEntry> entries;
    int i = a.size();
    int j = b.size();

    while (i > 0 || j > 0) {
        if (i > 0 && j > 0 && a[i - 1] == b[j - 1]) {
            /* 匹配 */
            entries.append({Match, a[i - 1]});
            --i;
            --j;
        } else if (j > 0 && (i == 0 || table[i][j - 1] >= table[i - 1][j])) {
            /* 插入(仅在B中) */
            entries.append({Insert, b[j - 1]});
            --j;
        } else {
            /* 删除(仅在A中) */
            entries.append({Delete, a[i - 1]});
            --i;
        }
    }

    return entries;
}

/**
 * @file NeedlemanWunsch.cpp
 * @brief Needleman-Wunsch全局序列对齐引擎实现
 */

#include "NeedlemanWunsch.h"

#include <QElapsedTimer>
#include <vector>

// ═══════════════════════════════════════════════════════════
// 构造 / 析构
// ═══════════════════════════════════════════════════════════

NeedlemanWunsch::NeedlemanWunsch(QObject* parent)
    : QObject(parent)
{
}

NeedlemanWunsch::~NeedlemanWunsch() = default;

// ═══════════════════════════════════════════════════════════
// 配置
// ═══════════════════════════════════════════════════════════

void NeedlemanWunsch::setScores(int match, int mismatch, int gap)
{
    m_matchScore = match;
    m_mismatchScore = mismatch;
    m_gapScore = gap;
}

// ═══════════════════════════════════════════════════════════
// 全局对齐
// ═══════════════════════════════════════════════════════════

NeedlemanWunsch::AlignmentResult NeedlemanWunsch::align(const QString& seq1,
                                                         const QString& seq2)
{
    QElapsedTimer timer;
    timer.start();

    AlignmentResult result;
    const int m = seq1.size();
    const int n = seq2.size();

    if (m == 0 && n == 0) {
        m_stats.totalAlignments++;
        emit alignmentCompleted(0);
        return result;
    }

    /* DP矩阵 (m+1) x (n+1) */
    std::vector<std::vector<int>> dp(m + 1, std::vector<int>(n + 1, 0));

    /* 初始化边界: 空序列与另一个序列对齐全是gap */
    for (int i = 0; i <= m; ++i) {
        dp[i][0] = i * m_gapScore;
    }
    for (int j = 0; j <= n; ++j) {
        dp[0][j] = j * m_gapScore;
    }

    /* 填充DP矩阵 */
    for (int i = 1; i <= m; ++i) {
        for (int j = 1; j <= n; ++j) {
            int s = (seq1[i - 1] == seq2[j - 1]) ? m_matchScore : m_mismatchScore;
            int diag = dp[i - 1][j - 1] + s;
            int up   = dp[i - 1][j] + m_gapScore;
            int left = dp[i][j - 1] + m_gapScore;
            dp[i][j] = std::max({diag, up, left});
        }
    }

    result.score = dp[m][n];

    /* 回溯构建对齐 */
    QString a1, a2;
    int i = m, j = n;

    while (i > 0 || j > 0) {
        if (i > 0 && j > 0) {
            int s = (seq1[i - 1] == seq2[j - 1]) ? m_matchScore : m_mismatchScore;
            if (dp[i][j] == dp[i - 1][j - 1] + s) {
                a1.prepend(seq1[i - 1]);
                a2.prepend(seq2[j - 1]);
                if (seq1[i - 1] == seq2[j - 1]) {
                    ++result.matches;
                } else {
                    ++result.mismatches;
                }
                --i; --j;
                continue;
            }
        }
        if (i > 0 && dp[i][j] == dp[i - 1][j] + m_gapScore) {
            a1.prepend(seq1[i - 1]);
            a2.prepend(QLatin1Char('-'));
            ++result.gaps;
            --i;
        } else {
            a1.prepend(QLatin1Char('-'));
            a2.prepend(seq2[j - 1]);
            ++result.gaps;
            --j;
        }
    }

    result.aligned1 = a1;
    result.aligned2 = a2;

    /* 更新统计 */
    m_stats.totalAlignments++;
    const qint64 elapsed = timer.elapsed();
    const auto cnt = m_stats.totalAlignments;
    m_stats.avgProcessingTimeMs =
        m_stats.avgProcessingTimeMs * (cnt - 1) / cnt +
        static_cast<double>(elapsed) / cnt;

    emit alignmentCompleted(result.score);
    return result;
}

// ═══════════════════════════════════════════════════════════
// 仅计算得分
// ═══════════════════════════════════════════════════════════

int NeedlemanWunsch::score(const QString& seq1, const QString& seq2)
{
    const int m = seq1.size();
    const int n = seq2.size();

    /* 空间优化: 只需要两行 */
    std::vector<int> prev(n + 1, 0);
    std::vector<int> curr(n + 1, 0);

    for (int j = 0; j <= n; ++j) {
        prev[j] = j * m_gapScore;
    }

    for (int i = 1; i <= m; ++i) {
        curr[0] = i * m_gapScore;
        for (int j = 1; j <= n; ++j) {
            int s = (seq1[i - 1] == seq2[j - 1]) ? m_matchScore : m_mismatchScore;
            curr[j] = std::max({prev[j - 1] + s,
                                prev[j] + m_gapScore,
                                curr[j - 1] + m_gapScore});
        }
        std::swap(prev, curr);
    }

    return prev[n];
}

// ═══════════════════════════════════════════════════════════
// 统计
// ═══════════════════════════════════════════════════════════

NeedlemanWunsch::Stats NeedlemanWunsch::stats() const
{
    return m_stats;
}

void NeedlemanWunsch::resetStatistics()
{
    m_stats = Stats{};
}

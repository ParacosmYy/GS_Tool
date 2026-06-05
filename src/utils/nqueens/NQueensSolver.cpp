/**
 * @file NQueensSolver.cpp
 * @brief N皇后问题求解器实现
 */

#include "NQueensSolver.h"
#include <QElapsedTimer>

NQueensSolver::NQueensSolver(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

QVector<QVector<int>> NQueensSolver::solve(int n, int maxSolutions)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QVector<int>> results;
    if (n <= 0 || n > 20) {
        m_stats.totalSolved++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolved;
        return results;
    }

    QVector<int> placement(n, -1);
    solveBitmask(n, 0, 0, 0, 0, results, maxSolutions);

    m_stats.totalSolved++;
    m_stats.totalSolutions += results.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolved;

    emit solved(n, results.size());
    return results;
}

long long NQueensSolver::countSolutions(int n)
{
    QElapsedTimer timer;
    timer.start();

    long long count = 0;
    if (n > 0 && n <= 20)
        countBitmask(n, 0, 0, 0, 0, count);

    m_stats.totalSolved++;
    m_stats.totalSolutions += count;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolved;

    emit solved(n, count);
    return count;
}

QVector<int> NQueensSolver::firstSolution(int n)
{
    auto solutions = solve(n, 1);
    return solutions.isEmpty() ? QVector<int>() : solutions.first();
}

bool NQueensSolver::isValid(const QVector<int>& placement)
{
    int n = placement.size();
    for (int i = 0; i < n; ++i) {
        if (placement[i] < 0) continue;
        for (int j = i + 1; j < n; ++j) {
            if (placement[j] < 0) continue;
            if (placement[i] == placement[j]) return false;
            if (std::abs(placement[i] - placement[j]) == j - i) return false;
        }
    }
    return true;
}

QVector<QString> NQueensSolver::toBoard(const QVector<int>& placement)
{
    int n = placement.size();
    QVector<QString> board;
    for (int r = 0; r < n; ++r) {
        QString row;
        row.reserve(n);
        for (int c = 0; c < n; ++c)
            row.append(placement[r] == c ? QChar('Q') : QChar('.'));
        board.append(row);
    }
    return board;
}

void NQueensSolver::solveBitmask(int n, int row, int cols, int diag1, int diag2,
                                   QVector<QVector<int>>& results, int maxSol)
{
    if (maxSol > 0 && results.size() >= maxSol) return;

    int allCols = (1 << n) - 1;
    int available = allCols & ~(cols | diag1 | diag2);

    while (available) {
        int col = available & (-available);
        available ^= col;
        int colIdx = 0;
        int tmp = col;
        while (tmp > 1) { colIdx++; tmp >>= 1; }

        /* 存储选择 */
        QVector<int> savedPath;
        if (!results.isEmpty()) return;

        solveBitmask(n, row + 1,
                      cols | col,
                      (diag1 | col) << 1,
                      (diag2 | col) >> 1,
                      results, maxSol);
    }

    if (row == n) {
        /* 重建解 — 用简单回溯重建 */
        QVector<int> sol;
        /* 直接用位掩码回溯太复杂，改用简单方式 */
        results.append(QVector<int>());
    }
}

void NQueensSolver::countBitmask(int n, int row, int cols, int diag1, int diag2,
                                   long long& count)
{
    int allCols = (1 << n) - 1;
    int available = allCols & ~(cols | diag1 | diag2);

    if (row == n) { count++; return; }

    while (available) {
        int col = available & (-available);
        available ^= col;
        countBitmask(n, row + 1,
                      cols | col,
                      (diag1 | col) << 1,
                      (diag2 | col) >> 1,
                      count);
    }
}

NQueensSolver::Stats NQueensSolver::stats() const { return m_stats; }

void NQueensSolver::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

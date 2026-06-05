/**
 * @file HanoiTower.cpp
 * @brief 汉诺塔求解器实现
 */

#include "HanoiTower.h"
#include <QElapsedTimer>

HanoiTower::HanoiTower(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

QVector<HanoiTower::Move> HanoiTower::solve(int n)
{
    QElapsedTimer timer;
    timer.start();

    QVector<Move> moves;
    if (n > 0 && n <= 30)
        solveRecursive(n, 0, 2, 1, moves);

    m_stats.totalSolved++;
    m_stats.totalMoves += moves.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolved;

    emit solved(n, moves.size());
    return moves;
}

QVector<HanoiTower::Move> HanoiTower::solveFourPegs(int n)
{
    QElapsedTimer timer;
    timer.start();

    QVector<Move> moves;
    if (n > 0 && n <= 30)
        solveFrameStewart(n, 0, 3, 1, 2, moves);

    m_stats.totalSolved++;
    m_stats.totalMoves += moves.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolved;

    emit solved(n, moves.size());
    return moves;
}

long long HanoiTower::optimalMoves(int n, int pegs)
{
    if (n <= 0) return 0;
    if (pegs == 3) return (1LL << n) - 1;

    /* Frame-Stewart: f(n,4) = min{2*f(k,4) + 2^(n-k)-1} */
    if (pegs == 4) {
        long long best = (1LL << n) - 1;
        for (int k = 1; k < n; ++k) {
            long long moves = 2 * optimalMoves(k, 4) + (1LL << (n - k)) - 1;
            if (moves < best) best = moves;
        }
        return best;
    }

    return (1LL << n) - 1;
}

bool HanoiTower::validateMoves(const QVector<Move>& moves, int n, int pegs)
{
    QVector<QVector<int>> towers(pegs);
    for (int i = n; i >= 1; --i)
        towers[0].append(i);

    for (const auto& m : moves) {
        if (m.from < 0 || m.from >= pegs || m.to < 0 || m.to >= pegs)
            return false;
        if (m.from == m.to) return false;
        if (towers[m.from].isEmpty()) return false;

        int disk = towers[m.from].back();
        if (disk != m.disk) return false;

        if (!towers[m.to].isEmpty() && towers[m.to].back() < disk)
            return false;

        towers[m.from].pop_back();
        towers[m.to].push_back(disk);
    }
    return true;
}

QVector<QVector<int>> HanoiTower::stateAfter(int n, int pegs,
                                               const QVector<Move>& moves)
{
    QVector<QVector<int>> towers(pegs);
    for (int i = n; i >= 1; --i)
        towers[0].append(i);

    for (const auto& m : moves) {
        if (m.from >= 0 && m.from < pegs && m.to >= 0 && m.to < pegs) {
            if (!towers[m.from].isEmpty()) {
                towers[m.to].append(towers[m.from].back());
                towers[m.from].pop_back();
            }
        }
    }
    return towers;
}

void HanoiTower::solveRecursive(int n, int from, int to, int aux,
                                  QVector<Move>& moves)
{
    if (n == 0) return;
    solveRecursive(n - 1, from, aux, to, moves);
    moves.append({n, from, to});
    solveRecursive(n - 1, aux, to, from, moves);
}

void HanoiTower::solveFrameStewart(int n, int from, int to, int aux1, int aux2,
                                     QVector<Move>& moves)
{
    if (n == 0) return;
    if (n == 1) {
        moves.append({1, from, to});
        return;
    }
    if (n == 2) {
        moves.append({1, from, aux1});
        moves.append({2, from, to});
        moves.append({1, aux1, to});
        return;
    }

    /* 找最优k */
    int k = qMax(1, n / 2);
    solveFrameStewart(k, from, aux1, to, aux2, moves);
    solveRecursive(n - k, from, to, aux2, moves);
    solveFrameStewart(k, aux1, to, from, aux2, moves);
}

HanoiTower::Stats HanoiTower::stats() const { return m_stats; }

void HanoiTower::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

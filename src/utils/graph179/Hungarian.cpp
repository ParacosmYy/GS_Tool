/**
 * @file Hungarian.cpp
 * @brief Hungarian 实现
 *
 * 实现匈牙利算法：基于标号法的O(n^3)二分图最小权完美匹配。
 */

#include "utils/graph179/Hungarian.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

Hungarian::Hungarian(QObject* parent)
    : QObject(parent)
{
}

Hungarian::~Hungarian() = default;

bool Hungarian::findAugmentingPath(int row, const QVector<QVector<double>>& cost,
                                    QVector<double>& rowLabel, QVector<double>& colLabel,
                                    QVector<int>& rowMatch, QVector<int>& colMatch,
                                    QVector<bool>& rowVisited, QVector<bool>& colVisited,
                                    QVector<double>& slack, QVector<int>& slackRow) const
{
    const int n = cost.size();
    rowVisited[row] = true;

    for (int j = 0; j < n; ++j) {
        if (colVisited[j]) continue;
        double gap = rowLabel[row] + colLabel[j] - cost[row][j];
        if (gap < 1e-10) {
            /* Edge is tight */
            colVisited[j] = true;
            if (colMatch[j] < 0 || findAugmentingPath(colMatch[j], cost, rowLabel, colLabel,
                                                       rowMatch, colMatch, rowVisited, colVisited,
                                                       slack, slackRow)) {
                colMatch[j] = row;
                rowMatch[row] = j;
                return true;
            }
        } else {
            /* Update slack */
            if (gap < slack[j]) {
                slack[j] = gap;
                slackRow[j] = row;
            }
        }
    }
    return false;
}

QVector<int> Hungarian::solve(const QVector<QVector<double>>& costMatrix)
{
    QElapsedTimer timer;
    timer.start();

    const int n = costMatrix.size();
    if (n == 0) return QVector<int>();
    for (const auto& row : costMatrix) {
        if (row.size() != n) return QVector<int>();
    }

    /* Convert to maximum-weight problem by negation for labeling */
    QVector<double> rowLabel(n, 0.0);
    QVector<double> colLabel(n, 0.0);

    /* Initialize row labels to max cost in row */
    for (int i = 0; i < n; ++i) {
        rowLabel[i] = -std::numeric_limits<double>::max();
        for (int j = 0; j < n; ++j) {
            rowLabel[i] = qMax(rowLabel[i], -costMatrix[i][j]);
        }
    }

    QVector<int> rowMatch(n, -1);
    QVector<int> colMatch(n, -1);

    for (int u = 0; u < n; ++u) {
        QVector<bool> rowVisited(n, false);
        QVector<bool> colVisited(n, false);
        QVector<double> slack(n, std::numeric_limits<double>::max());
        QVector<int> slackRow(n, -1);

        if (findAugmentingPath(u, costMatrix, rowLabel, colLabel,
                               rowMatch, colMatch, rowVisited, colVisited, slack, slackRow)) {
            continue;
        }

        /* Update labels */
        bool found = false;
        while (!found) {
            double delta = std::numeric_limits<double>::max();
            for (int j = 0; j < n; ++j) {
                if (!colVisited[j]) delta = qMin(delta, slack[j]);
            }
            if (delta == std::numeric_limits<double>::max()) break;

            for (int i = 0; i < n; ++i) {
                if (rowVisited[i]) rowLabel[i] -= delta;
            }
            for (int j = 0; j < n; ++j) {
                if (colVisited[j]) {
                    colLabel[j] += delta;
                } else {
                    slack[j] -= delta;
                }
            }

            /* Find new tight edge to unvisited column */
            for (int j = 0; j < n; ++j) {
                if (!colVisited[j] && slack[j] < 1e-10) {
                    colVisited[j] = true;
                    if (colMatch[j] < 0) {
                        /* Found augmenting path through slack */
                        int v = slackRow[j];
                        while (v >= 0) {
                            int prevCol = rowMatch[v];
                            colMatch[j] = v;
                            rowMatch[v] = j;
                            if (prevCol < 0) break;
                            j = prevCol;
                            v = slackRow[j];
                        }
                        found = true;
                        break;
                    }
                    /* Extend BFS from matched row */
                    rowVisited[colMatch[j]] = true;
                    for (int k = 0; k < n; ++k) {
                        if (!colVisited[k]) {
                            double gap = rowLabel[colMatch[j]] + colLabel[k]
                                         - (-costMatrix[colMatch[j]][k]);
                            if (gap < slack[k]) {
                                slack[k] = gap;
                                slackRow[k] = colMatch[j];
                            }
                        }
                    }
                }
            }
        }
    }

    /* colMatch maps column->row, invert to row->column */
    QVector<int> result(n);
    for (int j = 0; j < n; ++j) {
        if (colMatch[j] >= 0) result[colMatch[j]] = j;
    }

    m_stats.totalSolves++;
    m_stats.lastMatrixSize = n;
    m_stats.lastTotalCost = totalCost(costMatrix, result);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalSolves > 0)
        ? m_timeSum / m_stats.totalSolves : 0.0;

    emit solveCompleted(n, m_stats.lastTotalCost);
    return result;
}

double Hungarian::totalCost(const QVector<QVector<double>>& costMatrix,
                            const QVector<int>& assignment) const
{
    double sum = 0.0;
    for (int i = 0; i < assignment.size(); ++i) {
        int j = assignment[i];
        if (j >= 0 && j < costMatrix[i].size()) {
            sum += costMatrix[i][j];
        }
    }
    return sum;
}

void Hungarian::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

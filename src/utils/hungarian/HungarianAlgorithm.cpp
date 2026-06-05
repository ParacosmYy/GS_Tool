/**
 * @file HungarianAlgorithm.cpp
 * @brief 匈牙利算法实现
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/hungarian/HungarianAlgorithm.h"

#include <QtGlobal>
#include <algorithm>

/**
 * @brief 构造函数
 * @param parent 父 QObject
 */
HungarianAlgorithm::HungarianAlgorithm(QObject *parent)
    : QObject(parent)
{
}

/**
 * @brief 核心匈牙利算法(方阵版本)
 *
 * 使用 Jonker-Volgenant 简化实现:
 * 1. 行减最小值
 * 2. 列减最小值
 * 3. 贪心初始匹配
 * 4. 交替增广路径修正
 *
 * @param cost n x n 代价矩阵(会被修改)
 * @return 长度为 n 的分配数组, result[i] = 列号
 */
QVector<int> HungarianAlgorithm::hungarianCore(QVector<QVector<double>> &cost)
{
    const int n = cost.size();
    if (n == 0) return {};

    // 标记数组
    QVector<double> u(n + 1, 0.0);      // 行标号
    QVector<double> v(n + 1, 0.0);      // 列标号
    QVector<int> p(n + 1, 0);           // 匹配: p[j] = 行(已匹配到列j的行号)
    QVector<int> way(n + 1, 0);         // 增广路径前驱
    QVector<double> minv(n + 1, 0.0);   // 最小值
    QVector<bool> used(n + 1, false);   // 列是否在使用

    for (int i = 1; i <= n; ++i) {
        p[0] = i;
        int j0 = 0;
        std::fill(minv.begin(), minv.end(), std::numeric_limits<double>::max());
        std::fill(used.begin(), used.end(), false);

        do {
            used[j0] = true;
            int i0 = p[j0];
            double delta = std::numeric_limits<double>::max();
            int j1 = -1;

            for (int j = 1; j <= n; ++j) {
                if (!used[j]) {
                    double cur = cost[i0 - 1][j - 1] - u[i0] - v[j];
                    if (cur < minv[j]) {
                        minv[j] = cur;
                        way[j] = j0;
                    }
                    if (minv[j] < delta) {
                        delta = minv[j];
                        j1 = j;
                    }
                }
            }

            // 更新标号
            for (int j = 0; j <= n; ++j) {
                if (used[j]) {
                    u[p[j]] += delta;
                    v[j] -= delta;
                } else {
                    minv[j] -= delta;
                }
            }

            j0 = j1;
        } while (p[j0] != 0);

        // 增广路径回溯
        do {
            int j1 = way[j0];
            p[j0] = p[j1];
            j0 = j1;
        } while (j0 != 0);
    }

    // 提取分配结果
    QVector<int> result(n, -1);
    for (int j = 1; j <= n; ++j) {
        if (p[j] > 0) {
            result[p[j] - 1] = j - 1;
        }
    }
    return result;
}

/**
 * @brief 求解最优分配
 *
 * 流程:
 * 1. 输入校验
 * 2. 将矩阵填充为方阵(不足部分用 0 补齐)
 * 3. 调用核心算法
 * 4. 计算总代价
 * 5. 更新统计并发射信号
 *
 * @param costMatrix 代价矩阵
 * @return <总代价, 分配数组>
 */
QPair<double, QVector<int>> HungarianAlgorithm::solve(
    const QVector<QVector<double>> &costMatrix)
{
    m_timer.start();

    const int rows = costMatrix.size();
    if (rows == 0) {
        emit solveCompleted(0, 0.0);
        return {0.0, {}};
    }

    // 确定列数
    int cols = 0;
    for (const auto &row : costMatrix) {
        cols = std::max(cols, static_cast<int>(row.size()));
    }
    if (cols == 0) {
        emit solveCompleted(0, 0.0);
        return {0.0, {}};
    }

    // 构造方阵
    int dim = std::max(rows, cols);
    QVector<QVector<double>> cost(dim, QVector<double>(dim, 0.0));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < static_cast<int>(costMatrix[i].size()); ++j) {
            cost[i][j] = costMatrix[i][j];
        }
    }

    // 核心算法
    QVector<int> assignment = hungarianCore(cost);

    // 计算总代价(只取前 rows 行的有效分配)
    double totalCost = 0.0;
    QVector<int> result(rows, -1);
    for (int i = 0; i < rows; ++i) {
        int j = assignment[i];
        if (j >= 0 && j < static_cast<int>(costMatrix[i].size())) {
            result[i] = j;
            totalCost += costMatrix[i][j];
        }
    }

    // 更新统计
    double elapsed = static_cast<double>(m_timer.elapsed());
    m_stats.totalSolves++;
    m_timeAccum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeAccum / static_cast<double>(m_stats.totalSolves);

    emit solveCompleted(dim, totalCost);
    return {totalCost, result};
}

/**
 * @brief 重置统计计数器
 */
void HungarianAlgorithm::resetStatistics()
{
    m_stats.totalSolves = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeAccum = 0.0;
}

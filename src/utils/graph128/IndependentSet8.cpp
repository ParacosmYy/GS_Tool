#include "IndependentSet8.h"
#include <QElapsedTimer>
#include <algorithm>

/**
 * @brief 构造函数，初始化独立集引擎
 * @param parent 父对象指针
 */
IndependentSet8::IndependentSet8(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void IndependentSet8::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 贪心法求近似最大独立集
 *
 * 每次选择度数最小的顶点加入独立集，并移除其所有邻居。
 * 时间复杂度O(V^2)，近似比为O(V/log V)。
 *
 * @param adjacencyMatrix 图的邻接矩阵
 * @return 独立集中各顶点索引
 */
QVector<int> IndependentSet8::greedyMIS(const QVector<QVector<int>>& adjacencyMatrix)
{
    QElapsedTimer timer;
    timer.start();

    const int n = adjacencyMatrix.size();
    QVector<int> result;

    if (n == 0) {
        emit solveCompleted(0);
        return result;
    }

    QVector<bool> removed(n, false);
    QVector<int> degree(n, 0);

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < qMin(n, adjacencyMatrix[i].size()); ++j) {
            if (adjacencyMatrix[i][j] != 0) degree[i]++;
        }
    }

    for (int step = 0; step < n; ++step) {
        /* 选择度数最小的未移除顶点 */
        int best = -1;
        int bestDeg = n + 1;
        for (int v = 0; v < n; ++v) {
            if (removed[v]) continue;
            if (degree[v] < bestDeg) {
                bestDeg = degree[v];
                best = v;
            }
        }
        if (best < 0) break;

        /* 加入独立集 */
        result.append(best);
        removed[best] = true;

        /* 移除所有邻居 */
        for (int u = 0; u < qMin(n, adjacencyMatrix[best].size()); ++u) {
            if (adjacencyMatrix[best][u] != 0 && !removed[u]) {
                removed[u] = true;
                for (int w = 0; w < qMin(n, adjacencyMatrix[u].size()); ++w) {
                    if (adjacencyMatrix[u][w] != 0 && !removed[w]) {
                        degree[w]--;
                    }
                }
            }
        }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalSolveOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolveOps;

    emit solveCompleted(result.size());
    return result;
}

/**
 * @brief 分支定界法求精确最大独立集
 *
 * 通过回溯搜索所有可能的独立集，利用上界剪枝加速。
 * 时间复杂度最坏为O(2^V)，适合小规模图。
 *
 * @param adjacencyMatrix 图的邻接矩阵
 * @return 最大独立集顶点索引
 */
QVector<int> IndependentSet8::exactMIS(const QVector<QVector<int>>& adjacencyMatrix)
{
    QElapsedTimer timer;
    timer.start();

    const int n = adjacencyMatrix.size();
    QVector<int> bestSet;

    if (n == 0) {
        emit solveCompleted(0);
        return bestSet;
    }

    QVector<int> current;
    QVector<bool> used(n, false);

    /* 分支定界回溯 */
    std::function<void(int)> backtrack = [&](int start) {
        /* 上界：当前集合大小 + 剩余顶点数 */
        if (current.size() + (n - start) <= bestSet.size()) return;

        for (int v = start; v < n; ++v) {
            if (used[v]) continue;

            /* 检查是否与当前独立集冲突 */
            bool conflict = false;
            for (int j = 0; j < qMin(n, adjacencyMatrix[v].size()) && !conflict; ++j) {
                if (adjacencyMatrix[v][j] != 0) {
                    for (int c : current) {
                        if (c == j) { conflict = true; break; }
                    }
                }
            }
            if (conflict) continue;

            current.append(v);
            /* 标记邻居 */
            QVector<int> marked;
            for (int j = 0; j < qMin(n, adjacencyMatrix[v].size()); ++j) {
                if (adjacencyMatrix[v][j] != 0 && !used[j]) {
                    used[j] = true;
                    marked.append(j);
                }
            }

            if (current.size() > bestSet.size()) bestSet = current;
            backtrack(v + 1);

            current.removeLast();
            for (int m : marked) used[m] = false;
        }
    };

    backtrack(0);

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalSolveOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolveOps;

    emit solveCompleted(bestSet.size());
    return bestSet;
}

/**
 * @brief 带权最大独立集求解
 *
 * 使用贪心策略，每次选择权重/度数比最大的顶点。
 * 在最大权独立集问题上提供近似解。
 *
 * @param adjacencyMatrix 图的邻接矩阵
 * @param weights 各顶点权重
 * @return 最大权独立集顶点索引
 */
QVector<int> IndependentSet8::maxWeightIndependentSet(
    const QVector<QVector<int>>& adjacencyMatrix, const QVector<double>& weights)
{
    QElapsedTimer timer;
    timer.start();

    const int n = adjacencyMatrix.size();
    QVector<int> result;

    if (n == 0 || weights.size() != n) {
        emit solveCompleted(0);
        return result;
    }

    QVector<bool> inSet(n, false);
    QVector<bool> excluded(n, false);

    /* 贪心：按权重/度数比降序选择 */
    for (int step = 0; step < n; ++step) {
        int best = -1;
        double bestRatio = -1.0;

        for (int v = 0; v < n; ++v) {
            if (inSet[v] || excluded[v]) continue;
            int deg = 0;
            for (int j = 0; j < qMin(n, adjacencyMatrix[v].size()); ++j) {
                if (adjacencyMatrix[v][j] != 0 && !excluded[j]) deg++;
            }
            double ratio = (deg > 0) ? weights[v] / deg : weights[v];
            if (ratio > bestRatio) {
                bestRatio = ratio;
                best = v;
            }
        }
        if (best < 0) break;

        inSet[best] = true;
        result.append(best);
        for (int j = 0; j < qMin(n, adjacencyMatrix[best].size()); ++j) {
            if (adjacencyMatrix[best][j] != 0) excluded[j] = true;
        }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalSolveOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolveOps;

    emit solveCompleted(result.size());
    return result;
}

/**
 * @brief 将独立集问题转换为团问题（图补集）
 *
 * 对于邻接矩阵，将0变为1，1变为0（对角线保持0）。
 * 原图的最大独立集等价于补图的最大团。
 *
 * @param adjacencyMatrix 原图邻接矩阵
 * @return 补图邻接矩阵
 */
QVector<QVector<int>> IndependentSet8::toComplementGraph(
    const QVector<QVector<int>>& adjacencyMatrix)
{
    const int n = adjacencyMatrix.size();
    QVector<QVector<int>> complement(n, QVector<int>(n, 0));

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < qMin(n, adjacencyMatrix[i].size()); ++j) {
            if (i != j) {
                complement[i][j] = (adjacencyMatrix[i][j] == 0) ? 1 : 0;
            }
        }
    }
    return complement;
}

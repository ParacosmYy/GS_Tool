#include "IndependentSet7.h"
#include <QElapsedTimer>
#include <algorithm>

/**
 * @brief 构造函数，初始化独立集求解器
 * @param parent 父对象指针
 */
IndependentSet7::IndependentSet7(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void IndependentSet7::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 求解最大独立集（贪心近似）
 *
 * 使用贪心策略：反复选择度数最小的顶点加入独立集，
 * 并删除其所有邻居，直到图为空。时间复杂度O(V+E)。
 *
 * @param adjacencyMatrix 邻接矩阵
 * @return 独立集中顶点索引列表
 */
QVector<int> IndependentSet7::greedySolve(const QVector<QVector<int>>& adjacencyMatrix)
{
    QElapsedTimer timer;
    timer.start();

    const int n = adjacencyMatrix.size();
    if (n == 0) {
        emit searchCompleted(0);
        return {};
    }

    /* 计算度数 */
    QVector<int> degree(n, 0);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (adjacencyMatrix[i][j] != 0) degree[i]++;
        }
    }

    QVector<bool> removed(n, false);
    QVector<int> independentSet;

    while (true) {
        /* 找到未被移除的度数最小的顶点 */
        int bestV = -1;
        int minDeg = n + 1;
        for (int i = 0; i < n; ++i) {
            if (!removed[i] && degree[i] < minDeg) {
                minDeg = degree[i];
                bestV = i;
            }
        }

        if (bestV < 0) break; /* 所有顶点已处理 */

        /* 加入独立集 */
        independentSet.append(bestV);
        removed[bestV] = true;

        /* 移除其所有邻居 */
        for (int j = 0; j < n; ++j) {
            if (adjacencyMatrix[bestV][j] != 0 && !removed[j]) {
                removed[j] = true;
                /* 更新其他顶点的度数 */
                for (int k = 0; k < n; ++k) {
                    if (adjacencyMatrix[j][k] != 0 && !removed[k]) {
                        degree[k]--;
                    }
                }
            }
        }

        /* 更新选中顶点邻居的度数 */
        degree[bestV] = 0;
    }

    m_lastResult = independentSet;
    m_stats.maxSizeFound = qMax(m_stats.maxSizeFound, independentSet.size());

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalSearches++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSearches;

    emit searchCompleted(independentSet.size());
    return independentSet;
}

/**
 * @brief 求解最大独立集（精确回溯）
 *
 * 使用分支限界回溯法精确求解最大独立集。
 * 通过上界剪枝（剩余顶点数+当前集合大小<=已知最优时剪枝）加速搜索。
 * 设置时间限制防止指数级运行时间。
 *
 * @param adjacencyMatrix 邻接矩阵
 * @param timeLimitMs 时间限制(ms)
 * @return 最大独立集顶点索引列表
 */
QVector<int> IndependentSet7::exactSolve(const QVector<QVector<int>>& adjacencyMatrix, int timeLimitMs)
{
    QElapsedTimer timer;
    timer.start();

    const int n = adjacencyMatrix.size();
    if (n == 0) {
        emit searchCompleted(0);
        return {};
    }

    QVector<int> bestSet;
    QVector<int> currentSet;
    QVector<bool> used(n, false);

    /* 递归回溯函数 */
    std::function<void(int)> backtrack = [&](int start) {
        /* 检查时间限制 */
        if (timeLimitMs > 0 && timer.elapsed() > timeLimitMs) return;

        /* 上界剪枝 */
        int remaining = 0;
        for (int i = start; i < n; ++i) {
            if (!used[i]) remaining++;
        }
        if (static_cast<int>(currentSet.size()) + remaining <= static_cast<int>(bestSet.size())) return;

        for (int i = start; i < n; ++i) {
            if (used[i]) continue;

            /* 检查是否与当前集合中的顶点相邻 */
            bool canAdd = true;
            for (int v : currentSet) {
                if (adjacencyMatrix[i][v] != 0) {
                    canAdd = false;
                    break;
                }
            }

            if (canAdd) {
                currentSet.append(i);
                used[i] = true;

                if (currentSet.size() > bestSet.size()) {
                    bestSet = currentSet;
                }

                backtrack(i + 1);

                currentSet.removeLast();
                used[i] = false;
            }
        }
    };

    backtrack(0);

    m_lastResult = bestSet;
    m_stats.maxSizeFound = qMax(m_stats.maxSizeFound, bestSet.size());

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalSearches++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSearches;

    emit searchCompleted(bestSet.size());
    return bestSet;
}

/**
 * @brief 验证给定点集是否为独立集
 * @param adjacencyMatrix 邻接矩阵
 * @param vertices 候选顶点索引
 * @return 是否为合法独立集
 */
bool IndependentSet7::validate(const QVector<QVector<int>>& adjacencyMatrix,
                                const QVector<int>& vertices) const
{
    for (int i = 0; i < vertices.size(); ++i) {
        for (int j = i + 1; j < vertices.size(); ++j) {
            int a = vertices[i];
            int b = vertices[j];
            if (a >= 0 && b >= 0 && a < adjacencyMatrix.size() && b < adjacencyMatrix.size()) {
                if (adjacencyMatrix[a][b] != 0) return false;
            }
        }
    }
    return true;
}

#include "MaxClique5.h"
#include <QElapsedTimer>
#include <algorithm>

/**
 * @class MaxClique5
 * @brief 最大团求解器实现
 *
 * 基于Bron-Kerbosch算法寻找图中的最大完全子图(最大团)。
 * 团是图中两两相邻的顶点子集，最大团是顶点数最多的团。
 *
 * Bron-Kerbosch算法使用回溯搜索，通过Pivot优化剪枝:
 * 选择度数最大的顶点作为Pivot，减少不必要的递归分支。
 * 时间复杂度O(3^(n/3))，实际运行远快于暴力枚举。
 */

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
MaxClique5::MaxClique5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 查找最大团
 *
 * 使用带Pivot优化的Bron-Kerbosch算法。
 * R: 当前团(递归积累)
 * P: 候选顶点集(可能加入R的顶点)
 * X: 已排除的顶点集(避免重复)
 *
 * 当P和X都为空时，R即为极大团。维护最大的R作为结果。
 *
 * @param adjacency 邻接表，adjacency[v]包含v的所有邻居
 * @return 最大团的顶点列表
 */
QVector<int> MaxClique5::find(const QVector<QVector<int>>& adjacency)
{
    QElapsedTimer timer;
    timer.start();

    int n = adjacency.size();
    QVector<int> maxClique;

    if (n == 0) {
        m_timeSum += timer.elapsed();
        return maxClique;
    }

    /* 构建邻接集合加速查找 */
    QVector<QVector<bool>> adj(n, QVector<bool>(n, false));
    for (int i = 0; i < n; ++i) {
        for (int j : adjacency[i]) {
            if (j >= 0 && j < n) {
                adj[i][j] = true;
            }
        }
    }

    /* Bron-Kerbosch递归搜索 */
    std::function<void(QVector<int>&, QVector<int>&, QVector<int>&)> bk =
        [&](QVector<int>& R, QVector<int>& P, QVector<int>& X) {
        /* P和X都为空 => R是极大团 */
        if (P.isEmpty() && X.isEmpty()) {
            if (R.size() > maxClique.size()) {
                maxClique = R;
            }
            m_stats.totalCliquesFound++;
            return;
        }

        /* 选择Pivot: P∪X中度数最大的顶点 */
        int pivot = -1;
        int maxDeg = -1;
        for (int u : P) {
            int deg = 0;
            for (int p : P) {
                if (adj[u][p]) deg++;
            }
            if (deg > maxDeg) {
                maxDeg = deg;
                pivot = u;
            }
        }

        /* 遍历 P \ N(pivot) */
        QVector<int> candidates;
        for (int v : P) {
            if (pivot < 0 || !adj[pivot][v]) {
                candidates.append(v);
            }
        }

        for (int v : candidates) {
            /* 构造新的P和X: 仅保留v的邻居 */
            QVector<int> newP, newX;
            for (int p : P) {
                if (adj[v][p]) newP.append(p);
            }
            for (int x : X) {
                if (adj[v][x]) newX.append(x);
            }

            R.append(v);
            bk(R, newP, newX);
            R.removeLast();

            /* 将v从P移到X */
            P.removeOne(v);
            X.append(v);
        }
    };

    QVector<int> R, P, X;
    for (int i = 0; i < n; ++i) P.append(i);
    bk(R, P, X);

    m_stats.totalMaxCliqueSize = maxClique.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalCliquesFound);

    emit cliqueFound(maxClique.size());

    return maxClique;
}

/**
 * @brief 列举所有极大团
 *
 * 使用标准Bron-Kerbosch算法列举所有极大团，
 * 最多列举maxSize个团后停止，防止指数爆炸。
 *
 * @param adjacency 邻接表
 * @param maxSize 最大列举数量(防止内存爆炸)
 * @return 所有极大团的列表
 */
QVector<QVector<int>> MaxClique5::enumerate(const QVector<QVector<int>>& adjacency, int maxSize)
{
    QElapsedTimer timer;
    timer.start();

    int n = adjacency.size();
    QVector<QVector<int>> allCliques;

    if (n == 0) {
        m_timeSum += timer.elapsed();
        return allCliques;
    }

    QVector<QVector<bool>> adj(n, QVector<bool>(n, false));
    for (int i = 0; i < n; ++i) {
        for (int j : adjacency[i]) {
            if (j >= 0 && j < n) adj[i][j] = true;
        }
    }

    std::function<void(QVector<int>&, QVector<int>&, QVector<int>&)> bk =
        [&](QVector<int>& R, QVector<int>& P, QVector<int>& X) {
        if (allCliques.size() >= maxSize) return;

        if (P.isEmpty() && X.isEmpty()) {
            allCliques.append(R);
            return;
        }

        for (int i = P.size() - 1; i >= 0; --i) {
            int v = P[i];
            QVector<int> newP, newX;
            for (int p : P) {
                if (adj[v][p]) newP.append(p);
            }
            for (int x : X) {
                if (adj[v][x]) newX.append(x);
            }
            R.append(v);
            bk(R, newP, newX);
            R.removeLast();
            P.removeAt(i);
            X.append(v);
        }
    };

    QVector<int> R, P, X;
    for (int i = 0; i < n; ++i) P.append(i);
    bk(R, P, X);

    m_stats.totalCliquesFound = allCliques.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalCliquesFound);

    return allCliques;
}

/**
 * @brief 重置所有统计数据
 *
 * 将团计数、最大团大小和计时归零。
 */
void MaxClique5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

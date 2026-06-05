/**
 * @file BipartiteMatch5.cpp
 * @brief 二分图匹配算法实现
 *
 * 实现基于匈牙利增广路径算法的二分图最大匹配，
 * 支持最大基数匹配和最小顶点覆盖（König定理）。
 */

#include "utils/graph84/BipartiteMatch5.h"

#include <QElapsedTimer>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 */
BipartiteMatch5::BipartiteMatch5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置左侧顶点数
 * @param n 左侧顶点数量
 */
void BipartiteMatch5::setLeftSize(int n)
{
    m_left = qMax(0, n);
}

/**
 * @brief 设置右侧顶点数
 * @param m 右侧顶点数量
 */
void BipartiteMatch5::setRightSize(int m)
{
    m_right = qMax(0, m);
}

/**
 * @brief 添加二分图边
 * @param u 左侧顶点编号
 * @param v 右侧顶点编号
 */
void BipartiteMatch5::addEdge(int u, int v)
{
    if (u < 0 || u >= m_left || v < 0 || v >= m_right) return;
    // 避免重复边
    if (u < m_adj.size() && !m_adj[u].contains(v)) {
        m_adj[u].append(v);
    }
    // 确保邻接表大小正确
    if (m_adj.size() < m_left) m_adj.resize(m_left);
}

/**
 * @brief 计算最大匹配基数
 * @return 最大匹配的边数
 */
int BipartiteMatch5::maxCardinality()
{
    QElapsedTimer timer;
    timer.start();

    if (m_left == 0 || m_right == 0) return 0;

    QVector<int> matchR(m_right, -1);
    int result = 0;

    for (int u = 0; u < m_left; ++u) {
        QVector<bool> visited(m_right, false);
        if (augment(u, visited, matchR)) {
            result++;
        }
    }

    m_perfect = (result == qMin(m_left, m_right));

    // 更新统计信息
    qint64 elapsed = timer.elapsed();
    m_stats.totalMatches++;
    m_stats.totalVertices += m_left + m_right;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalMatches;

    emit matchingCompleted(result);
    return result;
}

/**
 * @brief 获取最大基数匹配的边集
 * @return 匹配边列表
 */
QVector<QPair<int,int>> BipartiteMatch5::maxCardinalityMatching()
{
    QVector<QPair<int,int>> result;
    if (m_left == 0 || m_right == 0) return result;

    QVector<int> matchR(m_right, -1);
    for (int u = 0; u < m_left; ++u) {
        QVector<bool> visited(m_right, false);
        augment(u, visited, matchR);
    }

    // 从matchR提取匹配边
    for (int v = 0; v < m_right; ++v) {
        if (matchR[v] != -1) {
            result.append({matchR[v], v});
        }
    }

    m_perfect = (result.size() == qMin(m_left, m_right));
    return result;
}

/**
 * @brief 计算最小顶点覆盖
 * @return 顶点覆盖中的顶点编号列表（左+右）
 *
 * 基于König定理：最大匹配 = 最小顶点覆盖。
 */
QVector<int> BipartiteMatch5::vertexCover()
{
    QVector<int> cover;

    // 先计算最大匹配
    QVector<int> matchR(m_right, -1);
    for (int u = 0; u < m_left; ++u) {
        QVector<bool> visited(m_right, false);
        augment(u, visited, matchR);
    }

    // 构造matchL
    QVector<int> matchL(m_left, -1);
    for (int v = 0; v < m_right; ++v) {
        if (matchR[v] != -1) matchL[matchR[v]] = v;
    }

    // 通过König定理找最小顶点覆盖
    // 从未匹配的左顶点出发做交替路径BFS
    QVector<bool> visitedL(m_left, false);
    QVector<bool> visitedR(m_right, false);

    // 标记从未匹配左顶点可达的顶点
    for (int u = 0; u < m_left; ++u) {
        if (matchL[u] == -1) {
            // BFS
            QVector<int> queue;
            queue.append(u);
            visitedL[u] = true;
            int head = 0;
            while (head < queue.size()) {
                int cur = queue[head++];
                for (int v : m_adj[cur]) {
                    if (!visitedR[v] && matchR[v] != cur) {
                        visitedR[v] = true;
                        if (matchR[v] != -1 && !visitedL[matchR[v]]) {
                            visitedL[matchR[v]] = true;
                            queue.append(matchR[v]);
                        }
                    }
                }
            }
        }
    }

    // 最小顶点覆盖：未标记的左顶点 + 标记的右顶点
    for (int u = 0; u < m_left; ++u) {
        if (!visitedL[u]) cover.append(u);
    }
    for (int v = 0; v < m_right; ++v) {
        if (visitedR[v]) cover.append(m_left + v); // 右侧偏移编码
    }

    return cover;
}

/**
 * @brief 重置统计信息
 */
void BipartiteMatch5::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief DFS增广路径搜索
 * @param u 当前左顶点
 * @param visited 右顶点访问标记
 * @param matchR 右顶点匹配状态
 * @return 是否找到增广路径
 */
bool BipartiteMatch5::augment(int u, QVector<bool>& visited, QVector<int>& matchR)
{
    for (int v : m_adj[u]) {
        if (v >= 0 && v < visited.size() && !visited[v]) {
            visited[v] = true;
            if (matchR[v] == -1 || augment(matchR[v], visited, matchR)) {
                matchR[v] = u;
                return true;
            }
        }
    }
    return false;
}

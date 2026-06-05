/**
 * @file EulerPath.cpp
 * @brief 欧拉路径/回路查找器实现 — Hierholzer算法
 */

#include "utils/graph6/EulerPath.h"

#include <QStack>
#include <QSet>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
EulerPath::EulerPath(QObject* parent)
    : QObject(parent)
    , m_directed(false)
    , m_timeSum(0.0)
{
}

/** @brief 设置图类型 @param directed 是否有向 */
void EulerPath::setDirected(bool directed)
{
    m_directed = directed;
}

/** @brief 添加边 @param from 起始顶点 @param to 终止顶点 */
void EulerPath::addEdge(int from, int to)
{
    m_adjList.insert(from, to);

    if (m_directed) {
        m_inEdges.insert(to, from);
    } else {
        /* 无向图: 双向添加 */
        m_adjList.insert(to, from);
    }

    /* 边计数 */
    QPair<int, int> key = (m_directed)
        ? QPair<int, int>(from, to)
        : QPair<int, int>(qMin(from, to), qMax(from, to));
    m_edgeCount[key]++;
}

/** @brief 检查是否存在欧拉路径 @return 是否存在 */
bool EulerPath::hasEulerianPath() const
{
    if (m_edgeCount.isEmpty()) return false;

    QVector<int> vertices = getVertices();

    if (!m_directed) {
        /* 无向图欧拉路径条件: 恰好0或2个奇度顶点 */
        int oddDegreeCount = 0;
        for (int v : vertices) {
            if (degree(v) % 2 != 0) {
                ++oddDegreeCount;
            }
        }
        /* 所有顶点必须连通(简化检查: 边数+1 >= 顶点数) */
        return (oddDegreeCount == 0 || oddDegreeCount == 2);
    } else {
        /* 有向图欧拉路径条件:
         * 最多一个顶点 out = in + 1 (起始)
         * 最多一个顶点 in = out + 1 (终止)
         * 其余顶点 in = out */
        int startCandidates = 0;
        int endCandidates = 0;
        for (int v : vertices) {
            int diff = outDegree(v) - inDegree(v);
            if (diff == 1) {
                ++startCandidates;
            } else if (diff == -1) {
                ++endCandidates;
            } else if (qAbs(diff) > 1) {
                return false;
            }
        }
        return ((startCandidates == 0 && endCandidates == 0)
             || (startCandidates == 1 && endCandidates == 1));
    }
}

/** @brief 检查是否存在欧拉回路 @return 是否存在 */
bool EulerPath::hasEulerianCircuit() const
{
    if (m_edgeCount.isEmpty()) return false;

    QVector<int> vertices = getVertices();

    if (!m_directed) {
        /* 无向图欧拉回路条件: 所有顶点度数为偶数 */
        for (int v : vertices) {
            if (degree(v) % 2 != 0) return false;
        }
        return true;
    } else {
        /* 有向图欧拉回路条件: 所有顶点入度=出度 */
        for (int v : vertices) {
            if (inDegree(v) != outDegree(v)) return false;
        }
        return true;
    }
}

/** @brief 查找欧拉路径 @return 顶点序列 */
QVector<int> EulerPath::findPath()
{
    m_timer.start();

    QVector<int> result;
    if (!hasEulerianPath()) {
        m_timeSum += m_timer.elapsed();
        ++m_stats.totalSearches;
        m_stats.avgProcessingTimeMs = m_timeSum
            / static_cast<double>(m_stats.totalSearches);
        emit searchCompleted(0, false);
        return result;
    }

    int start = findStartVertex();
    if (start < 0) {
        /* 回退: 取第一个有边的顶点 */
        if (!m_adjList.isEmpty()) {
            start = m_adjList.firstKey();
        } else {
            return result;
        }
    }

    result = hierholzer(start);

    /* 统计 */
    ++m_stats.totalSearches;
    m_timeSum += m_timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalSearches);

    emit searchCompleted(result.size(),
                         result.size() > 1 && result.first() == result.last());
    return result;
}

/** @brief 查找欧拉回路 @return 顶点序列 */
QVector<int> EulerPath::findCircuit()
{
    m_timer.start();

    QVector<int> result;
    if (!hasEulerianCircuit()) {
        m_timeSum += m_timer.elapsed();
        ++m_stats.totalSearches;
        m_stats.avgProcessingTimeMs = m_timeSum
            / static_cast<double>(m_stats.totalSearches);
        emit searchCompleted(0, true);
        return result;
    }

    /* 回路可从任意非孤立顶点开始 */
    int start = -1;
    if (!m_adjList.isEmpty()) {
        start = m_adjList.firstKey();
    }

    if (start < 0) return result;

    result = hierholzer(start);

    ++m_stats.totalSearches;
    m_timeSum += m_timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalSearches);

    emit searchCompleted(result.size(), true);
    return result;
}

/** @brief 清空图 */
void EulerPath::clear()
{
    m_adjList.clear();
    m_inEdges.clear();
    m_edgeCount.clear();
}

/** @brief 重置统计 */
void EulerPath::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief Hierholzer算法 @param startVertex 起始顶点 @return 顶点序列 */
QVector<int> EulerPath::hierholzer(int startVertex)
{
    /* 复制邻接表用于遍历(消耗边) */
    QMultiMap<int, int> adjCopy = m_adjList;

    QStack<int> stack;
    QVector<int> circuit;

    stack.push(startVertex);
    int current = startVertex;

    while (!stack.isEmpty()) {
        /* 查找当前顶点的未使用出边 */
        bool found = false;
        if (adjCopy.contains(current)) {
            auto it = adjCopy.find(current);
            if (it != adjCopy.end()) {
                int next = it.value();
                adjCopy.erase(it);

                /* 无向图: 也要移除反向边 */
                if (!m_directed) {
                    auto range = adjCopy.equal_range(next);
                    for (auto rit = range.first; rit != range.second; ++rit) {
                        if (*rit == current) {
                            adjCopy.erase(rit);
                            break;
                        }
                    }
                }

                stack.push(current);
                current = next;
                found = true;
            }
        }

        if (!found) {
            circuit.append(current);
            if (!stack.isEmpty()) {
                current = stack.pop();
            }
        }
    }

    /* 反转得到正确顺序 */
    std::reverse(circuit.begin(), circuit.end());
    return circuit;
}

/** @brief 获取所有顶点 @return 顶点列表 */
QVector<int> EulerPath::getVertices() const
{
    QSet<int> verts;
    for (auto it = m_adjList.constBegin(); it != m_adjList.constEnd(); ++it) {
        verts.insert(it.key());
        verts.insert(it.value());
    }
    return QVector<int>(verts.begin(), verts.end());
}

/** @brief 计算无向图顶点度数 @param v 顶点 @return 度数 */
int EulerPath::degree(int v) const
{
    /* 无向图中每条边在邻接表中出现两次 */
    return m_adjList.count(v);
}

/** @brief 计算有向图入度 @param v 顶点 @return 入度 */
int EulerPath::inDegree(int v) const
{
    return m_inEdges.count(v);
}

/** @brief 计算有向图出度 @param v 顶点 @return 出度 */
int EulerPath::outDegree(int v) const
{
    return m_adjList.count(v);
}

/** @brief 查找欧拉路径起始顶点 @return 起始顶点 */
int EulerPath::findStartVertex() const
{
    QVector<int> vertices = getVertices();

    if (!m_directed) {
        /* 无向图: 找奇度顶点作为起点(若有的话) */
        for (int v : vertices) {
            if (degree(v) % 2 != 0) return v;
        }
    } else {
        /* 有向图: 找 out = in + 1 的顶点 */
        for (int v : vertices) {
            if (outDegree(v) == inDegree(v) + 1) return v;
        }
    }

    /* 回路情况: 返回任意有边顶点 */
    if (!m_adjList.isEmpty()) {
        return m_adjList.firstKey();
    }
    return -1;
}

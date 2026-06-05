/**
 * @file EulerianPath.cpp
 * @brief 欧拉路径/回路检测与Hierholzer算法实现
 */

#include "utils/graph13/EulerianPath.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <stack>

/** @brief 构造函数 @param parent 父对象 */
EulerianPath::EulerianPath(QObject* parent)
    : QObject(parent)
{
}

/** @brief 设置图类型 @param type 无向/有向 */
void EulerianPath::setGraphType(GraphType type)
{
    m_graphType = type;
}

/** @brief 添加边 @param from 起点 @param to 终点 @param weight 权重 */
void EulerianPath::addEdge(int from, int to, double weight)
{
    Edge e;
    e.from = from;
    e.to = to;
    e.weight = weight;
    e.id = m_nextEdgeId++;
    m_edges.append(e);
}

/** @brief 批量设置边 @param edges 边列表 */
void EulerianPath::setEdges(const QList<Edge>& edges)
{
    m_edges = edges;
    m_nextEdgeId = 0;
    for (const auto& e : edges) {
        if (e.id >= m_nextEdgeId) {
            m_nextEdgeId = e.id + 1;
        }
    }
}

/** @brief 检测欧拉性质
 *  @param vertexCount 顶点数
 *  @return 欧拉性质 */
EulerianPath::EulerianProperty EulerianPath::checkEulerian(
    int vertexCount) const
{
    ++m_stats.totalChecks;

    if (m_edges.isEmpty()) {
        return EulerianProperty::Circuit;
    }

    if (m_graphType == GraphType::Undirected) {
        /* 无向图: 计算各顶点度数 */
        QMap<int, int> degree;
        for (const auto& e : m_edges) {
            degree[e.from]++;
            degree[e.to]++;
        }

        int oddDegreeCount = 0;
        for (auto it = degree.constBegin(); it != degree.constEnd(); ++it) {
            if (it.value() % 2 != 0) {
                ++oddDegreeCount;
            }
        }

        /* 连通性检查 */
        int components = connectedComponents(vertexCount);
        if (components > 1) {
            /* 检查边端点是否都在一个连通分量中 */
            QSet<int> edgeVertices;
            for (const auto& e : m_edges) {
                edgeVertices.insert(e.from);
                edgeVertices.insert(e.to);
            }
            /* 仅考虑有边的顶点 */
            QMap<int, QList<QPair<int, int>>> adj = buildAdjacencyList();
            QSet<int> visited;
            if (!edgeVertices.isEmpty()) {
                dfs(*edgeVertices.begin(), visited, adj);
            }
            for (int v : edgeVertices) {
                if (!visited.contains(v)) {
                    return EulerianProperty::None;
                }
            }
        }

        if (oddDegreeCount == 0) {
            return EulerianProperty::Circuit;
        } else if (oddDegreeCount == 2) {
            return EulerianProperty::Path;
        }
        return EulerianProperty::None;

    } else {
        /* 有向图: 检查入度/出度 */
        QMap<int, int> inDegree, outDegree;
        for (const auto& e : m_edges) {
            outDegree[e.from]++;
            inDegree[e.to]++;
        }

        int startCandidates = 0;
        int endCandidates = 0;
        QSet<int> allVertices;
        for (const auto& e : m_edges) {
            allVertices.insert(e.from);
            allVertices.insert(e.to);
        }

        for (int v : allVertices) {
            int diff = outDegree[v] - inDegree[v];
            if (diff == 1) {
                ++startCandidates;
            } else if (diff == -1) {
                ++endCandidates;
            } else if (qAbs(diff) > 1) {
                return EulerianProperty::None;
            }
        }

        if (startCandidates == 0 && endCandidates == 0) {
            return EulerianProperty::Circuit;
        } else if (startCandidates == 1 && endCandidates == 1) {
            return EulerianProperty::Path;
        }
        return EulerianProperty::None;
    }
}

/** @brief 求解欧拉路径/回路(Hierholzer算法)
 *  @param vertexCount 顶点数
 *  @return 欧拉求解结果 */
EulerianPath::EulerResult EulerianPath::solve(int vertexCount)
{
    QElapsedTimer timer;
    timer.start();

    EulerResult result;
    result.totalEdges = m_edges.size();
    result.totalVertices = vertexCount;

    EulerianProperty prop = checkEulerian(vertexCount);
    result.property = prop;

    if (prop == EulerianProperty::None) {
        result.hasEulerian = false;
        emit solveCompleted(result);
        return result;
    }

    result.hasEulerian = true;

    /* 构建邻接表(使用边ID标记已访问) */
    QMap<int, QList<QPair<int, int>>> adj = buildAdjacencyList();
    QSet<int> usedEdges;

    /* 找起始顶点 */
    int start = findStartVertex(vertexCount);

    /* Hierholzer算法 */
    QList<int> circuit;
    std::stack<int> stack;
    stack.push(start);

    while (!stack.empty()) {
        int v = stack.top();
        bool found = false;

        /* 查找未使用的边 */
        if (adj.contains(v)) {
            for (auto& item : adj[v]) {
                int to = item.first;
                int eid = item.second;
                if (!usedEdges.contains(eid)) {
                    usedEdges.insert(eid);
                    stack.push(to);

                    /* 从邻接表中移除此边 */
                    item = {-1, -1};
                    found = true;
                    break;
                }
            }
        }

        if (!found) {
            circuit.prepend(v);
            stack.pop();
        }
    }

    /* 构建顶点和边路径 */
    result.vertexPath = circuit;

    /* 根据顶点路径恢复边ID */
    for (int i = 0; i < circuit.size() - 1; ++i) {
        int from = circuit[i];
        int to = circuit[i + 1];
        for (const auto& e : m_edges) {
            if (e.from == from && e.to == to) {
                result.edgePath.append(e.id);
                break;
            }
            if (m_graphType == GraphType::Undirected &&
                e.from == to && e.to == from) {
                result.edgePath.append(e.id);
                break;
            }
        }
    }

    ++m_stats.totalSolves;
    m_stats.totalEdgesProcessed += static_cast<quint64>(m_edges.size());

    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalSolves);

    emit solveCompleted(result);
    return result;
}

/** @brief 清空图 */
void EulerianPath::clearGraph()
{
    m_edges.clear();
    m_nextEdgeId = 0;
}

/** @brief 获取连通分量数
 *  @param vertexCount 顶点数
 *  @return 连通分量数 */
int EulerianPath::connectedComponents(int vertexCount) const
{
    QMap<int, QList<QPair<int, int>>> adj = buildAdjacencyList();
    QSet<int> visited;
    int count = 0;

    for (int v = 0; v < vertexCount; ++v) {
        if (!visited.contains(v) && adj.contains(v)) {
            dfs(v, visited, adj);
            ++count;
        }
    }
    return qMax(count, 1);
}

/** @brief 深度优先搜索计算连通分量 */
void EulerianPath::dfs(int v, QSet<int>& visited,
                       const QMap<int, QList<QPair<int, int>>>& adj) const
{
    visited.insert(v);
    if (adj.contains(v)) {
        for (const auto& pair : adj[v]) {
            if (!visited.contains(pair.first)) {
                dfs(pair.first, visited, adj);
            }
        }
    }
}

/** @brief 构建邻接表 */
QMap<int, QList<QPair<int, int>>> EulerianPath::buildAdjacencyList() const
{
    QMap<int, QList<QPair<int, int>>> adj;
    for (const auto& e : m_edges) {
        adj[e.from].append({e.to, e.id});
        if (m_graphType == GraphType::Undirected) {
            adj[e.to].append({e.from, e.id});
        }
    }
    return adj;
}

/** @brief 找到欧拉路径起点 */
int EulerianPath::findStartVertex(int vertexCount) const
{
    if (m_graphType == GraphType::Undirected) {
        /* 找奇数度顶点作为起点 */
        QMap<int, int> degree;
        for (const auto& e : m_edges) {
            degree[e.from]++;
            degree[e.to]++;
        }
        for (auto it = degree.constBegin(); it != degree.constEnd(); ++it) {
            if (it.value() % 2 != 0) return it.key();
        }
    } else {
        /* 有向图: 出度-入度=1的顶点 */
        QMap<int, int> balance;
        for (const auto& e : m_edges) {
            balance[e.from]++;
            balance[e.to]--;
        }
        for (auto it = balance.constBegin(); it != balance.constEnd(); ++it) {
            if (it.value() == 1) return it.key();
        }
    }

    /* 默认返回第一条边的起点 */
    return m_edges.isEmpty() ? 0 : m_edges.first().from;
}

/** @brief 获取统计信息 */
EulerianPath::Stats EulerianPath::stats() const
{
    return m_stats;
}

/** @brief 重置统计 */
void EulerianPath::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

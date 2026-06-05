/**
 * @file StronglyConnected2.cpp
 * @brief 强连通分量实现 — Tarjan迭代/Kosaraju BFS/缩图DAG
 */

#include "utils/graph35/StronglyConnected2.h"

#include <QElapsedTimer>
#include <algorithm>
#include <stack>
#include <queue>

/** @brief 构造函数 @param parent 父对象 */
StronglyConnected2::StronglyConnected2(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief 设置有向图 @param adjacency 邻接表 */
void StronglyConnected2::setGraph(const QMap<int, QList<int>>& adjacency)
{
    m_adjacency = adjacency;
    m_vertexToComponent.clear();
    m_components.clear();
    m_dagEdges.clear();

    /* 确保所有顶点都在邻接表中(包括无边顶点) */
    QSet<int> allVertices;
    for (auto it = m_adjacency.constBegin(); it != m_adjacency.constEnd(); ++it) {
        allVertices.insert(it.key());
        for (int v : it.value()) {
            allVertices.insert(v);
            if (!m_adjacency.contains(v)) {
                m_adjacency[v] = QList<int>();
            }
        }
    }
}

/** @brief 计算强连通分量 @param algorithm 算法 @return 顶点->分量ID */
QMap<int, int> StronglyConnected2::compute(Algorithm algorithm)
{
    QElapsedTimer timer;
    timer.start();

    if (algorithm == Algorithm::TarjanIterative) {
        m_vertexToComponent = tarjanIterative();
    } else {
        m_vertexToComponent = kosarajuBFS();
    }

    /* 构建分量详情 */
    m_components.clear();
    QMap<int, QList<int>> compVertices;
    for (auto it = m_vertexToComponent.constBegin();
         it != m_vertexToComponent.constEnd(); ++it) {
        compVertices[it.value()].append(it.key());
    }

    for (auto it = compVertices.constBegin(); it != compVertices.constEnd(); ++it) {
        Component comp;
        comp.id = it.key();
        comp.vertices = it.value();
        comp.isCyclic = (it.value().size() > 1)
            || (it.value().size() == 1 && m_adjacency[it.value()[0]].contains(it.value()[0]));
        m_components.append(comp);
    }

    /* 更新统计 */
    double elapsed = timer.elapsed();
    ++m_stats.totalComputations;
    m_stats.totalVerticesProcessed += m_adjacency.size();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalComputations);
    if (m_components.size() > m_stats.maxComponentsFound) {
        m_stats.maxComponentsFound = m_components.size();
    }

    /* 构建DAG */
    m_dagEdges = buildCondensationDag();

    emit computationComplete(m_components.size(), m_dagEdges.size());
    return m_vertexToComponent;
}

/** @brief 获取分量详情 @return 分量列表 */
QList<StronglyConnected2::Component> StronglyConnected2::components() const
{
    return m_components;
}

/** @brief 构建缩图DAG @return DAG边列表 */
QList<StronglyConnected2::DagEdge> StronglyConnected2::buildCondensationDag()
{
    QList<DagEdge> edges;
    QSet<QPair<int, int>> seen;

    for (const auto& comp : m_components) {
        for (int v : comp.vertices) {
            for (int neighbor : m_adjacency.value(v)) {
                int neighborComp = m_vertexToComponent.value(neighbor, -1);
                if (neighborComp != comp.id && neighborComp >= 0) {
                    QPair<int, int> key = {comp.id, neighborComp};
                    if (!seen.contains(key)) {
                        seen.insert(key);
                        edges.append({comp.id, neighborComp});
                    }
                }
            }
        }
    }

    /* 更新分量的入度/出度 */
    QMap<int, int> inDeg, outDeg;
    for (const auto& e : edges) {
        ++outDeg[e.from];
        ++inDeg[e.to];
    }
    for (auto& comp : m_components) {
        comp.inDegree = inDeg.value(comp.id, 0);
        comp.outDegree = outDeg.value(comp.id, 0);
    }

    m_dagEdges = edges;
    return edges;
}

/** @brief 缩图拓扑排序 @return 分量ID列表 */
QList<int> StronglyConnected2::topologicalOrder() const
{
    QMap<int, int> inDegree;
    QSet<int> allComps;
    for (const auto& comp : m_components) {
        allComps.insert(comp.id);
        inDegree[comp.id] = comp.inDegree;
    }

    QList<int> result;
    QQueue<int> queue;
    for (int id : allComps) {
        if (inDegree[id] == 0) queue.enqueue(id);
    }

    while (!queue.isEmpty()) {
        int cur = queue.dequeue();
        result.append(cur);
        for (const auto& e : m_dagEdges) {
            if (e.from == cur) {
                --inDegree[e.to];
                if (inDegree[e.to] == 0) queue.enqueue(e.to);
            }
        }
    }
    return result;
}

/** @brief 判断两顶点是否同一分量 @param u 顶点u @param v 顶点v @return 是否同分量 */
bool StronglyConnected2::sameComponent(int u, int v) const
{
    return m_vertexToComponent.value(u, -1) == m_vertexToComponent.value(v, -2)
        && m_vertexToComponent.contains(u) && m_vertexToComponent.contains(v);
}

/** @brief Tarjan迭代算法 @return 顶点->分量ID */
QMap<int, int> StronglyConnected2::tarjanIterative()
{
    QMap<int, int> result;
    QMap<int, int> indexMap;
    QMap<int, int> lowLink;
    QSet<int> onStack;
    std::stack<int> stack;
    int globalIndex = 0;
    int componentId = 0;

    /* 收集所有顶点 */
    QList<int> allVertices = m_adjacency.keys();
    std::sort(allVertices.begin(), allVertices.end());

    /* 迭代Tarjan: 使用显式栈模拟递归 */
    for (int startVertex : allVertices) {
        if (indexMap.contains(startVertex)) continue;

        /* 工作栈: (顶点, 邻居迭代位置, 是否回溯阶段) */
        std::stack<std::tuple<int, int, bool>> workStack;
        workStack.push({startVertex, 0, false});

        while (!workStack.empty()) {
            auto [v, ni, backtrack] = workStack.top();
            workStack.pop();

            if (!backtrack) {
                if (indexMap.contains(v)) continue;

                /* 前向阶段: 初始化顶点 */
                indexMap[v] = globalIndex;
                lowLink[v] = globalIndex;
                ++globalIndex;
                stack.push(v);
                onStack.insert(v);

                /* 遍历邻居 */
                const QList<int>& neighbors = m_adjacency.value(v);
                /* 推入回溯标记 */
                workStack.push({v, 0, true});

                for (int i = neighbors.size() - 1; i >= 0; --i) {
                    int w = neighbors[i];
                    if (!indexMap.contains(w)) {
                        workStack.push({w, 0, false});
                    } else if (onStack.contains(w)) {
                        lowLink[v] = qMin(lowLink[v], indexMap[w]);
                    }
                }
            } else {
                /* 回溯阶段: 更新lowLink */
                const QList<int>& neighbors = m_adjacency.value(v);
                for (int w : neighbors) {
                    if (onStack.contains(w) && indexMap.contains(w)) {
                        /* 如果w是v的后代, 用lowLink[w]更新 */
                        if (lowLink[w] < lowLink[v] && indexMap[w] > indexMap[v]) {
                            lowLink[v] = qMin(lowLink[v], lowLink[w]);
                        }
                    }
                }

                /* 检查是否为根节点 */
                if (lowLink[v] == indexMap[v]) {
                    QList<int> component;
                    while (true) {
                        int w = stack.top();
                        stack.pop();
                        onStack.remove(w);
                        result[w] = componentId;
                        component.append(w);
                        if (w == v) break;
                    }
                    ++componentId;
                }
            }
        }
    }

    return result;
}

/** @brief Kosaraju BFS算法 @return 顶点->分量ID */
QMap<int, int> StronglyConnected2::kosarajuBFS()
{
    QMap<int, int> result;
    QList<int> allVertices = m_adjacency.keys();

    /* 第一遍: BFS计算完成顺序 */
    QSet<int> visited;
    QList<int> finishOrder;

    /* DFS用显式栈模拟获取完成序 */
    for (int start : allVertices) {
        if (visited.contains(start)) continue;

        std::stack<QPair<int, int>> dfsStack;
        dfsStack.push({start, 0});

        while (!dfsStack.empty()) {
            auto [v, idx] = dfsStack.top();

            if (idx == 0) {
                if (visited.contains(v)) { dfsStack.pop(); continue; }
                visited.insert(v);
            }

            const QList<int>& neighbors = m_adjacency.value(v);
            bool pushed = false;
            for (int i = idx; i < neighbors.size(); ++i) {
                int w = neighbors[i];
                if (!visited.contains(w)) {
                    dfsStack.top().second = i + 1;
                    dfsStack.push({w, 0});
                    pushed = true;
                    break;
                }
            }
            if (!pushed) {
                finishOrder.append(v);
                dfsStack.pop();
            }
        }
    }

    /* 第二遍: 在转置图上按完成序逆序BFS */
    QMap<int, QList<int>> transposed = transposeGraph();
    int componentId = 0;

    for (int i = finishOrder.size() - 1; i >= 0; --i) {
        int start = finishOrder[i];
        if (result.contains(start)) continue;

        QQueue<int> queue;
        queue.enqueue(start);
        result[start] = componentId;

        while (!queue.isEmpty()) {
            int v = queue.dequeue();
            for (int w : transposed.value(v)) {
                if (!result.contains(w)) {
                    result[w] = componentId;
                    queue.enqueue(w);
                }
            }
        }
        ++componentId;
    }

    return result;
}

/** @brief 转置图 @return 转置邻接表 */
QMap<int, QList<int>> StronglyConnected2::transposeGraph() const
{
    QMap<int, QList<int>> transposed;
    for (auto it = m_adjacency.constBegin(); it != m_adjacency.constEnd(); ++it) {
        if (!transposed.contains(it.key())) transposed[it.key()] = QList<int>();
        for (int v : it.value()) {
            transposed[v].append(it.key());
        }
    }
    return transposed;
}

void StronglyConnected2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

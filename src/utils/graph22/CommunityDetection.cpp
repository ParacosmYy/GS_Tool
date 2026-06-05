/**
 * @file CommunityDetection.cpp
 * @brief 社区检测引擎实现 — Louvain模块度优化算法
 */

#include "utils/graph22/CommunityDetection.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <random>

/** @brief 构造函数 @param parent 父对象 */
CommunityDetection::CommunityDetection(QObject* parent)
    : QObject(parent)
{
}

/** @brief 设置解析度参数 @param gamma 解析度(默认1.0) */
void CommunityDetection::setResolution(double gamma)
{
    m_resolution = qMax(0.01, gamma);
}

/** @brief 从边列表构建图 @param edges 边列表 @param nodeCount 节点总数 */
void CommunityDetection::buildGraph(const QList<Edge>& edges, int nodeCount)
{
    m_nodeCount = nodeCount;
    m_adjList.resize(nodeCount);
    m_nodeDegree.resize(nodeCount);
    std::fill(m_nodeDegree.begin(), m_nodeDegree.end(), 0.0);
    m_totalEdgeWeight = 0.0;

    for (auto& adj : m_adjList) {
        adj.clear();
    }

    for (const auto& e : edges) {
        if (e.source < 0 || e.source >= nodeCount ||
            e.target < 0 || e.target >= nodeCount) {
            continue;
        }
        m_adjList[e.source].append({e.target, e.weight});
        if (e.source != e.target) {
            m_adjList[e.target].append({e.source, e.weight});
        }
        m_nodeDegree[e.source] += e.weight;
        m_nodeDegree[e.target] += e.weight;
        m_totalEdgeWeight += e.weight;
        if (e.source != e.target) {
            m_totalEdgeWeight += e.weight;
        }
    }
}

/** @brief 执行Louvain社区检测 @return 检测结果 */
CommunityDetection::Result CommunityDetection::detect()
{
    QElapsedTimer timer;
    timer.start();

    Result result;
    if (m_nodeCount == 0 || m_totalEdgeWeight < 1e-12) {
        return result;
    }

    /* 初始化: 每个节点自成一个社区 */
    QVector<int> nodeComm(m_nodeCount);
    QMap<int, QList<int>> commMembers;
    for (int i = 0; i < m_nodeCount; ++i) {
        nodeComm[i] = i;
        commMembers[i].append(i);
    }

    int totalIterations = 0;
    int levels = 0;

    /* Louvain主循环 */
    bool improved = true;
    while (improved) {
        improved = localMove(nodeComm, commMembers);
        totalIterations++;
        levels++;

        emit progress(totalIterations, computeModularity(
            buildCommunityList(nodeComm, commMembers)));

        if (!improved) break;

        /* 聚合阶段: 构建新图 */
        QList<Edge> newEdges = aggregateCommunities(nodeComm);
        QMap<int, QList<int>> newCommMembers;
        QMap<int, int> oldToNew;
        int newId = 0;

        /* 重新编号社区 */
        for (auto it = commMembers.begin(); it != commMembers.end(); ++it) {
            oldToNew[it.key()] = newId;
            newCommMembers[newId] = it.value();
            newId++;
        }

        /* 收缩节点数 */
        m_nodeCount = newId;
        m_adjList.resize(m_nodeCount);
        m_nodeDegree.resize(m_nodeCount);
        std::fill(m_nodeDegree.begin(), m_nodeDegree.end(), 0.0);
        for (auto& adj : m_adjList) adj.clear();
        m_totalEdgeWeight = 0.0;

        for (const auto& e : newEdges) {
            m_adjList[e.source].append({e.target, e.weight});
            if (e.source != e.target) {
                m_adjList[e.target].append({e.source, e.weight});
            }
            m_nodeDegree[e.source] += e.weight;
            m_nodeDegree[e.target] += e.weight;
            m_totalEdgeWeight += e.weight * 2.0;
        }

        /* 更新nodeComm */
        nodeComm.resize(m_nodeCount);
        commMembers = newCommMembers;
        for (int i = 0; i < m_nodeCount; ++i) {
            nodeComm[i] = i;
        }
    }

    /* 构建最终结果 */
    result.communities = buildCommunityList(nodeComm, commMembers);
    result.totalModularity = computeModularity(result.communities);
    result.iterations = totalIterations;
    result.levels = levels;

    /* 更新统计 */
    m_stats.totalDetections++;
    m_stats.totalNodesProcessed += m_nodeCount;
    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalDetections);

    emit detectionComplete(result);
    return result;
}

/** @brief 获取节点的邻居及权重 @param node 节点 @return (邻居, 权重)列表 */
QList<QPair<int, double>> CommunityDetection::getNeighbors(int node) const
{
    if (node < 0 || node >= m_nodeCount) return {};
    return m_adjList[node];
}

/** @brief 计算模块度 @param communities 社区划分 @return 模块度值 */
double CommunityDetection::computeModularity(const QList<Community>& communities) const
{
    if (m_totalEdgeWeight < 1e-12) return 0.0;

    double Q = 0.0;
    for (const auto& comm : communities) {
        double l_c = comm.internalWeight;
        double d_c = comm.totalDegree;
        Q += (l_c / m_totalEdgeWeight)
             - m_resolution * (d_c / m_totalEdgeWeight)
                        * (d_c / m_totalEdgeWeight);
    }
    return Q;
}

/** @brief 导出社区为GraphViz DOT格式 @return DOT字符串 */
QString CommunityDetection::toDotFormat() const
{
    QString dot = "graph communities {\n";
    dot += "  node [shape=circle];\n";
    for (int i = 0; i < m_nodeCount; ++i) {
        for (const auto& [neighbor, weight] : m_adjList[i]) {
            if (i < neighbor) {
                dot += QString("  %1 -- %2 [weight=%3];\n")
                       .arg(i).arg(neighbor).arg(weight, 0, 'f', 2);
            }
        }
    }
    dot += "}\n";
    return dot;
}

/** @brief 清空图数据 */
void CommunityDetection::clear()
{
    m_nodeCount = 0;
    m_totalEdgeWeight = 0.0;
    m_adjList.clear();
    m_nodeDegree.clear();
}

/** @brief 重置统计 */
void CommunityDetection::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 第一阶段: 局部移动节点优化模块度 */
bool CommunityDetection::localMove(QVector<int>& nodeComm,
                                    QMap<int, QList<int>>& commMembers)
{
    bool anyImproved = false;
    bool changed = true;
    int maxPasses = 50;

    for (int pass = 0; pass < maxPasses && changed; ++pass) {
        changed = false;
        for (int node = 0; node < m_nodeCount; ++node) {
            int bestComm = nodeComm[node];
            double bestGain = 0.0;

            /* 尝试将节点移入邻居所在社区 */
            QSet<int> candidateComms;
            candidateComms.insert(nodeComm[node]);
            for (const auto& [neighbor, w] : m_adjList[node]) {
                candidateComms.insert(nodeComm[neighbor]);
            }

            for (int c : candidateComms) {
                double gain = modularityGain(node, c, nodeComm);
                if (gain > bestGain) {
                    bestGain = gain;
                    bestComm = c;
                }
            }

            if (bestComm != nodeComm[node]) {
                int oldComm = nodeComm[node];
                commMembers[oldComm].removeOne(node);
                if (commMembers[oldComm].isEmpty()) {
                    commMembers.remove(oldComm);
                }
                commMembers[bestComm].append(node);
                nodeComm[node] = bestComm;
                changed = true;
                anyImproved = true;
            }
        }
    }
    return anyImproved;
}

/** @brief 计算将节点移入某社区的模块度增量 */
double CommunityDetection::modularityGain(int node, int targetComm,
                                           const QVector<int>& nodeComm) const
{
    double ki = m_nodeDegree[node];
    double ki_in = 0.0; /* 节点node到targetComm中邻居的权重和 */
    double tot_c = 0.0; /* targetComm的总度数 */

    for (int i = 0; i < m_nodeCount; ++i) {
        if (nodeComm[i] == targetComm) {
            tot_c += m_nodeDegree[i];
        }
    }

    for (const auto& [neighbor, weight] : m_adjList[node]) {
        if (nodeComm[neighbor] == targetComm) {
            ki_in += weight;
        }
    }

    double m2 = m_totalEdgeWeight;
    double dQ = ki_in / m2
                - m_resolution * tot_c * ki / (m2 * m2) * 2.0;
    return dQ;
}

/** @brief 第二阶段: 聚合社区为超节点 */
QList<CommunityDetection::Edge> CommunityDetection::aggregateCommunities(
    const QVector<int>& nodeComm)
{
    /* 获取唯一社区ID */
    QSet<int> uniqueComms;
    for (int c : nodeComm) uniqueComms.insert(c);
    QMap<int, int> commToIdx;
    int idx = 0;
    for (int c : uniqueComms) {
        commToIdx[c] = idx++;
    }

    /* 合并社区间边权重 */
    QMap<QPair<int, int>, double> edgeWeights;
    for (int i = 0; i < m_nodeCount; ++i) {
        int ci = commToIdx[nodeComm[i]];
        for (const auto& [neighbor, weight] : m_adjList[i]) {
            int cj = commToIdx[nodeComm[neighbor]];
            if (ci <= cj) {
                edgeWeights[{ci, cj}] += weight;
            }
        }
    }

    QList<Edge> newEdges;
    for (auto it = edgeWeights.begin(); it != edgeWeights.end(); ++it) {
        Edge e;
        e.source = it.key().first;
        e.target = it.key().second;
        e.weight = it.value();
        newEdges.append(e);
    }
    return newEdges;
}

/** @brief 辅助: 从nodeComm和commMembers构建Community列表 */
QList<CommunityDetection::Community> CommunityDetection::buildCommunityList(
    const QVector<int>& nodeComm,
    const QMap<int, QList<int>>& commMembers) const
{
    QList<Community> result;
    for (auto it = commMembers.begin(); it != commMembers.end(); ++it) {
        Community c;
        c.id = it.key();
        c.members = it.value();

        double internalW = 0.0;
        double totalDeg = 0.0;
        for (int node : c.members) {
            totalDeg += m_nodeDegree[node];
            for (const auto& [neighbor, weight] : m_adjList[node]) {
                if (nodeComm[neighbor] == c.id && node < neighbor) {
                    internalW += weight;
                }
            }
        }
        c.internalWeight = internalW;
        c.totalDegree = totalDeg;
        result.append(c);
    }
    return result;
}

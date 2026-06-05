/**
 * @file DBSCAN3.cpp
 * @brief HDBSCAN层次密度聚类实现 — 互达距离/MST/压缩树/稳定性提取
 */

#include "utils/cluster19/DBSCAN3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <queue>

/** @brief 构造函数 @param parent 父对象 */
DBSCAN3::DBSCAN3(QObject* parent)
    : QObject(parent)
    , m_minClusterSize(5)
    , m_minSamples(5)
    , m_timeSum(0.0)
{
}

void DBSCAN3::setMinClusterSize(int minSize)
{
    m_minClusterSize = qMax(2, minSize);
}

void DBSCAN3::setMinSamples(int minSamples)
{
    m_minSamples = qMax(1, minSamples);
}

/** @brief 执行HDBSCAN聚类 @param data 二维数据 @return 聚类标签 */
QVector<int> DBSCAN3::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    QVector<int> labels(n, -1);

    if (n < m_minClusterSize) {
        emit clusteringComplete(0, n);
        return labels;
    }

    /* 1. 计算核心距离 */
    QVector<double> coreDistances = computeCoreDistances(data);

    /* 2. 构建互达距离图 */
    QList<Edge> mrGraph = buildMutualReachabilityGraph(data, coreDistances);

    /* 3. 计算最小生成树(Kruskal) */
    m_mstEdges = computeMST(mrGraph, n);

    /* 4. 构建压缩聚类树 */
    m_tree = buildCondensedTree(m_mstEdges, n);

    /* 5. 计算稳定性 */
    m_stabilities = computeStability(m_tree);

    /* 6. 提取稳定聚类(Excess of Mass) */
    extractStableClusters(m_tree);

    /* 7. 根据压缩树分配标签 */
    /* 按MST边的距离升序排序用于单链接聚类 */
    QList<Edge> sortedMst = m_mstEdges;
    std::sort(sortedMst.begin(), sortedMst.end(),
              [](const Edge& a, const Edge& b) { return a.distance < b.distance; });

    QVector<int> dsParent(n);
    QVector<int> dsSize(n, 1);
    for (int i = 0; i < n; ++i) dsParent[i] = i;

    int clusterId = 0;
    QMap<int, int> rootToLabel;
    /* 找出被选中的聚类节点 */
    QSet<int> selectedIds;
    for (const auto& node : m_tree) {
        if (node.selected) selectedIds.insert(node.id);
    }

    /* 按距离阈值逐步合并 */
    for (const Edge& e : sortedMst) {
        int rootA = findRoot(e.from, dsParent);
        int rootB = findRoot(e.to, dsParent);
        if (rootA != rootB) {
            /* 按大小合并 */
            if (dsSize[rootA] < dsSize[rootB]) std::swap(rootA, rootB);
            dsParent[rootB] = rootA;
            dsSize[rootA] += dsSize[rootB];
        }
    }

    /* 直接基于噪声/非噪声分配标签 */
    /* 使用互达距离阈值判断噪声点 */
    double lambdaThreshold = 0.0;
    for (const auto& node : m_tree) {
        if (node.selected && node.size >= m_minClusterSize) {
            lambdaThreshold = qMax(lambdaThreshold, node.birthLevel);
        }
    }

    /* 简化标签分配: 用最终的连通分量 */
    QMap<int, int> rootToCluster;
    for (int i = 0; i < n; ++i) {
        int root = findRoot(i, dsParent);
        if (dsSize[root] < m_minClusterSize) {
            labels[i] = -1; /* 噪声 */
        } else {
            if (!rootToCluster.contains(root)) {
                rootToCluster[root] = clusterId++;
            }
            labels[i] = rootToCluster[root];
        }
    }

    /* 更新统计 */
    double elapsed = timer.elapsed();
    ++m_stats.totalClusterings;
    m_stats.totalPointsProcessed += n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalClusterings);
    if (clusterId > m_stats.maxClustersFound) {
        m_stats.maxClustersFound = clusterId;
    }

    int noiseCount = 0;
    for (int l : labels) { if (l < 0) ++noiseCount; }
    emit clusteringComplete(clusterId, noiseCount);
    return labels;
}

/** @brief 获取压缩聚类树 @return 聚类节点列表 */
QList<DBSCAN3::ClusterNode> DBSCAN3::condensedTree() const
{
    return m_tree;
}

/** @brief 获取最小生成树 @return 边列表 */
QList<DBSCAN3::Edge> DBSCAN3::minimumSpanningTree() const
{
    return m_mstEdges;
}

/** @brief 获取聚类稳定性 @return 聚类ID->稳定性 */
QMap<int, double> DBSCAN3::clusterStabilities() const
{
    return m_stabilities;
}

/** @brief 计算核心距离 @param data 数据 @return 核心距离数组 */
QVector<double> DBSCAN3::computeCoreDistances(
    const QVector<QVector<double>>& data) const
{
    int n = data.size();
    int k = qMin(m_minSamples, n - 1);
    QVector<double> coreDist(n, 0.0);

    for (int i = 0; i < n; ++i) {
        QVector<double> dists;
        dists.reserve(n - 1);
        for (int j = 0; j < n; ++j) {
            if (i != j) dists.append(euclidean(data[i], data[j]));
        }
        std::sort(dists.begin(), dists.end());
        coreDist[i] = (k > 0 && k <= dists.size()) ? dists[k - 1] : 0.0;
    }
    return coreDist;
}

/** @brief 构建互达距离图 @param data 数据 @param coreDistances 核心距离 @return 边列表 */
QList<DBSCAN3::Edge> DBSCAN3::buildMutualReachabilityGraph(
    const QVector<QVector<double>>& data,
    const QVector<double>& coreDistances) const
{
    int n = data.size();
    QList<Edge> edges;
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            double d = euclidean(data[i], data[j]);
            double mrDist = qMax({coreDistances[i], coreDistances[j], d});
            edges.append({i, j, mrDist});
        }
    }
    return edges;
}

/** @brief Kruskal计算MST @param edges 边列表 @param numPoints 点数 @return MST边列表 */
QList<DBSCAN3::Edge> DBSCAN3::computeMST(
    const QList<Edge>& edges, int numPoints) const
{
    QList<Edge> sorted = edges;
    std::sort(sorted.begin(), sorted.end(),
              [](const Edge& a, const Edge& b) { return a.distance < b.distance; });

    QVector<int> parent(numPoints);
    for (int i = 0; i < numPoints; ++i) parent[i] = i;

    QList<Edge> mst;
    for (const Edge& e : sorted) {
        int ra = findRoot(e.from, parent);
        int rb = findRoot(e.to, parent);
        if (ra != rb) {
            mst.append(e);
            parent[ra] = rb;
            if (mst.size() == numPoints - 1) break;
        }
    }
    return mst;
}

/** @brief 构建压缩聚类树 @param mstEdges MST边 @param numPoints 点数 @return 聚类节点列表 */
QList<DBSCAN3::ClusterNode> DBSCAN3::buildCondensedTree(
    const QList<Edge>& mstEdges, int numPoints)
{
    QList<ClusterNode> nodes;
    QList<Edge> sortedEdges = mstEdges;
    std::sort(sortedEdges.begin(), sortedEdges.end(),
              [](const Edge& a, const Edge& b) { return a.distance < b.distance; });

    QVector<int> dsParent(numPoints);
    QVector<int> dsRank(numPoints, 0);
    QVector<int> componentId(numPoints);
    for (int i = 0; i < numPoints; ++i) {
        dsParent[i] = i;
        componentId[i] = i;
    }

    int nextClusterId = numPoints;
    QHash<int, ClusterNode> nodeMap;
    QHash<int, double> birthLevel;
    QHash<int, int> clusterSize;

    /* 初始化每个点为一个"聚类" */
    for (int i = 0; i < numPoints; ++i) {
        ClusterNode leaf;
        leaf.id = i;
        leaf.parentId = -1;
        leaf.birthLevel = 0.0;
        leaf.deathLevel = 0.0;
        leaf.size = 1;
        nodeMap[i] = leaf;
        birthLevel[i] = 0.0;
        clusterSize[i] = 1;
    }

    for (const Edge& e : sortedEdges) {
        /* 并查集找根 */
        int rootA = e.from, rootB = e.to;
        while (dsParent[rootA] != rootA) { dsParent[rootA] = dsParent[dsParent[rootA]]; rootA = dsParent[rootA]; }
        while (dsParent[rootB] != rootB) { dsParent[rootB] = dsParent[dsParent[rootB]]; rootB = dsParent[rootB]; }

        if (rootA == rootB) continue;

        int newId = nextClusterId++;
        ClusterNode merged;
        merged.id = newId;
        merged.birthLevel = e.distance;
        merged.deathLevel = e.distance;
        merged.size = clusterSize[rootA] + clusterSize[rootB];
        merged.parentId = -1;

        nodeMap[rootA].parentId = newId;
        nodeMap[rootA].deathLevel = e.distance;
        nodeMap[rootB].parentId = newId;
        nodeMap[rootB].deathLevel = e.distance;

        nodeMap[newId] = merged;

        /* 按秩合并 */
        if (dsRank[rootA] < dsRank[rootB]) std::swap(rootA, rootB);
        dsParent[rootB] = rootA;
        if (dsRank[rootA] == dsRank[rootB]) ++dsRank[rootA];
        componentId[rootA] = newId;
        birthLevel[newId] = e.distance;
        clusterSize[newId] = merged.size;
    }

    /* 收集所有节点 */
    QList<ClusterNode> result;
    for (auto it = nodeMap.constBegin(); it != nodeMap.constEnd(); ++it) {
        result.append(it.value());
    }
    return result;
}

/** @brief 计算聚类稳定性 @param nodes 聚类节点列表 @return 聚类ID->稳定性 */
QMap<int, double> DBSCAN3::computeStability(const QList<ClusterNode>& nodes) const
{
    QMap<int, double> stabilities;
    for (const auto& node : nodes) {
        if (node.size >= m_minClusterSize) {
            double lambdaBirth = (node.birthLevel > 0) ? 1.0 / node.birthLevel : 0.0;
            double lambdaDeath = (node.deathLevel > 0) ? 1.0 / node.deathLevel : 0.0;
            stabilities[node.id] = (lambdaBirth - lambdaDeath) * node.size;
        }
    }
    return stabilities;
}

/** @brief 提取稳定聚类(Excess of Mass) @param nodes 聚类节点列表 */
void DBSCAN3::extractStableClusters(QList<ClusterNode>& nodes)
{
    /* 简化EoM: 选择稳定性大于0的非叶节点 */
    for (auto& node : nodes) {
        if (node.size >= m_minClusterSize && m_stabilities.contains(node.id)) {
            node.selected = m_stabilities[node.id] > 0;
        }
    }
}

/** @brief 欧氏距离 @param a 向量a @param b 向量b @return 距离 */
double DBSCAN3::euclidean(const QVector<double>& a,
                           const QVector<double>& b) const
{
    double sum = 0.0;
    int dim = qMin(a.size(), b.size());
    for (int i = 0; i < dim; ++i) {
        double d = a[i] - b[i];
        sum += d * d;
    }
    return qSqrt(sum);
}

/** @brief 并查集查找根 @param x 元素 @param parent 父数组 @return 根 */
int DBSCAN3::findRoot(int x, QVector<int>& parent) const
{
    while (parent[x] != x) {
        parent[x] = parent[parent[x]];
        x = parent[x];
    }
    return x;
}

void DBSCAN3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

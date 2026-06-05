/**
 * @file GraphIsomorphism.cpp
 * @brief 图同构检测实现 — Weisfeiler-Lehman (WL) 测试
 */

#include "utils/graph19/GraphIsomorphism.h"

#include <QCryptographicHash>
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <random>

GraphIsomorphism::GraphIsomorphism(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

GraphIsomorphism::IsomorphismResult GraphIsomorphism::checkIsomorphism(
    const AdjacencyList& adjList1, const AdjacencyList& adjList2,
    int maxIterations)
{
    QElapsedTimer timer;
    timer.start();

    IsomorphismResult result;

    /* 第一阶段: 快速预筛选 */
    if (!quickFilter(adjList1, adjList2)) {
        result.definitelyNonIsomorphic = true;
        result.possiblyIsomorphic = false;
        result.failureReason = QStringLiteral("基本属性不匹配(节点数/边数/度序列)");
        ++m_stats.totalComparisons;
        ++m_stats.totalNonIsomorphicFound;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComparisons;
        return result;
    }

    int n = adjList1.size();

    /* 第二阶段: WL标签传播 */
    QVector<int> labels1 = initializeLabels(adjList1);
    QVector<int> labels2 = initializeLabels(adjList2);

    for (int iter = 0; iter < maxIterations; ++iter) {
        /* 执行一轮WL迭代 */
        QMap<int, int> hist1 = wlIteration(adjList1, labels1);
        QMap<int, int> hist2 = wlIteration(adjList2, labels2);

        /* 比较标签分布 */
        if (hist1 != hist2) {
            result.definitelyNonIsomorphic = true;
            result.possiblyIsomorphic = false;
            result.iterationsUsed = iter + 1;
            result.failureReason = QStringLiteral("WL迭代第 %1 轮标签分布不一致").arg(iter + 1);
            ++m_stats.totalComparisons;
            ++m_stats.totalNonIsomorphicFound;
            m_timeSum += timer.elapsed();
            m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComparisons;
            return result;
        }

        /* 检查标签是否收敛(不再变化) */
        QByteArray hash1 = computeLabelHash(labels1);
        QByteArray hash2 = computeLabelHash(labels2);
        if (iter > 0 && hash1 == hash2) {
            result.iterationsUsed = iter + 1;
            break;
        }
        result.iterationsUsed = iter + 1;
    }

    /* 第三阶段: 构建节点映射 */
    result.nodeMapping = buildNodeMapping(adjList1, adjList2, labels1, labels2);
    result.possiblyIsomorphic = true;

    /* 计算置信度: 基于标签分布的唯一性 */
    QMap<int, int> labelCount;
    for (int lbl : labels1) ++labelCount[lbl];
    int uniqueLabels = 0;
    for (auto it = labelCount.begin(); it != labelCount.end(); ++it) {
        if (it.value() == 1) ++uniqueLabels;
    }
    result.confidence = (n > 0) ? static_cast<double>(uniqueLabels) / n : 0.0;

    ++m_stats.totalComparisons;
    ++m_stats.totalIsomorphicFound;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComparisons;

    return result;
}

GraphIsomorphism::GraphSignature GraphIsomorphism::computeSignature(
    const AdjacencyList& adjList, int maxIterations)
{
    QElapsedTimer timer;
    timer.start();

    GraphSignature sig;
    int n = adjList.size();
    sig.nodeCount = n;

    /* 计算边数 */
    sig.edgeCount = 0;
    for (const auto& neighbors : adjList) {
        sig.edgeCount += neighbors.size();
    }
    sig.edgeCount /= 2; /* 无向图每条边计数两次 */

    /* WL标签传播 */
    QVector<int> labels = initializeLabels(adjList);

    for (int iter = 0; iter < maxIterations; ++iter) {
        QVector<int> oldLabels = labels;
        wlIteration(adjList, labels);
        if (labels == oldLabels) break;
    }

    /* 排序标签作为规范签名 */
    sig.canonicalLabels = labels;
    std::sort(sig.canonicalLabels.begin(), sig.canonicalLabels.end());

    /* 计算MD5哈希 */
    QByteArray data;
    for (int lbl : sig.canonicalLabels) {
        data.append(reinterpret_cast<const char*>(&lbl), sizeof(int));
    }
    sig.hashValue = QCryptographicHash::hash(data, QCryptographicHash::Md5);

    ++m_stats.totalSignaturesComputed;
    m_timeSum += timer.elapsed();

    return sig;
}

bool GraphIsomorphism::quickFilter(const AdjacencyList& adjList1,
                                     const AdjacencyList& adjList2) const
{
    /* 节点数必须相等 */
    if (adjList1.size() != adjList2.size()) return false;

    int n = adjList1.size();
    if (n == 0) return true;

    /* 计算度序列并排序比较 */
    QVector<int> deg1(n), deg2(n);
    int edges1 = 0, edges2 = 0;
    for (int i = 0; i < n; ++i) {
        deg1[i] = adjList1[i].size();
        deg2[i] = adjList2[i].size();
        edges1 += deg1[i];
        edges2 += deg2[i];
    }

    /* 边数必须相等 */
    if (edges1 != edges2) return false;

    /* 度序列必须相等 */
    std::sort(deg1.begin(), deg1.end());
    std::sort(deg2.begin(), deg2.end());
    return deg1 == deg2;
}

double GraphIsomorphism::signatureSimilarity(const GraphSignature& sig1,
                                               const GraphSignature& sig2) const
{
    if (sig1.hashValue == sig2.hashValue) return 1.0;
    if (sig1.nodeCount != sig2.nodeCount || sig1.edgeCount != sig2.edgeCount)
        return 0.0;

    /* 比较规范标签的Jaccard相似度 */
    int n = qMin(sig1.canonicalLabels.size(), sig2.canonicalLabels.size());
    if (n == 0) return 0.0;

    int matches = 0;
    int i = 0, j = 0;
    while (i < sig1.canonicalLabels.size() && j < sig2.canonicalLabels.size()) {
        if (sig1.canonicalLabels[i] == sig2.canonicalLabels[j]) {
            ++matches; ++i; ++j;
        } else if (sig1.canonicalLabels[i] < sig2.canonicalLabels[j]) {
            ++i;
        } else {
            ++j;
        }
    }
    int total = sig1.canonicalLabels.size() + sig2.canonicalLabels.size() - matches;
    return (total > 0) ? static_cast<double>(matches) / total : 0.0;
}

QVector<int> GraphIsomorphism::batchFindIsomorphic(
    const AdjacencyList& target,
    const QVector<AdjacencyList>& candidates)
{
    QVector<int> results;

    /* 先计算目标签名 */
    GraphSignature targetSig = computeSignature(target);

    for (int i = 0; i < candidates.size(); ++i) {
        /* 快速预筛选 */
        if (!quickFilter(target, candidates[i])) continue;

        /* 签名匹配测试 */
        GraphSignature candSig = computeSignature(candidates[i]);
        if (targetSig.hashValue == candSig.hashValue) {
            /* WL完整测试 */
            auto result = checkIsomorphism(target, candidates[i]);
            if (result.possiblyIsomorphic) {
                results.append(i);
            }
        }
    }

    return results;
}

QMap<int, int> GraphIsomorphism::wlIteration(const AdjacencyList& adjList,
                                               QVector<int>& labels) const
{
    int n = adjList.size();
    QVector<int> newLabels(n);
    QMap<int, int> histogram;

    for (int v = 0; v < n; ++v) {
        /* 收集邻居标签并排序 */
        QVector<int> neighborLabels;
        for (int neighbor : adjList[v]) {
            neighborLabels.append(labels[neighbor]);
        }
        std::sort(neighborLabels.begin(), neighborLabels.end());

        /* 构造复合标签: 自身标签 + 邻居标签序列 */
        QByteArray hashInput;
        int selfLabel = labels[v];
        hashInput.append(reinterpret_cast<const char*>(&selfLabel), sizeof(int));
        for (int nl : neighborLabels) {
            hashInput.append(reinterpret_cast<const char*>(&nl), sizeof(int));
        }

        /* 使用哈希生成新标签 */
        QByteArray hash = QCryptographicHash::hash(hashInput, QCryptographicHash::Md5);
        int newLabel = 0;
        for (int i = 0; i < 4 && i < hash.size(); ++i) {
            newLabel = (newLabel << 8) | (static_cast<unsigned char>(hash[i]));
        }
        newLabels[v] = newLabel;
        ++histogram[newLabel];
    }

    labels = newLabels;
    return histogram;
}

QVector<int> GraphIsomorphism::initializeLabels(const AdjacencyList& adjList) const
{
    int n = adjList.size();
    QVector<int> labels(n);

    /* 初始标签基于度数 */
    for (int i = 0; i < n; ++i) {
        labels[i] = adjList[i].size();
    }

    return labels;
}

GraphIsomorphism::IsomorphismMap GraphIsomorphism::buildNodeMapping(
    const AdjacencyList& adjList1, const AdjacencyList& adjList2,
    const QVector<int>& labels1, const QVector<int>& labels2) const
{
    IsomorphismMap mapping;
    int n = labels1.size();

    /* 构建标签到节点的映射 */
    QMap<int, QVector<int>> labelToNodes2;
    for (int i = 0; i < n; ++i) {
        labelToNodes2[labels2[i]].append(i);
    }

    /* 贪心匹配: 相同标签的节点对 */
    QSet<int> used2;
    for (int i = 0; i < n; ++i) {
        int lbl = labels1[i];
        if (labelToNodes2.contains(lbl)) {
            for (int j : labelToNodes2[lbl]) {
                if (!used2.contains(j)) {
                    mapping[i] = j;
                    used2.insert(j);
                    break;
                }
            }
        }
    }

    return mapping;
}

QByteArray GraphIsomorphism::computeLabelHash(const QVector<int>& labels) const
{
    QByteArray data;
    for (int lbl : labels) {
        data.append(reinterpret_cast<const char*>(&lbl), sizeof(int));
    }
    return QCryptographicHash::hash(data, QCryptographicHash::Md5);
}

GraphIsomorphism::Stats GraphIsomorphism::stats() const
{
    return m_stats;
}

void GraphIsomorphism::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @file IsolationForest.cpp
 * @brief 孤立森林实现 — 随机隔离树构建 + 异常分数计算
 */

#include "utils/cluster11/IsolationForest.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <cmath>
#include <numeric>
#include <random>

/* ── 构造/配置 ── */

/** @brief 构造函数 @param parent 父对象 */
IsolationForest::IsolationForest(QObject* parent)
    : QObject(parent)
    , m_treeCount(100)
    , m_sampleSize(256)
    , m_maxDepth(0)
    , m_threshold(0.6)
    , m_featureCount(0)
    , m_trainSize(0)
{
}

/** @brief 设置树数量 @param count 树数量 */
void IsolationForest::setTreeCount(int count)
{
    m_treeCount = qMax(1, count);
}

/** @brief 设置子采样大小 @param size 子采样大小 */
void IsolationForest::setSampleSize(int size)
{
    m_sampleSize = qMax(2, size);
}

/** @brief 设置最大树深度 @param depth 最大深度 */
void IsolationForest::setMaxDepth(int depth)
{
    m_maxDepth = qMax(1, depth);
}

/** @brief 设置异常判定阈值 @param threshold 阈值 */
void IsolationForest::setAnomalyThreshold(double threshold)
{
    m_threshold = qBound(0.0, threshold, 1.0);
}

/* ── 训练 ── */

/** @brief 训练孤立森林 @param data 训练数据[样本 x 特征] */
void IsolationForest::fit(const QVector<QVector<double>>& data)
{
    if (data.isEmpty()) return;

    QElapsedTimer timer;
    timer.start();

    m_featureCount = data[0].size();
    m_trainSize = static_cast<int>(data.size());
    int effectiveSample = qMin(m_sampleSize, m_trainSize);
    if (m_maxDepth <= 0) {
        m_maxDepth = static_cast<int>(std::ceil(std::log2(effectiveSample)));
    }

    m_forest.clear();
    m_forest.reserve(m_treeCount);

    std::mt19937 rng(std::random_device{}());

    for (int t = 0; t < m_treeCount; ++t) {
        /* 随机子采样 */
        QVector<int> indices;
        indices.reserve(effectiveSample);
        if (m_trainSize <= effectiveSample) {
            for (int i = 0; i < m_trainSize; ++i) indices.append(i);
        } else {
            std::uniform_int_distribution<int> dist(0, m_trainSize - 1);
            for (int i = 0; i < effectiveSample; ++i) {
                indices.append(dist(rng));
            }
        }

        QRandomGenerator qtRng(static_cast<quint32>(rng()));
        auto tree = buildTree(data, indices, 0, qtRng);
        m_forest.append(std::move(tree));
    }

    /* 更新统计 */
    m_stats.totalTreesBuilt += m_treeCount;
    m_stats.totalSamplesTrained += m_trainSize;
    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;

    emit trainingComplete(m_treeCount, m_trainSize);
}

/* ── 预测 ── */

/** @brief 预测单个样本 @param sample 样本特征向量 @return 异常检测结果 */
IsolationForest::AnomalyResult IsolationForest::predict(
    const QVector<double>& sample) const
{
    AnomalyResult result;
    if (m_forest.isEmpty() || sample.size() != m_featureCount) return result;

    /* 计算平均路径长度 */
    double totalPath = 0.0;
    for (const auto& tree : m_forest) {
        totalPath += static_cast<double>(pathLength(sample, tree));
    }
    double avgPath = totalPath / static_cast<double>(m_forest.size());
    result.pathLength = static_cast<int>(std::round(avgPath));

    /* 异常分数: s(x,n) = 2^(-E(h(x))/c(n)) */
    double c = averagePathLength(m_trainSize);
    if (c < 1e-10) c = 1.0;
    result.score = std::pow(2.0, -avgPath / c);
    result.score = qBound(0.0, result.score, 1.0);
    result.isAnomaly = (result.score >= m_threshold);

    return result;
}

/** @brief 批量预测 @param data 样本集合 @return 结果列表 */
QList<IsolationForest::AnomalyResult> IsolationForest::predictBatch(
    const QVector<QVector<double>>& data) const
{
    QList<AnomalyResult> results;
    for (const auto& sample : data) {
        results.append(predict(sample));
    }

    /* 统计异常数(需const_cast因为信号不是const) */
    const_cast<IsolationForest*>(this)->m_stats.totalPredictions
        += data.size();
    for (const auto& r : results) {
        if (r.isAnomaly) {
            ++const_cast<IsolationForest*>(this)->m_stats.totalAnomaliesFound;
            emit const_cast<IsolationForest*>(this)->anomalyDetected(r.score);
        }
    }
    return results;
}

/* ── 私有: 构建隔离树 ── */

/** @brief 递归构建单棵隔离树 */
QVector<IsolationForest::IsoNode> IsolationForest::buildTree(
    const QVector<QVector<double>>& data,
    const QVector<int>& indices, int depth,
    QRandomGenerator& rng)
{
    QVector<IsoNode> nodes;

    if (indices.size() <= 1 || depth >= m_maxDepth) {
        IsoNode leaf;
        leaf.splitFeature = -1;
        leaf.size = static_cast<int>(indices.size());
        leaf.depth = depth;
        nodes.append(leaf);
        return nodes;
    }

    /* 随机选择分割特征 */
    int feat = rng.bounded(m_featureCount);

    /* 计算该特征在当前样本中的[min, max] */
    double minVal = data[indices[0]][feat];
    double maxVal = minVal;
    for (int idx : indices) {
        double v = data[idx][feat];
        if (v < minVal) minVal = v;
        if (v > maxVal) maxVal = v;
    }

    if (qFuzzyCompare(minVal, maxVal)) {
        /* 特征值全部相同，无法分割 */
        IsoNode leaf;
        leaf.splitFeature = -1;
        leaf.size = static_cast<int>(indices.size());
        leaf.depth = depth;
        nodes.append(leaf);
        return nodes;
    }

    /* 随机选择分割点 */
    double splitVal = minVal + rng.generateDouble() * (maxVal - minVal);

    /* 分割样本 */
    QVector<int> leftIdx, rightIdx;
    for (int idx : indices) {
        if (data[idx][feat] < splitVal) {
            leftIdx.append(idx);
        } else {
            rightIdx.append(idx);
        }
    }

    /* 创建内部节点 */
    IsoNode node;
    node.splitFeature = feat;
    node.splitValue = splitVal;
    node.depth = depth;
    node.size = static_cast<int>(indices.size());
    int nodeIdx = static_cast<int>(nodes.size());
    nodes.append(node);

    /* 递归构建左子树 */
    auto leftTree = buildTree(data, leftIdx, depth + 1, rng);
    nodes[nodeIdx].left = static_cast<int>(nodes.size());
    for (auto& n : leftTree) nodes.append(std::move(n));

    /* 递归构建右子树 */
    auto rightTree = buildTree(data, rightIdx, depth + 1, rng);
    nodes[nodeIdx].right = static_cast<int>(nodes.size());
    for (auto& n : rightTree) nodes.append(std::move(n));

    return nodes;
}

/* ── 私有: 路径长度 ── */

/** @brief 计算样本在单棵树中的路径长度(到达叶子的边数) */
int IsolationForest::pathLength(const QVector<double>& sample,
                                 const QVector<IsoNode>& tree) const
{
    int idx = 0;
    while (idx >= 0 && idx < tree.size()) {
        const auto& node = tree[idx];
        if (node.splitFeature < 0) {
            /* 叶节点: 路径长度 = 深度 + c(node.size) */
            return node.depth
                + static_cast<int>(std::ceil(averagePathLength(node.size)));
        }
        if (sample[node.splitFeature] < node.splitValue) {
            idx = node.left;
        } else {
            idx = node.right;
        }
    }
    return m_maxDepth;
}

/* ── 私有: 平均路径长度 ── */

/** @brief c(n) = 2H(n-1) - 2(n-1)/n，H为调和数 */
double IsolationForest::averagePathLength(int n)
{
    if (n <= 1) return 0.0;
    if (n == 2) return 1.0;
    /* H(i) ≈ ln(i) + EulerGamma */
    const double EulerGamma = 0.5772156649015329;
    double h = std::log(static_cast<double>(n - 1)) + EulerGamma;
    return 2.0 * h - 2.0 * static_cast<double>(n - 1) / static_cast<double>(n);
}

/* ── 统计 ── */

/** @brief 重置统计 */
void IsolationForest::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

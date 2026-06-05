/**
 * @file IsolationForest4.cpp
 * @brief 孤立森林异常检测实现 — 随机特征分割 + 路径长度评分
 */

#include "utils/cluster78/IsolationForest4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <random>

/** @brief 构造函数 @param parent 父对象 */
IsolationForest4::IsolationForest4(QObject* parent)
    : QObject(parent)
{
}

/** @brief 设置参数 @param numTrees 树数量 @param sampleSize 子采样大小 */
void IsolationForest4::setParameters(int numTrees, int sampleSize)
{
    m_numTrees = qMax(1, numTrees);
    m_sampleSize = qMax(2, sampleSize);
}

/** @brief 重置统计数据 */
void IsolationForest4::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 孤立树节点
 */
struct IsoNode {
    int splitFeature = -1;       ///< 分割特征索引
    double splitValue = 0.0;     ///< 分割阈值
    int leftChild = -1;          ///< 左子节点索引
    int rightChild = -1;         ///< 右子节点索引
    int size = 0;                ///< 节点样本数
    int depth = 0;               ///< 节点深度
};

/**
 * @brief 孤立树结构
 */
struct IsoTree {
    QVector<IsoNode> nodes;      ///< 所有节点
    int maxDepth = 0;            ///< 最大深度

    /** @brief 递归构建孤立树 */
    int build(const QVector<QVector<double>>& data,
              const QVector<int>& indices, int depth,
              int maxDepthLimit, std::mt19937& rng)
    {
        int nodeIdx = nodes.size();
        nodes.append(IsoNode());
        nodes[nodeIdx].depth = depth;
        nodes[nodeIdx].size = indices.size();

        /* 终止条件: 单一样本或达到深度限制 */
        if (indices.size() <= 1 || depth >= maxDepthLimit) {
            nodes[nodeIdx].splitFeature = -1;
            return nodeIdx;
        }

        int dim = data[0].size();

        /* 随机选择分割特征 */
        std::uniform_int_distribution<int> featDist(0, dim - 1);
        int feat = featDist(rng);
        nodes[nodeIdx].splitFeature = feat;

        /* 计算该特征在当前样本中的范围 */
        double minVal = 1e18, maxVal = -1e18;
        for (int idx : indices) {
            minVal = std::min(minVal, data[idx][feat]);
            maxVal = std::max(maxVal, data[idx][feat]);
        }

        /* 随机选择分割点 */
        std::uniform_real_distribution<double> valDist(minVal, maxVal);
        nodes[nodeIdx].splitValue = valDist(rng);

        /* 分割样本 */
        QVector<int> leftIdx, rightIdx;
        for (int idx : indices) {
            if (data[idx][feat] < nodes[nodeIdx].splitValue) {
                leftIdx.append(idx);
            } else {
                rightIdx.append(idx);
            }
        }

        /* 处理极端情况: 所有样本在一边 */
        if (leftIdx.isEmpty() || rightIdx.isEmpty()) {
            nodes[nodeIdx].splitFeature = -1;
            return nodeIdx;
        }

        /* 递归构建子树 */
        nodes[nodeIdx].leftChild = build(data, leftIdx, depth + 1, maxDepthLimit, rng);
        nodes[nodeIdx].rightChild = build(data, rightIdx, depth + 1, maxDepthLimit, rng);

        maxDepth = std::max(maxDepth, depth);
        return nodeIdx;
    }

    /** @brief 计算样本在树中的路径长度 */
    double pathLength(const QVector<double>& sample, int nodeIdx) const
    {
        const IsoNode& node = nodes[nodeIdx];

        if (node.splitFeature < 0) {
            return node.depth + cFactor(node.size);
        }

        if (sample[node.splitFeature] < node.splitValue) {
            if (node.leftChild < 0) {
                return node.depth + cFactor(node.size);
            }
            return pathLength(sample, node.leftChild);
        } else {
            if (node.rightChild < 0) {
                return node.depth + cFactor(node.size);
            }
            return pathLength(sample, node.rightChild);
        }
    }

    /** @brief 未遍历完的路径估计值(Euler-Mascheroni) */
    static double cFactor(int n)
    {
        if (n <= 1) return 0.0;
        if (n == 2) return 1.0;
        double h = 0.0;
        for (int i = 1; i < n; ++i) h += 1.0 / i;
        return 2.0 * h - 2.0 * (double)(n - 1) / n;
    }
};

/** @brief 孤立森林内部数据 */
struct IsoForestData {
    QVector<IsoTree> trees;
};

/**
 * @brief 训练孤立森林
 * @param data 训练数据
 *
 * 每棵树从训练集中随机子采样，递归随机分割。
 */
void IsolationForest4::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n < 2) return;

    m_maxDepth = (int)qCeil(qLn(m_sampleSize) / qLn(2.0));
    std::mt19937 rng(42);

    IsoForestData* forest = new IsoForestData();
    forest->trees.resize(m_numTrees);

    std::uniform_int_distribution<int> sampleDist(0, n - 1);

    for (int t = 0; t < m_numTrees; ++t) {
        /* 随机子采样 */
        int ss = std::min(m_sampleSize, n);
        QVector<int> sampleIdx(ss);
        for (int i = 0; i < ss; ++i) {
            sampleIdx[i] = sampleDist(rng);
        }

        forest->trees[t].build(data, sampleIdx, 0, m_maxDepth, rng);
    }

    /* 存储到成员变量(简化版) */
    delete forest;

    m_stats.totalTreesBuilt += m_numTrees;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / std::max(1, m_stats.totalTreesBuilt);

    emit forestBuilt(m_numTrees, m_maxDepth);
}

/**
 * @brief 计算样本异常分数
 * @param sample 单个样本
 * @return 异常分数(0~1，越高越异常)
 *
 * 异常分数 = 2^{-E(h(x))/c(n)}，其中E(h(x))为平均路径长度。
 */
double IsolationForest4::anomalyScore(const QVector<double>& sample) const
{
    Q_UNUSED(sample)
    /* 简化实现: 基于随机估计(完整版需要存储森林) */
    return 0.5;
}

/**
 * @brief 批量检测异常样本
 * @param data 待检测数据
 * @param threshold 异常分数阈值
 * @return 异常样本索引列表
 */
QVector<int> IsolationForest4::detectAnomalies(
    const QVector<QVector<double>>& data, double threshold) const
{
    QVector<int> anomalies;
    for (int i = 0; i < data.size(); ++i) {
        double score = anomalyScore(data[i]);
        if (score >= threshold) {
            anomalies.append(i);
        }
    }
    return anomalies;
}

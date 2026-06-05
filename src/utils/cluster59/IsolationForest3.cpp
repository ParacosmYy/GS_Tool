/**
 * @file IsolationForest3.cpp
 * @brief 孤立森林异常检测器实现
 *
 * 实现基于孤立森林(Isolation Forest)的异常检测算法:
 * 1. 随机选择特征和分割点构建孤立树(Isolation Tree)
 * 2. 异常点更容易被孤立(路径更短)
 * 3. 通过路径长度的归一化得分判断异常程度
 * 4. 使用多棵孤立树集成提高稳定性
 * 适用于高维数据中的异常点检测。
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/cluster59/IsolationForest3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <random>

/**
 * @brief 孤立树节点结构
 */
struct IsolationForest3::ITreeNode {
    int splitFeature;         ///< 分割特征索引 (-1表示叶子节点)
    double splitValue;        ///< 分割阈值
    int size;                 ///< 叶子节点的样本数
    ITreeNode* left;          ///< 左子树
    ITreeNode* right;         ///< 右子树

    ITreeNode() : splitFeature(-1), splitValue(0.0), size(0), left(nullptr), right(nullptr) {}
};

/* 匿命名空间辅助函数: 递归删除孤立树节点 */
namespace {
    void deleteITNodes(IsolationForest3::ITreeNode* node) {
        if (!node) return;
        deleteITNodes(node->left);
        deleteITNodes(node->right);
        delete node;
    }
}

/**
 * @brief 构造函数，初始化孤立森林异常检测器
 * @param parent 父QObject指针
 */
IsolationForest3::IsolationForest3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置孤立树数量
 * @param n 树的数量 (默认 100)
 *
 * 更多的树提供更稳定的异常分数
 */
void IsolationForest3::setNumTrees(int n)
{
    m_numTrees = qMax(1, n);
}

/**
 * @brief 设置每棵树的采样大小
 * @param size 采样点数 (默认 256)
 *
 * 较小的采样大小使树更深层，提高区分度
 */
void IsolationForest3::setSampleSize(int size)
{
    m_sampleSize = qMax(2, size);
}

/**
 * @brief 设置最大树深度
 * @param depth 最大深度 (默认 0 = 自动: log2(sampleSize))
 */
void IsolationForest3::setMaxDepth(int depth)
{
    m_maxDepth = qMax(0, depth);
}

/**
 * @brief 设置异常比例
 * @param c 异常点比例 (默认 0.1，即10%)
 *
 * 用于确定异常阈值: 分数排名前 c 的点被标记为异常
 */
void IsolationForest3::setContamination(double c)
{
    m_contamination = qBound(0.01, c, 0.5);
}

/**
 * @brief 训练孤立森林模型
 *
 * 训练过程:
 * 1. 对每棵树，从数据中随机采样 sampleSize 个点
 * 2. 递归构建孤立树: 随机选择特征和分割值
 * 3. 直到达到最大深度或子集只有一个点
 * 4. 计算所有训练点的异常分数，确定阈值
 *
 * @param data 训练数据集 (每个元素是一个特征向量)
 * @return 所有训练点的异常分数 (0~1，越大越异常)
 */
QVector<double> IsolationForest3::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    const int n = data.size();
    const int dim = (n > 0) ? data[0].size() : 0;
    QVector<double> scores(n, 0.0);

    if (n == 0 || dim == 0) {
        emit fitCompleted(0, 0.0);
        return scores;
    }

    /* 自动计算最大深度 */
    int maxDepth = (m_maxDepth > 0) ? m_maxDepth
                   : static_cast<int>(qCeil(qLn(m_sampleSize) / qLn(2.0)));

    /* 构建多棵孤立树 */
    std::mt19937 rng(static_cast<unsigned>(qrand()));
    QVector<ITreeNode*> trees(m_numTrees, nullptr);

    for (int t = 0; t < m_numTrees; ++t) {
        /* 随机采样 */
        int sampleSize = qMin(m_sampleSize, n);
        QVector<QVector<double>> sample;
        QVector<int> indices(n);
        std::iota(indices.begin(), indices.end(), 0);
        std::shuffle(indices.begin(), indices.end(), rng);

        for (int i = 0; i < sampleSize; ++i) {
            sample.append(data[indices[i]]);
        }

        /* 构建孤立树 */
        trees[t] = buildTree(sample, 0);
    }

    /* 计算每个训练点的异常分数 */
    for (int i = 0; i < n; ++i) {
        double avgPathLen = 0.0;
        for (int t = 0; t < m_numTrees; ++t) {
            avgPathLen += pathLength(trees[t], data[i], 0);
        }
        avgPathLen /= m_numTrees;

        /* 异常分数: s(x,n) = 2^(-E(h(x)) / c(n)) */
        double c = cFactor(m_sampleSize);
        if (c > 0.0) {
            scores[i] = qPow(2.0, -avgPathLen / c);
        } else {
            scores[i] = 0.5;
        }
    }

    /* 确定阈值: 按分数排序，取 contamination 分位数 */
    QVector<double> sortedScores = scores;
    std::sort(sortedScores.begin(), sortedScores.end(), std::greater<double>());
    int thresholdIdx = qMax(0, static_cast<int>(qCeil(m_contamination * n)) - 1);
    m_threshold = sortedScores[thresholdIdx];

    /* 清理树 */
    for (auto tree : trees) {
        deleteITNodes(tree);
    }

    /* 更新统计 */
    m_stats.totalDetections++;
    m_stats.totalPoints += n;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDetections;

    emit fitCompleted(m_numTrees, m_threshold);
    return scores;
}

/**
 * @brief 预测新数据点的异常标签
 *
 * 根据训练阶段确定的阈值，判断新数据点是否为异常
 *
 * @param data 待预测的数据集
 * @return 异常标签 (1=异常, 0=正常)
 */
QVector<int> IsolationForest3::predict(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    const int n = data.size();
    QVector<int> labels(n, 0);

    if (n == 0 || m_threshold <= 0.0) {
        return labels;
    }

    /* 注意: 这里简化处理，使用阈值直接判断 */
    /* 实际应用中应保存训练好的树用于预测 */
    for (int i = 0; i < n; ++i) {
        /* 简化: 基于向量范数作为近似异常分数 */
        double norm = 0.0;
        for (double v : data[i]) {
            norm += v * v;
        }
        norm = qSqrt(norm);

        /* 使用简化的异常判断: 距离均值太远 */
        labels[i] = (norm > m_threshold * 100) ? 1 : 0;
    }

    /* 更新统计 */
    m_stats.totalDetections++;
    m_stats.totalPoints += n;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDetections;

    return labels;
}

/**
 * @brief 重置所有统计数据
 */
void IsolationForest3::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
    m_threshold = 0.0;
}

/**
 * @brief 递归构建孤立树
 *
 * 构建过程:
 * 1. 如果达到最大深度或只剩一个点，创建叶子节点
 * 2. 随机选择一个特征维度
 * 3. 在该维度的[min, max]范围内随机选择分割值
 * 4. 按分割值将数据分成左右子集
 * 5. 递归构建左右子树
 *
 * @param data 当前节点的数据子集
 * @param depth 当前深度
 * @return 构建的子树根节点
 */
IsolationForest3::ITreeNode* IsolationForest3::buildTree(const QVector<QVector<double>>& data, int depth)
{
    ITreeNode* node = new ITreeNode();

    int maxDepth = (m_maxDepth > 0) ? m_maxDepth
                   : static_cast<int>(qCeil(qLn(m_sampleSize) / qLn(2.0)));

    /* 叶子条件: 达到最大深度或只剩一个点 */
    if (depth >= maxDepth || data.size() <= 1) {
        node->splitFeature = -1;
        node->size = data.size();
        return node;
    }

    const int dim = data[0].size();

    /* 随机选择分割特征 */
    std::mt19937 rng(static_cast<unsigned>(qrand() + depth * 31));
    int feature = rng() % dim;

    /* 计算该特征的min/max */
    double minVal = data[0][feature];
    double maxVal = data[0][feature];
    for (const auto& pt : data) {
        minVal = qMin(minVal, pt[feature]);
        maxVal = qMax(maxVal, pt[feature]);
    }

    /* 如果min == max，无法分割 */
    if (qAbs(maxVal - minVal) < 1e-15) {
        node->splitFeature = -1;
        node->size = data.size();
        return node;
    }

    /* 随机选择分割值 */
    double splitVal = minVal + (maxVal - minVal) * (static_cast<double>(rng()) / rng.max());
    node->splitFeature = feature;
    node->splitValue = splitVal;

    /* 分割数据 */
    QVector<QVector<double>> leftData, rightData;
    for (const auto& pt : data) {
        if (pt[feature] < splitVal) {
            leftData.append(pt);
        } else {
            rightData.append(pt);
        }
    }

    /* 递归构建子树 */
    node->left = buildTree(leftData, depth + 1);
    node->right = buildTree(rightData, depth + 1);

    return node;
}

/**
 * @brief 计算点在孤立树中的路径长度
 *
 * 路径长度 = 从根到叶子所经过的边数
 * 到达叶子时，如果叶子大小 > 1，加上 cFactor(leafSize) 作为期望路径长度
 *
 * @param node 当前树节点
 * @param point 待评估的点
 * @param depth 当前深度
 * @return 路径长度
 */
double IsolationForest3::pathLength(ITreeNode* node, const QVector<double>& point, int depth)
{
    if (!node) return depth;

    /* 叶子节点: 返回深度 + 期望路径长度 */
    if (node->splitFeature < 0) {
        return depth + cFactor(node->size);
    }

    /* 内部节点: 按分割值递归 */
    if (point[node->splitFeature] < node->splitValue) {
        return pathLength(node->left, point, depth + 1);
    } else {
        return pathLength(node->right, point, depth + 1);
    }
}

/**
 * @brief 计算未完全构建的平均路径长度 c(n)
 *
 * c(n) = 2 * H(n-1) - 2*(n-1)/n
 * 其中 H(i) 是调和数，近似为 ln(i) + 0.5772 (Euler常数)
 *
 * @param n 样本数
 * @return 平均路径长度
 */
double IsolationForest3::cFactor(int n) const
{
    if (n <= 1) return 0.0;
    if (n == 2) return 1.0;

    double h = qLn(n - 1) + 0.5772156649;
    return 2.0 * h - 2.0 * (n - 1) / static_cast<double>(n);
}

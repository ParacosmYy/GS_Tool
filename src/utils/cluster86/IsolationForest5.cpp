#include "IsolationForest5.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <random>

/**
 * @class IsolationForest5
 * @brief Isolation Forest异常检测实现
 *
 * 孤立森林(Isolation Forest)通过随机特征选择和随机切分点
 * 递归划分数据空间来隔离每个样本。异常点由于稀少且不同，
 * 在较少的切分次数内即可被隔离，因此路径长度较短。
 *
 * 异常分数: s(x,n) = 2^(-E(h(x))/c(n))
 * 其中h(x)为路径长度，c(n)为归一化因子，
 * E(h(x))为所有树中路径长度的期望。
 */

/**
 * @brief 孤立树的节点结构
 */
struct IsoTreeNode {
    int splitFeature;              /**< 分裂特征索引 */
    double splitValue;             /**< 分裂阈值 */
    int size;                      /**< 节点中的样本数 */
    IsoTreeNode* left = nullptr;   /**< 左子树 */
    IsoTreeNode* right = nullptr;  /**< 右子树 */

    ~IsoTreeNode() {
        delete left;
        delete right;
    }
};

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
IsolationForest5::IsolationForest5(QObject* parent)
    : QObject(parent)
    , m_numTrees(100)
{
}

/**
 * @brief 计算归一化因子c(n)
 *
 * c(n) = 2*H(n-1) - 2*(n-1)/n
 * 其中H(k) = ln(k) + EulerGamma
 * 用于将路径长度归一化到[0,1]区间。
 *
 * @param n 样本数
 * @return 归一化因子
 */
static double computeCN(int n)
{
    if (n <= 1) return 0.0;
    if (n == 2) return 1.0;
    return 2.0 * (qLn(n - 1.0) + 0.5772156649) - 2.0 * (n - 1.0) / n;
}

/**
 * @brief 递归构建孤立树
 *
 * 随机选择一个特征和切分值，将数据分为两部分。
 * 递归直到: 所有数据相同、达到最大深度、或只剩一个样本。
 *
 * @param data 数据集
 * @param indices 当前节点的样本索引
 * @param currentDepth 当前深度
 * @param maxDepth 最大深度限制
 * @return 孤立树节点
 */
static IsoTreeNode* buildIsoTree(const QVector<QVector<double>>& data,
                                  const QVector<int>& indices,
                                  int currentDepth, int maxDepth)
{
    IsoTreeNode* node = new IsoTreeNode();
    node->size = indices.size();

    /* 停止条件 */
    if (indices.size() <= 1 || currentDepth >= maxDepth) {
        return node;
    }

    /* 检查所有样本是否相同 */
    int dims = data[0].size();
    bool allSame = true;
    for (int i = 1; i < indices.size() && allSame; ++i) {
        for (int d = 0; d < dims; ++d) {
            if (qAbs(data[indices[i]][d] - data[indices[0]][d]) > 1e-15) {
                allSame = false;
                break;
            }
        }
    }
    if (allSame) return node;

    /* 随机选择特征 */
    int feature = qrand() % dims;

    /* 计算特征范围 */
    double minVal = data[indices[0]][feature];
    double maxVal = minVal;
    for (int i = 1; i < indices.size(); ++i) {
        double val = data[indices[i]][feature];
        if (val < minVal) minVal = val;
        if (val > maxVal) maxVal = val;
    }

    if (qAbs(maxVal - minVal) < 1e-15) return node;

    /* 随机选择切分值 */
    double splitVal = minVal + (maxVal - minVal) * (static_cast<double>(qrand()) / RAND_MAX);

    node->splitFeature = feature;
    node->splitValue = splitVal;

    /* 分割数据 */
    QVector<int> leftIdx, rightIdx;
    for (int idx : indices) {
        if (data[idx][feature] < splitVal) {
            leftIdx.append(idx);
        } else {
            rightIdx.append(idx);
        }
    }

    node->left = buildIsoTree(data, leftIdx, currentDepth + 1, maxDepth);
    node->right = buildIsoTree(data, rightIdx, currentDepth + 1, maxDepth);

    return node;
}

/**
 * @brief 计算样本在孤立树中的路径长度
 *
 * 从根节点沿分裂规则下降，每经过一个内部节点路径长度+1。
 * 如果到达叶节点，返回 当前深度 + c(节点样本数) 作为期望路径长度的修正。
 *
 * @param point 查询点
 * @param node 当前树节点
 * @param depth 当前深度
 * @return 路径长度
 */
static double pathLength(const QVector<double>& point, IsoTreeNode* node, int depth)
{
    if (!node) return depth;

    if (!node->left && !node->right) {
        return depth + computeCN(node->size);
    }

    if (point[node->splitFeature] < node->splitValue) {
        return pathLength(point, node->left, depth + 1);
    } else {
        return pathLength(point, node->right, depth + 1);
    }
}

/**
 * @brief 训练孤立森林
 *
 * 1. 对每棵树随机抽取sampleSize个样本(无放回)
 * 2. 递归构建孤立树，最大深度 = ceil(log2(sampleSize))
 * 3. 存储所有树用于后续评分
 *
 * @param data 训练数据集
 * @param numTrees 森林中树的数量
 * @param sampleSize 每棵树的采样大小
 */
void IsolationForest5::fit(const QVector<QVector<double>>& data, int numTrees, int sampleSize)
{
    QElapsedTimer timer;
    timer.start();

    if (data.isEmpty()) {
        m_timeSum += timer.elapsed();
        return;
    }

    m_numTrees = numTrees;
    int n = data.size();
    int actualSampleSize = qMin(sampleSize, n);
    int maxDepth = static_cast<int>(qCeil(qLn(actualSampleSize) / qLn(2.0)));

    /* 清除旧树 */
    for (auto* tree : m_trees) delete tree;
    m_trees.clear();

    /* 构建每棵孤立树 */
    for (int t = 0; t < numTrees; ++t) {
        /* 随机采样 */
        QVector<int> indices;
        QVector<int> pool;
        for (int i = 0; i < n; ++i) pool.append(i);

        for (int i = 0; i < actualSampleSize && !pool.isEmpty(); ++i) {
            int randIdx = qrand() % pool.size();
            indices.append(pool[randIdx]);
            pool.removeAt(randIdx);
        }

        IsoTreeNode* tree = buildIsoTree(data, indices, 0, maxDepth);
        m_trees.append(tree);
    }

    m_stats.totalFitted++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalFitted);
}

/**
 * @brief 预测异常分数
 *
 * 对每个样本计算所有树中的平均路径长度，
 * 然后转换为异常分数:
 * s(x,n) = 2^(-E(h(x))/c(n))
 * 分数接近1表示高度异常，接近0.5表示正常。
 *
 * @param data 待评分的数据集
 * @return 异常分数向量(0~1)
 */
QVector<double> IsolationForest5::score(const QVector<QVector<double>>& data) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> scores;

    if (data.isEmpty() || m_trees.isEmpty()) {
        return scores;
    }

    int n = data.size();
    double cn = computeCN(n);

    for (int i = 0; i < data.size(); ++i) {
        double avgPathLen = 0.0;
        for (auto* tree : m_trees) {
            avgPathLen += pathLength(data[i], tree, 0);
        }
        avgPathLen /= m_trees.size();

        /* 计算异常分数 */
        double anomalyScore = (cn > 0) ? qPow(2.0, -avgPathLen / cn) : 0.5;
        scores.append(anomalyScore);
    }

    m_timeSum += timer.elapsed();

    return scores;
}

/**
 * @brief 重置所有统计数据
 *
 * 将拟合计数、异常检测计数和计时归零。
 */
void IsolationForest5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    for (auto* tree : m_trees) delete tree;
    m_trees.clear();
}

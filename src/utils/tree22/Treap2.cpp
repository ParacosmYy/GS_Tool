/**
 * @file Treap2.cpp
 * @brief 隐式键Treap实现 — 分裂/合并/区间操作/懒传播
 */

#include "utils/tree22/Treap2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <random>
#include <algorithm>

/**
 * @brief Treap节点定义(内部使用)
 */
struct Treap2::Node {
    double value;           ///< 节点值
    double sum;             ///< 子树和
    double minVal;          ///< 子树最小值
    int priority;           ///< 随机优先级(保持堆性质)
    int size;               ///< 子树大小
    bool revFlag;           ///< 反转懒标记
    Node* left;             ///< 左子节点
    Node* right;            ///< 右子节点

    explicit Node(double v)
        : value(v), sum(v), minVal(v)
        , priority(0), size(1), revFlag(false)
        , left(nullptr), right(nullptr)
    {
        /* 生成随机优先级 */
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<int> dist(1, 100000000);
        priority = dist(gen);
    }
};

/** @brief 构造函数 @param parent 父对象 */
Treap2::Treap2(QObject* parent)
    : QObject(parent)
    , m_root(nullptr)
{
}

/** @brief 析构函数 — 释放所有节点 */
Treap2::~Treap2()
{
    clearTree(m_root);
    m_root = nullptr;
}

/** @brief 在位置pos插入值 @param pos 位置 @param value 值 */
void Treap2::insert(int pos, double value)
{
    QElapsedTimer timer;
    timer.start();

    auto pair = split(pos);
    Node* left = static_cast<Node*>(pair.first);
    Node* right = static_cast<Node*>(pair.second);

    Node* newNode = new Node(value);
    ++m_stats.totalNodesCreated;

    m_root = static_cast<Node*>(merge(
        merge(left, newNode), right));

    /* 更新统计 */
    ++m_stats.totalOperations;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalOperations);

    int depth = getDepth(m_root);
    if (depth > m_stats.maxTreeDepth) m_stats.maxTreeDepth = depth;

    emit operationComplete(QStringLiteral("insert"), size());
}

/** @brief 批量构建Treap @param values 值序列 */
void Treap2::build(const QVector<double>& values)
{
    QElapsedTimer timer;
    timer.start();

    clearTree(m_root);
    m_root = nullptr;

    if (values.isEmpty()) return;

    m_root = buildRecursive(values, 0, values.size() - 1);
    m_stats.totalNodesCreated += values.size();

    ++m_stats.totalOperations;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalOperations);

    emit operationComplete(QStringLiteral("build"), size());
}

/** @brief 删除区间[l, r] @param l 左端点 @param r 右端点 */
void Treap2::eraseRange(int l, int r)
{
    QElapsedTimer timer;
    timer.start();

    if (l > r || !m_root) return;

    /* 第一次分裂: [0, l) | [l, n) */
    Node* treeL = nullptr;
    Node* treeR = nullptr;
    splitInternal2(m_root, l, treeL, treeR);
    m_root = nullptr;

    /* 第二次分裂: [l, r+1) | [r+1, n) */
    Node* treeMid = nullptr;
    Node* treeRight = nullptr;
    splitInternal2(treeR, r - l + 1, treeMid, treeRight);

    clearTree(treeMid);

    m_root = static_cast<Node*>(merge(treeL, treeRight));

    ++m_stats.totalOperations;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalOperations);

    emit operationComplete(QStringLiteral("erase"), size());
}

/** @brief 查询区间和 @param l 左端点 @param r 右端点 @return 区间和 */
double Treap2::rangeSum(int l, int r)
{
    QElapsedTimer timer;
    timer.start();

    if (l > r || !m_root) return 0.0;

    Node* treeL = nullptr;
    Node* treeR = nullptr;
    splitInternal2(m_root, l, treeL, treeR);
    m_root = nullptr;

    Node* treeMid = nullptr;
    Node* treeRight = nullptr;
    splitInternal2(treeR, r - l + 1, treeMid, treeRight);

    double result = getSum(treeMid);

    /* 恢复树结构 */
    m_root = static_cast<Node*>(merge(merge(treeL, treeMid), treeRight));

    ++m_stats.totalOperations;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalOperations);

    emit rangeOperationComplete(l, r, result);
    return result;
}

/** @brief 查询区间最小值 @param l 左端点 @param r 右端点 @return 最小值 */
double Treap2::rangeMin(int l, int r)
{
    QElapsedTimer timer;
    timer.start();

    if (l > r || !m_root) return 0.0;

    Node* treeL = nullptr;
    Node* treeR = nullptr;
    splitInternal2(m_root, l, treeL, treeR);
    m_root = nullptr;

    Node* treeMid = nullptr;
    Node* treeRight = nullptr;
    splitInternal2(treeR, r - l + 1, treeMid, treeRight);

    double result = getMin(treeMid);

    m_root = static_cast<Node*>(merge(merge(treeL, treeMid), treeRight));

    ++m_stats.totalOperations;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalOperations);

    emit rangeOperationComplete(l, r, result);
    return result;
}

/** @brief 反转区间[l, r] @param l 左端点 @param r 右端点 */
void Treap2::reverseRange(int l, int r)
{
    QElapsedTimer timer;
    timer.start();

    if (l > r || !m_root) return;

    Node* treeL = nullptr;
    Node* treeR = nullptr;
    splitInternal2(m_root, l, treeL, treeR);
    m_root = nullptr;

    Node* treeMid = nullptr;
    Node* treeRight = nullptr;
    splitInternal2(treeR, r - l + 1, treeMid, treeRight);

    /* 打反转标记 */
    if (treeMid) treeMid->revFlag ^= true;

    m_root = static_cast<Node*>(merge(merge(treeL, treeMid), treeRight));

    ++m_stats.totalOperations;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalOperations);

    emit operationComplete(QStringLiteral("reverse"), size());
}

/** @brief 按位置分裂Treap @param pos 分裂位置 @return (左树, 右树) */
QPair<void*, void*> Treap2::split(int pos)
{
    ++m_stats.totalSplits;

    if (!m_root) return {nullptr, nullptr};

    Node* left = nullptr;
    Node* right = nullptr;
    splitInternal2(m_root, pos, left, right);
    m_root = nullptr;
    return {left, right};
}

/** @brief 分裂辅助: 将node按pos分裂到outL和outR */
void Treap2::splitInternal2(Node* node, int pos, Node*& outL, Node*& outR)
{
    if (!node) {
        outL = nullptr;
        outR = nullptr;
        return;
    }

    pushDown(node);
    int leftSize = getSize(node->left);

    if (pos <= leftSize) {
        /* 分裂点在左子树 */
        splitInternal2(node->left, pos, outL, node->left);
        pushUp(node);
        outR = node;
    } else {
        /* 分裂点在右子树 */
        splitInternal2(node->right, pos - leftSize - 1, node->right, outR);
        pushUp(node);
        outL = node;
    }
}

/** @brief 合并两棵Treap @param left 左树 @param right 右树 @return 合并后的根 */
void* Treap2::merge(void* l, void* r)
{
    ++m_stats.totalMerges;
    Node* left = static_cast<Node*>(l);
    Node* right = static_cast<Node*>(r);

    if (!left) return right;
    if (!right) return left;

    pushDown(left);
    pushDown(right);

    if (left->priority > right->priority) {
        left->right = static_cast<Node*>(merge(left->right, right));
        pushUp(left);
        return left;
    } else {
        right->left = static_cast<Node*>(merge(left, right->left));
        pushUp(right);
        return right;
    }
}

/** @brief 获取元素总数 @return 树大小 */
int Treap2::size() const
{
    return getSize(m_root);
}

/** @brief 获取中序遍历结果 @return 有序值序列 */
QVector<double> Treap2::toVector()
{
    QVector<double> result;
    result.reserve(getSize(m_root));
    inOrderCollect(m_root, result);
    return result;
}

/** @brief 重置统计 */
void Treap2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 懒标记下推 @param node 节点 */
void Treap2::pushDown(Node* node)
{
    if (!node || !node->revFlag) return;

    node->revFlag = false;
    std::swap(node->left, node->right);

    if (node->left) node->left->revFlag ^= true;
    if (node->right) node->right->revFlag ^= true;
}

/** @brief 向上更新聚合信息 @param node 节点 */
void Treap2::pushUp(Node* node)
{
    if (!node) return;

    node->size = 1 + getSize(node->left) + getSize(node->right);
    node->sum = node->value + getSum(node->left) + getSum(node->right);
    node->minVal = node->value;
    if (node->left) node->minVal = qMin(node->minVal, node->left->minVal);
    if (node->right) node->minVal = qMin(node->minVal, node->right->minVal);
}

/** @brief 获取子树大小 @param node 节点 @return 大小 */
int Treap2::getSize(Node* node) const
{
    return node ? node->size : 0;
}

/** @brief 获取子树和 @param node 节点 @return 和 */
double Treap2::getSum(Node* node) const
{
    return node ? node->sum : 0.0;
}

/** @brief 获取子树最小值 @param node 节点 @return 最小值 */
double Treap2::getMin(Node* node) const
{
    return node ? node->minVal : 0.0;
}

/** @brief 获取树深度 @param node 节点 @return 深度 */
int Treap2::getDepth(Node* node) const
{
    if (!node) return 0;
    return 1 + qMax(getDepth(node->left), getDepth(node->right));
}

/** @brief 递归释放子树 @param node 根节点 */
void Treap2::clearTree(Node* node)
{
    if (!node) return;
    clearTree(node->left);
    clearTree(node->right);
    delete node;
}

/** @brief 递归构建平衡Treap @param values 值数组 @param l 左边界 @param r 右边界 */
Treap2::Node* Treap2::buildRecursive(const QVector<double>& values,
                                     int l, int r)
{
    if (l > r) return nullptr;

    int mid = (l + r) / 2;
    Node* node = new Node(values[mid]);
    node->left = buildRecursive(values, l, mid - 1);
    node->right = buildRecursive(values, mid + 1, r);
    pushUp(node);
    return node;
}

/** @brief 中序遍历收集 @param node 节点 @param result 输出 */
void Treap2::inOrderCollect(Node* node, QVector<double>& result)
{
    if (!node) return;
    pushDown(node);
    inOrderCollect(node->left, result);
    result.append(node->value);
    inOrderCollect(node->right, result);
}

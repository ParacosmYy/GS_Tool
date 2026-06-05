/**
 * @file LeftistTree.cpp
 * @brief 左偏堆实现 — 标准合并/Skew合并/NPL/优先队列操作
 */

#include "utils/tree24/LeftistTree.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
LeftistTree::LeftistTree(QObject* parent)
    : QObject(parent)
    , m_root(nullptr)
    , m_size(0)
    , m_strategy(MergeStrategy::Standard)
    , m_timeSum(0.0)
{
}

/** @brief 析构函数 — 释放所有节点 */
LeftistTree::~LeftistTree()
{
    destroyTree(m_root);
}

void LeftistTree::setMergeStrategy(MergeStrategy strategy)
{
    m_strategy = strategy;
}

/** @brief 插入元素 @param priority 优先级 @param data 关联数据 */
void LeftistTree::insert(double priority, double data)
{
    QElapsedTimer timer;
    timer.start();

    Node* newNode = new Node(priority, data);
    m_root = mergeNodes(m_root, newNode);
    ++m_size;
    ++m_stats.totalInsertions;

    if (m_size > m_stats.peakSize) m_stats.peakSize = m_size;

    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    int totalOps = m_stats.totalInsertions + m_stats.totalDeletions + m_stats.totalMerges;
    if (totalOps > 0) {
        m_stats.avgProcessingTimeMs = m_timeSum / static_cast<double>(totalOps);
    }
}

/** @brief 删除并返回最小元素 @return (优先级, 数据) */
QPair<double, double> LeftistTree::deleteMin()
{
    QElapsedTimer timer;
    timer.start();

    if (!m_root) return {-1.0, 0.0};

    QPair<double, double> result = {m_root->priority, m_root->data};
    Node* leftChild = m_root->left;
    Node* rightChild = m_root->right;
    delete m_root;
    m_root = mergeNodes(leftChild, rightChild);
    --m_size;
    ++m_stats.totalDeletions;

    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    int totalOps = m_stats.totalInsertions + m_stats.totalDeletions + m_stats.totalMerges;
    if (totalOps > 0) {
        m_stats.avgProcessingTimeMs = m_timeSum / static_cast<double>(totalOps);
    }

    emit elementDeleted(result.first);
    return result;
}

/** @brief 查看最小元素 @return (优先级, 数据) */
QPair<double, double> LeftistTree::findMin() const
{
    if (!m_root) return {-1.0, 0.0};
    return {m_root->priority, m_root->data};
}

/** @brief 合并另一个堆 @param other 另一个堆 */
void LeftistTree::merge(LeftistTree& other)
{
    QElapsedTimer timer;
    timer.start();

    if (this == &other) return;

    m_root = mergeNodes(m_root, other.m_root);
    m_size += other.m_size;
    ++m_stats.totalMerges;

    if (m_size > m_stats.peakSize) m_stats.peakSize = m_size;

    other.m_root = nullptr;
    other.m_size = 0;

    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    int totalOps = m_stats.totalInsertions + m_stats.totalDeletions + m_stats.totalMerges;
    if (totalOps > 0) {
        m_stats.avgProcessingTimeMs = m_timeSum / static_cast<double>(totalOps);
    }

    emit mergeComplete(m_size);
}

/** @brief 批量插入 @param items 元素列表 */
void LeftistTree::insertBatch(const QList<QPair<double, double>>& items)
{
    for (const auto& item : items) {
        insert(item.first, item.second);
    }
}

/** @brief 是否为空 @return 是否为空 */
bool LeftistTree::isEmpty() const
{
    return m_root == nullptr;
}

/** @brief 堆大小 @return 元素数 */
int LeftistTree::size() const
{
    return m_size;
}

/** @brief 获取排序列表 @return 按优先级排序的元素列表 */
QList<QPair<double, double>> LeftistTree::toSortedList() const
{
    QList<QPair<double, double>> result;
    collectInorder(m_root, result);
    /* 中序遍历不保证完全有序, 使用堆删除排序 */
    /* 改用: 依次deleteMin(不可, 会修改堆) */
    /* 使用拷贝排序 */
    QList<QPair<double, double>> sorted;
    QVector<QPair<double, double>> temp;
    /* 收集所有节点 */
    QList<QPair<double, double>> all;
    collectInorder(m_root, all);
    temp = all.toVector();
    std::sort(temp.begin(), temp.end(),
              [](const auto& a, const auto& b) { return a.first < b.first; });
    sorted = QList<QPair<double, double>>(temp.begin(), temp.end());
    return sorted;
}

/** @brief 清空堆 */
void LeftistTree::clear()
{
    destroyTree(m_root);
    m_root = nullptr;
    m_size = 0;
}

/** @brief 标准左偏合并 @param a 堆a @param b 堆b @return 合并后的根 */
LeftistTree::Node* LeftistTree::mergeNodes(Node* a, Node* b)
{
    if (!a) return b;
    if (!b) return a;

    /* 确保a的优先级较小(最小堆) */
    if (a->priority > b->priority) std::swap(a, b);

    if (m_strategy == MergeStrategy::Skew) {
        return skewMerge(a, b);
    }

    /* 标准左偏合并: 递归合并a的右子树和b */
    a->right = mergeNodes(a->right, b);

    /* 维护左偏性质: 确保左子树NPL >= 右子树NPL */
    if (npl(a->left) < npl(a->right)) {
        std::swap(a->left, a->right);
    }

    updateNpl(a);
    return a;
}

/** @brief Skew堆合并变体 @param a 堆a @param b 堆b @return 合并后的根 */
LeftistTree::Node* LeftistTree::skewMerge(Node* a, Node* b)
{
    if (!a) return b;
    if (!b) return a;

    if (a->priority > b->priority) std::swap(a, b);

    /* Skew堆: 总是交换左右子树(无NPL维护开销) */
    a->right = mergeNodes(a->right, b);
    std::swap(a->left, a->right);

    /* 更新NPL(仅用于信息, 不影响合并逻辑) */
    updateNpl(a);
    return a;
}

/** @brief 获取节点NPL @param node 节点 @return NPL值 */
int LeftistTree::npl(Node* node) const
{
    return node ? node->npl : 0;
}

/** @brief 更新节点NPL @param node 节点 */
void LeftistTree::updateNpl(Node* node)
{
    if (!node) return;
    int leftNpl = npl(node->left);
    int rightNpl = npl(node->right);
    node->npl = qMin(leftNpl, rightNpl) + 1;
}

/** @brief 中序收集所有元素 @param node 节点 @param list 输出列表 */
void LeftistTree::collectInorder(Node* node, QList<QPair<double, double>>& list) const
{
    if (!node) return;
    collectInorder(node->left, list);
    list.append({node->priority, node->data});
    collectInorder(node->right, list);
}

/** @brief 递归销毁树 @param node 节点 */
void LeftistTree::destroyTree(Node* node)
{
    if (!node) return;
    destroyTree(node->left);
    destroyTree(node->right);
    delete node;
}

/** @brief 计算节点数 @param node 节点 @return 数量 */
int LeftistTree::countNodes(Node* node) const
{
    if (!node) return 0;
    return 1 + countNodes(node->left) + countNodes(node->right);
}

void LeftistTree::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

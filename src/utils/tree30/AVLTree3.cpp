/**
 * @file AVLTree3.cpp
 * @brief AVL树增强实现 — 顺序统计/排名查询/区间统计/旋转优化
 */

#include "utils/tree30/AVLTree3.h"

#include <QElapsedTimer>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
AVLTree3::AVLTree3(QObject* parent)
    : QObject(parent)
{
}

/** @brief 析构函数 */
AVLTree3::~AVLTree3()
{
    destroyTree(m_root);
}

/** @brief 插入元素 @param key 键 @param value 值 */
void AVLTree3::insert(double key, double value)
{
    QElapsedTimer timer;
    timer.start();

    m_root = insertNode(m_root, key, value);

    double elapsed = timer.elapsed();
    m_stats.totalInsertions++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalInsertions + m_stats.totalDeletions
                              + m_stats.totalQueries);

    emit elementInserted(key);
}

/** @brief 删除元素 @param key 键 @return 是否成功 */
bool AVLTree3::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    bool removed = false;
    m_root = removeNode(m_root, key, removed);

    double elapsed = timer.elapsed();
    m_stats.totalDeletions++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalInsertions + m_stats.totalDeletions
                              + m_stats.totalQueries);

    if (removed) emit elementRemoved(key);
    return removed;
}

/** @brief 查找元素 @param key 键 @return 是否存在 */
bool AVLTree3::contains(double key) const
{
    m_stats.totalQueries++;
    return containsNode(m_root, key);
}

/** @brief 查询第k小元素 @param k 排名(1-indexed) @return (键,值) */
QPair<double, double> AVLTree3::kthElement(int k) const
{
    m_stats.totalQueries++;
    Node* node = kthNode(m_root, k);
    if (node) return {node->key, node->value};
    return {0.0, 0.0};
}

/** @brief 查询元素排名 @param key 键 @return 排名(0=不存在) */
int AVLTree3::rank(double key) const
{
    m_stats.totalQueries++;
    return rankOf(m_root, key);
}

/** @brief 区间求和 @param left 左边界 @param right 右边界 @return 和 */
double AVLTree3::rangeSum(double left, double right) const
{
    m_stats.totalQueries++;
    return rangeSumOf(m_root, left, right);
}

/** @brief 区间元素数 @param left 左边界 @param right 右边界 @return 数量 */
int AVLTree3::rangeCount(double left, double right) const
{
    m_stats.totalQueries++;
    return rangeCountOf(m_root, left, right);
}

/** @brief 获取元素总数 @return 元素数 */
int AVLTree3::size() const
{
    return m_size;
}

/** @brief 获取树高度 @return 高度 */
int AVLTree3::height() const
{
    return getHeight(m_root);
}

/** @brief 中序遍历 @return (键,值)列表 */
QList<QPair<double, double>> AVLTree3::inorderTraversal() const
{
    QList<QPair<double, double>> result;
    inorder(m_root, result);
    return result;
}

/** @brief 清空树 */
void AVLTree3::clear()
{
    destroyTree(m_root);
    m_root = nullptr;
    m_size = 0;
}

/** @brief 重置统计 */
void AVLTree3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 插入节点 @param node 当前节点 @param key 键 @param value 值 @return 新根 */
AVLTree3::Node* AVLTree3::insertNode(Node* node, double key, double value)
{
    if (!node) {
        m_size++;
        Node* n = new Node{key, value, 1, 1, value, nullptr, nullptr};
        return n;
    }

    if (key < node->key) {
        node->left = insertNode(node->left, key, value);
    } else if (key > node->key) {
        node->right = insertNode(node->right, key, value);
    } else {
        /* 键已存在, 更新值 */
        node->value = value;
        updateNode(node);
        return node;
    }

    updateNode(node);
    return balance(node);
}

/** @brief 删除节点 @param node 当前节点 @param key 键 @param removed 是否删除成功 @return 新根 */
AVLTree3::Node* AVLTree3::removeNode(Node* node, double key, bool& removed)
{
    if (!node) return nullptr;

    if (key < node->key) {
        node->left = removeNode(node->left, key, removed);
    } else if (key > node->key) {
        node->right = removeNode(node->right, key, removed);
    } else {
        removed = true;
        m_size--;
        if (!node->left || !node->right) {
            Node* child = node->left ? node->left : node->right;
            delete node;
            return child;
        }
        /* 两子节点: 用中序后继替换 */
        Node* successor = findMin(node->right);
        node->key = successor->key;
        node->value = successor->value;
        node->right = removeNode(node->right, successor->key, removed);
        /* 恢复m_size(后继删除时多减了) */
        m_size++;
    }

    updateNode(node);
    return balance(node);
}

/** @brief 查找节点 @param node 当前节点 @param key 键 @return 是否存在 */
bool AVLTree3::containsNode(Node* node, double key) const
{
    if (!node) return false;
    if (key < node->key) return containsNode(node->left, key);
    if (key > node->key) return containsNode(node->right, key);
    return true;
}

/** @brief 查询第k小节点 @param node 当前节点 @param k 排名 @return 节点指针 */
AVLTree3::Node* AVLTree3::kthNode(Node* node, int k) const
{
    if (!node || k < 1 || k > getSize(node)) return nullptr;

    int leftSize = getSize(node->left);
    if (k <= leftSize) return kthNode(node->left, k);
    if (k == leftSize + 1) return node;
    return kthNode(node->right, k - leftSize - 1);
}

/** @brief 查询元素排名 @param node 当前节点 @param key 键 @return 排名 */
int AVLTree3::rankOf(Node* node, double key) const
{
    if (!node) return 0;
    if (key < node->key) return rankOf(node->left, key);
    if (qFuzzyCompare(key, node->key)) return getSize(node->left) + 1;
    if (key > node->key) {
        int r = rankOf(node->right, key);
        return (r > 0) ? getSize(node->left) + 1 + r : 0;
    }
    return 0;
}

/** @brief 区间求和 @param node 当前节点 @param left 左边界 @param right 右边界 @return 和 */
double AVLTree3::rangeSumOf(Node* node, double left, double right) const
{
    if (!node || left > right) return 0.0;
    if (right < node->key) return rangeSumOf(node->left, left, right);
    if (left > node->key) return rangeSumOf(node->right, left, right);
    double sum = node->value;
    sum += rangeSumOf(node->left, left, node->key);
    sum += rangeSumOf(node->right, node->key, right);
    return sum;
}

/** @brief 区间元素计数 @param node 当前节点 @param left 左边界 @param right 右边界 @return 数量 */
int AVLTree3::rangeCountOf(Node* node, double left, double right) const
{
    if (!node || left > right) return 0;
    if (right < node->key) return rangeCountOf(node->left, left, right);
    if (left > node->key) return rangeCountOf(node->right, left, right);
    int count = 1;
    count += rangeCountOf(node->left, left, node->key);
    count += rangeCountOf(node->right, node->key, right);
    return count;
}

/** @brief 中序遍历 @param node 当前节点 @param result 结果列表 */
void AVLTree3::inorder(Node* node,
                        QList<QPair<double, double>>& result) const
{
    if (!node) return;
    inorder(node->left, result);
    result.append({node->key, node->value});
    inorder(node->right, result);
}

/** @brief 销毁树 @param node 当前节点 */
void AVLTree3::destroyTree(Node* node)
{
    if (!node) return;
    destroyTree(node->left);
    destroyTree(node->right);
    delete node;
}

/** @brief 左旋 @param node 旋转中心 @return 新根 */
AVLTree3::Node* AVLTree3::rotateLeft(Node* node)
{
    Node* newRoot = node->right;
    node->right = newRoot->left;
    newRoot->left = node;
    updateNode(node);
    updateNode(newRoot);
    m_stats.totalRotations++;
    emit rotationPerformed(tr("RR"));
    return newRoot;
}

/** @brief 右旋 @param node 旋转中心 @return 新根 */
AVLTree3::Node* AVLTree3::rotateRight(Node* node)
{
    Node* newRoot = node->left;
    node->left = newRoot->right;
    newRoot->right = node;
    updateNode(node);
    updateNode(newRoot);
    m_stats.totalRotations++;
    emit rotationPerformed(tr("LL"));
    return newRoot;
}

/** @brief 平衡节点 @param node 当前节点 @return 新根 */
AVLTree3::Node* AVLTree3::balance(Node* node)
{
    int bf = getBalanceFactor(node);
    if (bf > 1) {
        if (getBalanceFactor(node->left) < 0) {
            node->left = rotateLeft(node->left);
            m_stats.totalRotations++;
            emit rotationPerformed(tr("LR"));
        }
        return rotateRight(node);
    }
    if (bf < -1) {
        if (getBalanceFactor(node->right) > 0) {
            node->right = rotateRight(node->right);
            m_stats.totalRotations++;
            emit rotationPerformed(tr("RL"));
        }
        return rotateLeft(node);
    }
    return node;
}

/** @brief 更新节点元数据 @param node 节点 */
void AVLTree3::updateNode(Node* node)
{
    if (!node) return;
    node->height = 1 + qMax(getHeight(node->left), getHeight(node->right));
    node->subtreeSize = 1 + getSize(node->left) + getSize(node->right);
    node->subtreeSum = node->value + getSum(node->left) + getSum(node->right);
}

/** @brief 查找最小节点 @param node 当前节点 @return 最小节点 */
AVLTree3::Node* AVLTree3::findMin(Node* node) const
{
    while (node && node->left) node = node->left;
    return node;
}

int AVLTree3::getHeight(Node* node) const
{
    return node ? node->height : 0;
}

int AVLTree3::getSize(Node* node) const
{
    return node ? node->subtreeSize : 0;
}

double AVLTree3::getSum(Node* node) const
{
    return node ? node->subtreeSum : 0.0;
}

int AVLTree3::getBalanceFactor(Node* node) const
{
    return node ? getHeight(node->left) - getHeight(node->right) : 0;
}

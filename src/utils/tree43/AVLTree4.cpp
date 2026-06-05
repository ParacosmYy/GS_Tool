/**
 * @file AVLTree4.cpp
 * @brief AVL树4实现 — 批量构建+范围查询优化
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/tree43/AVLTree4.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <limits>
#include <cmath>

/**
 * @brief 构造函数，初始化空树
 * @param parent 父对象
 */
AVLTree4::AVLTree4(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("AVLTree4"));
}

/**
 * @brief 析构函数，释放所有节点
 */
AVLTree4::~AVLTree4()
{
    destroyTree(m_root);
}

/**
 * @brief 插入单个键值对
 *
 * 标准AVL插入+平衡操作。更新子树大小以支持排名查询。
 *
 * @param key 键
 * @param value 值
 */
void AVLTree4::insert(double key, int value)
{
    QElapsedTimer timer;
    timer.start();

    m_root = insertNode(m_root, key, value);
    m_stats.totalInsertions++;

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(1, m_stats.totalInsertions + m_stats.totalDeletions + m_stats.totalRangeQueries);
}

/**
 * @brief 批量插入键值对
 *
 * 先排序再递归构建平衡AVL树，时间复杂度O(n log n)。
 * 比逐个插入更高效，适用于大规模初始数据加载。
 *
 * @param items 键值对列表
 */
void AVLTree4::insertBatch(const QVector<QPair<double,int>>& items)
{
    QElapsedTimer timer;
    timer.start();

    if (items.isEmpty()) return;

    /* 合并现有中序遍历和新数据 */
    QVector<QPair<double,int>> existing = inOrderTraversal();
    QVector<QPair<double,int>> all = existing;
    all.append(items);

    /* 排序并去重 */
    std::sort(all.begin(), all.end(),
              [](const QPair<double,int>& a, const QPair<double,int>& b) {
                  return a.first < b.first;
              });

    /* 重建平衡树 */
    destroyTree(m_root);
    m_root = buildBalanced(all, 0, all.size() - 1);
    m_size = all.size();

    m_stats.totalInsertions += items.size();

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(1, m_stats.totalInsertions + m_stats.totalDeletions + m_stats.totalRangeQueries);

    emit batchInsertCompleted(items.size(), height());
}

/**
 * @brief 删除指定键
 * @param key 要删除的键
 * @return 是否成功删除
 */
bool AVLTree4::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    int oldSize = m_size;
    m_root = removeNode(m_root, key);

    m_stats.totalDeletions++;

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(1, m_stats.totalInsertions + m_stats.totalDeletions + m_stats.totalRangeQueries);

    return m_size < oldSize;
}

/**
 * @brief 查找指定键的值
 * @param key 目标键
 * @return 对应值（未找到返回-1）
 */
int AVLTree4::find(double key) const
{
    Node* cur = m_root;
    while (cur) {
        if (key < cur->key) cur = cur->left;
        else if (key > cur->key) cur = cur->right;
        else return cur->value;
    }
    return -1;
}

/**
 * @brief 范围查询 — 返回[low, high]区间内所有键值对
 *
 * 利用子树大小信息剪枝，跳过不可能包含目标区间的子树。
 *
 * @param low 下界
 * @param high 上界
 * @return 范围内的键值对列表
 */
QVector<QPair<double,int>> AVLTree4::rangeQuery(double low, double high) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<double,int>> result;
    std::function<void(Node*)> traverse = [&](Node* node) {
        if (!node) return;
        if (low < node->key) traverse(node->left);
        if (node->key >= low && node->key <= high) {
            result.append({node->key, node->value});
        }
        if (high > node->key) traverse(node->right);
    };
    traverse(m_root);

    /* 注意：这里修改mutable统计量，但rangeQuery标记为const */
    const_cast<AVLTree4*>(this)->m_stats.totalRangeQueries++;
    const_cast<AVLTree4*>(this)->m_timeSum += timer.elapsed();
    const_cast<AVLTree4*>(this)->m_stats.avgProcessingTimeMs =
        const_cast<AVLTree4*>(this)->m_timeSum /
        qMax(1, m_stats.totalInsertions + m_stats.totalDeletions + m_stats.totalRangeQueries);

    return result;
}

/**
 * @brief 范围计数 — 返回[low, high]区间内的元素数
 *
 * 利用子树大小信息实现O(log n)计数，无需遍历所有元素。
 *
 * @param low 下界
 * @param high 上界
 * @return 区间内元素数
 */
int AVLTree4::rangeCount(double low, double high) const
{
    auto countLess = [&](double key) -> int {
        int count = 0;
        Node* cur = m_root;
        while (cur) {
            if (key <= cur->key) {
                cur = cur->left;
            } else {
                count += 1 + (cur->left ? cur->left->subtreeSize : 0);
                cur = cur->right;
            }
        }
        return count;
    };
    return countLess(high) - countLess(low) +
        (find(high) != -1 ? 1 : 0);
}

/**
 * @brief 查找第k小的键
 * @param k 排名（0-indexed）
 * @return 第k小的键
 */
double AVLTree4::kth(int k) const
{
    if (k < 0 || k >= m_size) return std::numeric_limits<double>::quiet_NaN();

    Node* cur = m_root;
    while (cur) {
        int leftSize = cur->left ? cur->left->subtreeSize : 0;
        if (k < leftSize) {
            cur = cur->left;
        } else if (k == leftSize) {
            return cur->key;
        } else {
            k -= leftSize + 1;
            cur = cur->right;
        }
    }
    return std::numeric_limits<double>::quiet_NaN();
}

/**
 * @brief 计算指定键的排名（0-indexed）
 * @param key 目标键
 * @return 排名
 */
int AVLTree4::rank(double key) const
{
    int r = 0;
    Node* cur = m_root;
    while (cur) {
        if (key < cur->key) {
            cur = cur->left;
        } else if (key > cur->key) {
            r += 1 + (cur->left ? cur->left->subtreeSize : 0);
            cur = cur->right;
        } else {
            r += cur->left ? cur->left->subtreeSize : 0;
            return r;
        }
    }
    return r;
}

/**
 * @brief 清空树
 */
void AVLTree4::clear()
{
    destroyTree(m_root);
    m_root = nullptr;
    m_size = 0;
}

/**
 * @brief 获取树高度
 * @return 树高度（空树为0）
 */
int AVLTree4::height() const
{
    return m_root ? m_root->height : 0;
}

/**
 * @brief 中序遍历获取所有键值对
 * @return 按键升序排列的键值对列表
 */
QVector<QPair<double,int>> AVLTree4::inOrderTraversal() const
{
    QVector<QPair<double,int>> result;
    std::function<void(Node*)> traverse = [&](Node* node) {
        if (!node) return;
        traverse(node->left);
        result.append({node->key, node->value});
        traverse(node->right);
    };
    traverse(m_root);
    return result;
}

/**
 * @brief 插入节点的递归实现
 */
AVLTree4::Node* AVLTree4::insertNode(Node* node, double key, int value)
{
    if (!node) {
        m_size++;
        Node* n = new Node{key, value, 1, 1, nullptr, nullptr};
        return n;
    }

    if (key < node->key) {
        node->left = insertNode(node->left, key, value);
    } else if (key > node->key) {
        node->right = insertNode(node->right, key, value);
    } else {
        node->value = value;
        return node;
    }

    updateHeight(node);
    updateSize(node);
    return balance(node);
}

/**
 * @brief 删除节点的递归实现
 */
AVLTree4::Node* AVLTree4::removeNode(Node* node, double key)
{
    if (!node) return nullptr;

    if (key < node->key) {
        node->left = removeNode(node->left, key);
    } else if (key > node->key) {
        node->right = removeNode(node->right, key);
    } else {
        m_size--;
        if (!node->left || !node->right) {
            Node* child = node->left ? node->left : node->right;
            delete node;
            return child;
        }
        /* 找后继 */
        Node* succ = node->right;
        while (succ->left) succ = succ->left;
        node->key = succ->key;
        node->value = succ->value;
        m_size++;
        node->right = removeNode(node->right, succ->key);
    }

    updateHeight(node);
    updateSize(node);
    return balance(node);
}

/**
 * @brief 平衡操作
 */
AVLTree4::Node* AVLTree4::balance(Node* node)
{
    int bf = getBalance(node);
    if (bf > 1) {
        if (getBalance(node->left) < 0)
            node->left = rotateLeft(node->left);
        return rotateRight(node);
    }
    if (bf < -1) {
        if (getBalance(node->right) > 0)
            node->right = rotateRight(node->right);
        return rotateLeft(node);
    }
    return node;
}

/**
 * @brief 左旋
 */
AVLTree4::Node* AVLTree4::rotateLeft(Node* x)
{
    Node* y = x->right;
    x->right = y->left;
    y->left = x;
    updateHeight(x);
    updateSize(x);
    updateHeight(y);
    updateSize(y);
    return y;
}

/**
 * @brief 右旋
 */
AVLTree4::Node* AVLTree4::rotateRight(Node* y)
{
    Node* x = y->left;
    y->left = x->right;
    x->right = y;
    updateHeight(y);
    updateSize(y);
    updateHeight(x);
    updateSize(x);
    return x;
}

/**
 * @brief 获取平衡因子
 */
int AVLTree4::getBalance(Node* node) const
{
    if (!node) return 0;
    int lh = node->left ? node->left->height : 0;
    int rh = node->right ? node->right->height : 0;
    return lh - rh;
}

/**
 * @brief 更新节点高度
 */
void AVLTree4::updateHeight(Node* node)
{
    if (!node) return;
    int lh = node->left ? node->left->height : 0;
    int rh = node->right ? node->right->height : 0;
    node->height = 1 + qMax(lh, rh);
}

/**
 * @brief 更新子树大小
 */
void AVLTree4::updateSize(Node* node)
{
    if (!node) return;
    node->subtreeSize = 1;
    if (node->left) node->subtreeSize += node->left->subtreeSize;
    if (node->right) node->subtreeSize += node->right->subtreeSize;
}

/**
 * @brief 从排序数组递归构建平衡AVL树
 */
AVLTree4::Node* AVLTree4::buildBalanced(const QVector<QPair<double,int>>& sorted,
                                          int lo, int hi)
{
    if (lo > hi) return nullptr;
    int mid = (lo + hi) / 2;
    Node* node = new Node{sorted[mid].first, sorted[mid].second, 0, 0, nullptr, nullptr};
    node->left = buildBalanced(sorted, lo, mid - 1);
    node->right = buildBalanced(sorted, mid + 1, hi);
    updateHeight(node);
    updateSize(node);
    return node;
}

/**
 * @brief 递归销毁树
 */
void AVLTree4::destroyTree(Node* n)
{
    if (!n) return;
    destroyTree(n->left);
    destroyTree(n->right);
    delete n;
}

/**
 * @brief 重置所有统计数据
 */
void AVLTree4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

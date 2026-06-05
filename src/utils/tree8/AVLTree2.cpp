/**
 * @file AVLTree2.cpp
 * @brief 增强AVL树实现 — 带子树规模的顺序统计操作
 *
 * 每个节点额外维护subtreeSize字段，支持O(logN)的按秩查询、排名查询、
 * 范围计数等顺序统计操作。插入和删除后自动平衡。
 */

#include "utils/tree8/AVLTree2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <limits>

// ── 构造 / 析构 ──

/** @brief 构造函数 @param parent 父对象 */
AVLTree2::AVLTree2(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("AVLTree2"));
}

/** @brief 析构函数 — 递归销毁所有节点 */
AVLTree2::~AVLTree2()
{
    destroyTree(m_root);
}

// ── 插入 ──

/**
 * @brief 插入值
 * @param value 待插入值
 */
void AVLTree2::insert(double value)
{
    QElapsedTimer timer;
    timer.start();

    m_root = insertNode(m_root, value);

    ++m_stats.totalInsertions;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalInsertions
            + m_stats.totalDeletions + m_stats.totalQueries);

    emit elementInserted(value, size());
}

// ── 删除 ──

/**
 * @brief 删除值
 * @param value 待删除值
 * @return 是否成功删除
 */
bool AVLTree2::remove(double value)
{
    QElapsedTimer timer;
    timer.start();

    bool removed = false;
    m_root = removeNode(m_root, value, removed);

    ++m_stats.totalDeletions;
    if (!removed) --m_stats.totalDeletions; /* 未实际删除不计入 */
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalInsertions
            + m_stats.totalDeletions + m_stats.totalQueries);

    emit elementRemoved(value, removed);
    return removed;
}

// ── 查询 ──

/**
 * @brief 查询值是否存在
 * @param value 查询值
 * @return 是否存在
 */
bool AVLTree2::contains(double value) const
{
    Node* node = m_root;
    while (node) {
        if (qFuzzyCompare(value, node->value)) return true;
        if (value < node->value) node = node->left;
        else node = node->right;
    }
    return false;
}

/**
 * @brief 查询值的排名(从0开始)
 * @param value 查询值
 * @return 排名(严格小于value的元素个数)
 */
int AVLTree2::rank(double value) const
{
    ++const_cast<AVLTree2*>(this)->m_stats.totalQueries;
    return rankQuery(m_root, value);
}

/**
 * @brief 按秩查询第k小的值
 * @param k 秩(从0开始)
 * @return 第k小的值; 越界返回NaN
 */
double AVLTree2::kthElement(int k) const
{
    ++const_cast<AVLTree2*>(this)->m_stats.totalQueries;
    if (k < 0 || k >= size()) return std::numeric_limits<double>::quiet_NaN();
    return kthQuery(m_root, k);
}

/**
 * @brief 查询中位数
 * @return 中位数值
 */
double AVLTree2::median() const
{
    int n = size();
    if (n == 0) return std::numeric_limits<double>::quiet_NaN();
    if (n % 2 == 1) return kthElement(n / 2);
    return (kthElement(n / 2 - 1) + kthElement(n / 2)) * 0.5;
}

/**
 * @brief 范围计数: 统计[a,b]内的元素个数
 * @param a 区间左端
 * @param b 区间右端
 * @return 元素个数
 *
 * 利用 rank(b+eps) - rank(a) 的差值计算。
 */
int AVLTree2::rangeCount(double a, double b) const
{
    if (a > b) std::swap(a, b);
    ++const_cast<AVLTree2*>(this)->m_stats.totalQueries;
    return rangeCountQuery(m_root, a, b);
}

/**
 * @brief 范围查询: 获取[a,b]内所有值(升序)
 * @param a 区间左端
 * @param b 区间右端
 * @return 值列表
 */
QVector<double> AVLTree2::rangeQuery(double a, double b) const
{
    QVector<double> result;
    if (a > b) std::swap(a, b);
    rangeCollect(m_root, a, b, result);
    return result;
}

/**
 * @brief 前驱: 小于value的最大值
 * @param value 查询值
 * @return 前驱值; 不存在返回NaN
 */
double AVLTree2::predecessor(double value) const
{
    Node* node = m_root;
    Node* pred = nullptr;

    while (node) {
        if (node->value < value && !qFuzzyCompare(node->value, value)) {
            pred = node;
            node = node->right;
        } else {
            node = node->left;
        }
    }

    return pred ? pred->value
        : std::numeric_limits<double>::quiet_NaN();
}

/**
 * @brief 后继: 大于value的最小值
 * @param value 查询值
 * @return 后继值; 不存在返回NaN
 */
double AVLTree2::successor(double value) const
{
    Node* node = m_root;
    Node* succ = nullptr;

    while (node) {
        if (node->value > value && !qFuzzyCompare(node->value, value)) {
            succ = node;
            node = node->left;
        } else {
            node = node->right;
        }
    }

    return succ ? succ->value
        : std::numeric_limits<double>::quiet_NaN();
}

/** @brief 元素总数 */
int AVLTree2::size() const { return nodeSize(m_root); }

/** @brief 树高度 */
int AVLTree2::height() const { return nodeHeight(m_root); }

/** @brief 清空树 */
void AVLTree2::clear()
{
    destroyTree(m_root);
    m_root = nullptr;
}

/** @brief 重置统计 */
void AVLTree2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

// ── AVL平衡操作 ──

/** @brief 获取节点高度 */
int AVLTree2::nodeHeight(const Node* node)
{
    return node ? node->height : 0;
}

/** @brief 获取子树规模 */
int AVLTree2::nodeSize(const Node* node)
{
    return node ? node->subtreeSize : 0;
}

/** @brief 更新节点高度和规模 */
void AVLTree2::updateNode(Node* node)
{
    if (!node) return;
    node->height = 1 + qMax(nodeHeight(node->left),
        nodeHeight(node->right));
    node->subtreeSize = 1 + nodeSize(node->left) + nodeSize(node->right);
}

/** @brief 平衡因子 */
int AVLTree2::balanceFactor(const Node* node)
{
    return node ? nodeHeight(node->left) - nodeHeight(node->right) : 0;
}

/**
 * @brief 右旋(LL型)
 *
 *     y           x
 *    / \         / \
 *   x   C  ->  A   y
 *  / \             / \
 * A   B           B   C
 */
AVLTree2::Node* AVLTree2::rotateRight(Node* y)
{
    Node* x = y->left;
    y->left = x->right;
    x->right = y;
    updateNode(y);
    updateNode(x);
    ++m_stats.totalRotations;
    return x;
}

/**
 * @brief 左旋(RR型)
 *
 *   x             y
 *  / \           / \
 * A   y    ->   x   C
 *    / \       / \
 *   B   C     A   B
 */
AVLTree2::Node* AVLTree2::rotateLeft(Node* x)
{
    Node* y = x->right;
    x->right = y->left;
    y->left = x;
    updateNode(x);
    updateNode(y);
    ++m_stats.totalRotations;
    return y;
}

/** @brief 平衡化 */
AVLTree2::Node* AVLTree2::balance(Node* node)
{
    updateNode(node);
    int bf = balanceFactor(node);

    if (bf > 1) {
        if (balanceFactor(node->left) < 0) {
            node->left = rotateLeft(node->left);
        }
        return rotateRight(node);
    }
    if (bf < -1) {
        if (balanceFactor(node->right) > 0) {
            node->right = rotateRight(node->right);
        }
        return rotateLeft(node);
    }
    return node;
}

// ── 递归操作 ──

/** @brief 递归插入 */
AVLTree2::Node* AVLTree2::insertNode(Node* node, double value)
{
    if (!node) return new Node{value, 1, 1, nullptr, nullptr};

    if (value < node->value) {
        node->left = insertNode(node->left, value);
    } else if (value > node->value) {
        node->right = insertNode(node->right, value);
    } else {
        /* 重复值忽略 */
        return node;
    }
    return balance(node);
}

/** @brief 递归删除 */
AVLTree2::Node* AVLTree2::removeNode(Node* node, double value,
    bool& removed)
{
    if (!node) return nullptr;

    if (value < node->value) {
        node->left = removeNode(node->left, value, removed);
    } else if (value > node->value) {
        node->right = removeNode(node->right, value, removed);
    } else {
        removed = true;
        if (!node->left || !node->right) {
            Node* child = node->left ? node->left : node->right;
            delete node;
            return child;
        }
        /* 两子节点: 用中序后继替换 */
        Node* succ = findMin(node->right);
        node->value = succ->value;
        node->right = removeNode(node->right, succ->value, removed);
    }
    return balance(node);
}

/** @brief 找最小节点 */
AVLTree2::Node* AVLTree2::findMin(Node* node)
{
    while (node && node->left) node = node->left;
    return node;
}

/** @brief 递归排名查询 */
int AVLTree2::rankQuery(const Node* node, double value) const
{
    if (!node) return 0;

    if (value < node->value) {
        return rankQuery(node->left, value);
    } else if (value > node->value) {
        return 1 + nodeSize(node->left) + rankQuery(node->right, value);
    }
    /* 等于: 左子树大小 */
    return nodeSize(node->left);
}

/** @brief 递归按秩查询 */
double AVLTree2::kthQuery(const Node* node, int k) const
{
    if (!node) return std::numeric_limits<double>::quiet_NaN();

    int leftSize = nodeSize(node->left);
    if (k < leftSize) return kthQuery(node->left, k);
    if (k == leftSize) return node->value;
    return kthQuery(node->right, k - leftSize - 1);
}

/** @brief 递归范围计数 */
int AVLTree2::rangeCountQuery(const Node* node, double a,
    double b) const
{
    if (!node) return 0;
    if (node->value < a) return rangeCountQuery(node->right, a, b);
    if (node->value > b) return rangeCountQuery(node->left, a, b);
    return 1 + rangeCountQuery(node->left, a, b)
        + rangeCountQuery(node->right, a, b);
}

/** @brief 递归范围收集 */
void AVLTree2::rangeCollect(const Node* node, double a, double b,
    QVector<double>& result) const
{
    if (!node) return;
    if (node->value > a) rangeCollect(node->left, a, b, result);
    if (node->value >= a && node->value <= b) result.append(node->value);
    if (node->value < b) rangeCollect(node->right, a, b, result);
}

/** @brief 递归销毁 */
void AVLTree2::destroyTree(Node* node)
{
    if (!node) return;
    destroyTree(node->left);
    destroyTree(node->right);
    delete node;
}

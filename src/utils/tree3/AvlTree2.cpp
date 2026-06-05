/**
 * @file AvlTree2.cpp
 * @brief AvlTree2 实现 — 带子树大小增强的AVL平衡二叉搜索树
 *
 * 核心增强: 每个节点维护subtreeSize = 1 + left.subtreeSize + right.subtreeSize
 * - kth(k): 从根出发, 比较k与left.size决定向左/向右
 * - rank(key): 搜索路径上累加左子树的size
 * - rangeCount(lo, hi) = rank(hi+eps) - rank(lo)
 */

#include "utils/tree3/AvlTree2.h"

#include <QtMath>
#include <algorithm>

// ── 构造 / 析构 ──

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
AvlTree2::AvlTree2(QObject* parent)
    : QObject(parent)
    , m_root(nullptr)
{
    setObjectName(QStringLiteral("AvlTree2"));
    m_timer.start();
}

/**
 * @brief 析构函数, 递归销毁整棵树
 */
AvlTree2::~AvlTree2()
{
    destroy(m_root);
    m_root = nullptr;
}

// ── 核心操作 ──

/**
 * @brief 插入键值对
 *
 * 若key已存在则更新value。插入后自底向上维护平衡和subtreeSize。
 * @param key   键
 * @param value 关联值
 */
void AvlTree2::insert(double key, const QVariant& value)
{
    m_timer.restart();
    bool inserted = false;
    m_root = insertNode(m_root, key, value, inserted);

    if (inserted) {
        ++m_stats.totalInserts;
        updateAvgTime(static_cast<double>(m_timer.nsecsElapsed()) / 1e6);
        emit nodeInserted(key);
    } else {
        /* 更新不增加计数, 但仍记录耗时 */
        updateAvgTime(static_cast<double>(m_timer.nsecsElapsed()) / 1e6);
    }
}

/**
 * @brief 删除指定键
 * @param key 待删除的键
 * @return true = 成功删除
 */
bool AvlTree2::remove(double key)
{
    m_timer.restart();
    bool removed = false;
    m_root = removeNode(m_root, key, removed);

    if (removed) {
        ++m_stats.totalRemoves;
        updateAvgTime(static_cast<double>(m_timer.nsecsElapsed()) / 1e6);
        emit nodeRemoved(key);
    }
    return removed;
}

/**
 * @brief 查找键对应的值
 * @param key 查找键
 * @return 关联值或无效QVariant
 */
QVariant AvlTree2::find(double key)
{
    m_timer.restart();
    ++m_stats.totalSearches;

    Node* node = findNode(m_root, key);
    updateAvgTime(static_cast<double>(m_timer.nsecsElapsed()) / 1e6);

    return node ? node->value : QVariant();
}

// ── 增强查询 ──

/**
 * @brief 查询第k小的键(0-indexed)
 *
 * 算法: 从根出发,
 * - leftSize > k → 向左子树继续
 * - leftSize == k → 当前节点即为答案
 * - leftSize < k → 向右子树查询第 (k - leftSize - 1) 个
 * @param k 排名索引(0-based)
 * @return 第k小的键
 */
double AvlTree2::kth(int k)
{
    m_timer.restart();
    ++m_stats.totalSearches;

    if (k < 0 || k >= sizeOf(m_root)) {
        updateAvgTime(static_cast<double>(m_timer.nsecsElapsed()) / 1e6);
        return qQNaN();
    }

    Node* node = m_root;
    while (node) {
        int leftSize = sizeOf(node->left);
        if (k < leftSize) {
            node = node->left;
        } else if (k == leftSize) {
            updateAvgTime(static_cast<double>(m_timer.nsecsElapsed()) / 1e6);
            return node->key;
        } else {
            k -= leftSize + 1;
            node = node->right;
        }
    }

    updateAvgTime(static_cast<double>(m_timer.nsecsElapsed()) / 1e6);
    return qQNaN();
}

/**
 * @brief 查询键的排名(严格小于key的键数量)
 *
 * 算法: 从根出发搜索key, 路径上:
 * - 向右走时累加leftSize + 1(当前节点及其左子树都比key小)
 * - 向左走时不累加
 * - 找到key时不累加(严格小于)
 * @param key 查询键
 * @return 小于key的键数量
 */
int AvlTree2::rank(double key)
{
    m_timer.restart();
    ++m_stats.totalSearches;

    int r = 0;
    Node* node = m_root;
    while (node) {
        if (key < node->key) {
            node = node->left;
        } else if (key > node->key) {
            r += sizeOf(node->left) + 1;
            node = node->right;
        } else {
            /* key相等: 只计左子树 */
            r += sizeOf(node->left);
            break;
        }
    }

    updateAvgTime(static_cast<double>(m_timer.nsecsElapsed()) / 1e6);
    return r;
}

/**
 * @brief 范围计数: 统计[lo, hi]区间内的键数量
 *
 * 利用rank: count = rank(hi+epsilon) - rank(lo)
 * 其中 rank(hi+eps) 是 <= hi 的键数, rank(lo) 是 < lo 的键数。
 * 差值即为 [lo, hi] 内的键数。
 * @param lo 下界(包含)
 * @param hi 上界(包含)
 * @return 区间内键数量
 */
int AvlTree2::rangeCount(double lo, double hi)
{
    m_timer.restart();
    ++m_stats.totalSearches;

    if (lo > hi) {
        updateAvgTime(static_cast<double>(m_timer.nsecsElapsed()) / 1e6);
        return 0;
    }

    /* rank(hi) 返回 <= hi 的键数(因为rank找严格小于hi的)
       但我们需要 <= hi, 所以用 rank(hi + eps) 来包含等于hi的键 */
    int countLEHi = 0;
    Node* node = m_root;
    while (node) {
        if (hi < node->key) {
            node = node->left;
        } else if (hi > node->key) {
            countLEHi += sizeOf(node->left) + 1;
            node = node->right;
        } else {
            countLEHi += sizeOf(node->left) + 1;
            break;
        }
    }

    /* rank(lo) = 严格小于 lo 的键数 */
    int countLTLo = 0;
    node = m_root;
    while (node) {
        if (lo < node->key) {
            node = node->left;
        } else if (lo > node->key) {
            countLTLo += sizeOf(node->left) + 1;
            node = node->right;
        } else {
            countLTLo += sizeOf(node->left);
            break;
        }
    }

    updateAvgTime(static_cast<double>(m_timer.nsecsElapsed()) / 1e6);
    return countLEHi - countLTLo;
}

/**
 * @brief 中序遍历返回所有键(升序)
 * @return 升序排列的键列表
 */
QVector<double> AvlTree2::inOrder()
{
    QVector<double> result;
    result.reserve(sizeOf(m_root));
    inOrderHelper(m_root, result);
    return result;
}

/** @brief 节点总数 */
int AvlTree2::size() const
{
    return sizeOf(m_root);
}

/** @brief 树是否为空 */
bool AvlTree2::isEmpty() const
{
    return m_root == nullptr;
}

// ── 统计 ──

AvlTree2::Stats AvlTree2::stats() const
{
    return m_stats;
}

void AvlTree2::resetStatistics()
{
    m_stats = Stats{};
}

// ── 私有方法 ──

/**
 * @brief 递归销毁子树
 * @param node 当前节点
 */
void AvlTree2::destroy(Node* node)
{
    if (!node) return;
    destroy(node->left);
    destroy(node->right);
    delete node;
}

/**
 * @brief 递归插入节点
 *
 * 标准BST插入 + 自底向上平衡 + subtreeSize更新。
 * @param node     当前子树根
 * @param key      插入键
 * @param value    插入值
 * @param inserted [out] 是否实际新增了节点
 * @return 新的子树根
 */
AvlTree2::Node* AvlTree2::insertNode(Node* node, double key,
                                      const QVariant& value, bool& inserted)
{
    if (!node) {
        inserted = true;
        return new Node(key, value);
    }

    if (key < node->key) {
        node->left = insertNode(node->left, key, value, inserted);
    } else if (key > node->key) {
        node->right = insertNode(node->right, key, value, inserted);
    } else {
        /* 键已存在, 更新值 */
        node->value = value;
        inserted = false;
        return node;
    }

    return balance(node);
}

/**
 * @brief 递归删除节点
 *
 * BST删除: 找到后继节点替换, 然后自底向上平衡。
 * @param node    当前子树根
 * @param key     删除键
 * @param removed [out] 是否成功删除
 * @return 新的子树根
 */
AvlTree2::Node* AvlTree2::removeNode(Node* node, double key, bool& removed)
{
    if (!node) {
        removed = false;
        return nullptr;
    }

    if (key < node->key) {
        node->left = removeNode(node->left, key, removed);
    } else if (key > node->key) {
        node->right = removeNode(node->right, key, removed);
    } else {
        removed = true;
        /* 0或1个子节点 */
        if (!node->left || !node->right) {
            Node* child = node->left ? node->left : node->right;
            delete node;
            return child;
        }
        /* 2个子节点: 找中序后继替换 */
        Node* successor = node->right;
        while (successor->left) {
            successor = successor->left;
        }
        node->key = successor->key;
        node->value = successor->value;
        node->right = removeNode(node->right, successor->key, removed);
    }

    return balance(node);
}

/**
 * @brief 递归查找节点
 * @param node 当前子树根
 * @param key  查找键
 * @return 找到的节点指针(或nullptr)
 */
AvlTree2::Node* AvlTree2::findNode(Node* node, double key) const
{
    while (node) {
        if (key < node->key) {
            node = node->left;
        } else if (key > node->key) {
            node = node->right;
        } else {
            return node;
        }
    }
    return nullptr;
}

/**
 * @brief 右旋(LL型不平衡)
 * @param y 旋转轴心
 * @return 新的子树根
 */
AvlTree2::Node* AvlTree2::rotateRight(Node* y)
{
    Node* x = y->left;
    y->left = x->right;
    x->right = y;
    updateNode(y);
    updateNode(x);
    return x;
}

/**
 * @brief 左旋(RR型不平衡)
 * @param x 旋转轴心
 * @return 新的子树根
 */
AvlTree2::Node* AvlTree2::rotateLeft(Node* x)
{
    Node* y = x->right;
    x->right = y->left;
    y->left = x;
    updateNode(x);
    updateNode(y);
    return y;
}

/**
 * @brief 平衡节点(LL/LR/RR/RL四种情况)
 *
 * |bf| > 1时不平衡:
 * - bf > 1, left.bf >= 0: LL → 右旋
 * - bf > 1, left.bf < 0:  LR → 先左旋left再右旋
 * - bf < -1, right.bf <= 0: RR → 左旋
 * - bf < -1, right.bf > 0:  RL → 先右旋right再左旋
 * @param node 当前节点
 * @return 平衡后的子树根
 */
AvlTree2::Node* AvlTree2::balance(Node* node)
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

/**
 * @brief 更新节点高度和subtreeSize
 * @param node 待更新节点
 */
void AvlTree2::updateNode(Node* node)
{
    if (!node) return;
    node->height = 1 + std::max(heightOf(node->left), heightOf(node->right));
    node->subtreeSize = 1 + sizeOf(node->left) + sizeOf(node->right);
}

/**
 * @brief 获取平衡因子 = leftHeight - rightHeight
 * @param node 节点
 * @return 平衡因子
 */
int AvlTree2::balanceFactor(Node* node) const
{
    return node ? heightOf(node->left) - heightOf(node->right) : 0;
}

/** @brief 安全获取节点高度 */
int AvlTree2::heightOf(Node* node) const
{
    return node ? node->height : 0;
}

/** @brief 安全获取子树大小 */
int AvlTree2::sizeOf(Node* node) const
{
    return node ? node->subtreeSize : 0;
}

/**
 * @brief 中序遍历递归
 * @param node   当前节点
 * @param result 输出列表
 */
void AvlTree2::inOrderHelper(Node* node, QVector<double>& result) const
{
    if (!node) return;
    inOrderHelper(node->left, result);
    result.append(node->key);
    inOrderHelper(node->right, result);
}

/**
 * @brief 更新统计平均耗时
 * @param elapsed 本次操作耗时(ms)
 */
void AvlTree2::updateAvgTime(double elapsed)
{
    quint64 n = m_stats.totalInserts + m_stats.totalRemoves + m_stats.totalSearches;
    if (n > 0) {
        m_stats.avgProcessingTimeMs =
            (m_stats.avgProcessingTimeMs * static_cast<double>(n - 1)
             + elapsed) / static_cast<double>(n);
    }
}

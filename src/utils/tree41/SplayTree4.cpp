/**
 * @file SplayTree4.cpp
 * @brief 伸展树4 — 指针优化+局部性感知实现
 *
 * 实现Splay Tree自调整二叉搜索树，支持：
 * - 伸展操作（Zig/Zig-Zag/Zig-Zig）
 * - 插入/删除/查找
 * - 顺序统计（k-th和rank查询）
 * - 子树大小维护
 * 所有运算带有QElapsedTimer计时和统计信息追踪。
 */

#include "utils/tree41/SplayTree4.h"

#include <QElapsedTimer>

/**
 * @brief 构造函数
 * @param parent 父对象
 */
SplayTree4::SplayTree4(QObject* parent)
    : QObject(parent)
    , m_root(nullptr)
{
}

/**
 * @brief 析构函数，释放所有节点
 */
SplayTree4::~SplayTree4()
{
    destroyTree(m_root);
}

/**
 * @brief 递归销毁子树
 * @param n 子树根节点
 */
void SplayTree4::destroyTree(Node* n)
{
    if (!n) return;
    destroyTree(n->left);
    destroyTree(n->right);
    delete n;
}

/**
 * @brief 更新节点子树大小
 * @param x 目标节点
 */
void SplayTree4::updateSize(Node* x)
{
    if (!x) return;
    x->subtreeSize = 1;
    if (x->left) x->subtreeSize += x->left->subtreeSize;
    if (x->right) x->subtreeSize += x->right->subtreeSize;
}

/**
 * @brief 左旋
 * @param x 旋转节点
 */
void SplayTree4::leftRotate(Node* x)
{
    Node* y = x->right;
    if (!y) return;
    x->right = y->left;
    if (y->left) y->left->parent = x;
    y->parent = x->parent;
    if (!x->parent) m_root = y;
    else if (x == x->parent->left) x->parent->left = y;
    else x->parent->right = y;
    y->left = x;
    x->parent = y;
    updateSize(x);
    updateSize(y);
}

/**
 * @brief 右旋
 * @param x 旋转节点
 */
void SplayTree4::rightRotate(Node* x)
{
    Node* y = x->left;
    if (!y) return;
    x->left = y->right;
    if (y->right) y->right->parent = x;
    y->parent = x->parent;
    if (!x->parent) m_root = y;
    else if (x == x->parent->right) x->parent->right = y;
    else x->parent->left = y;
    y->right = x;
    x->parent = y;
    updateSize(x);
    updateSize(y);
}

/**
 * @brief 伸展操作：将节点x旋转到根
 * @param x 目标节点
 *
 * 执行Zig/Zig-Zag/Zig-Zig三种情况直到x成为根。
 */
void SplayTree4::splay(Node* x)
{
    while (x->parent) {
        Node* p = x->parent;
        Node* g = p->parent;
        if (!g) {
            /* Zig情况 */
            if (x == p->left) rightRotate(p);
            else leftRotate(p);
        } else if (x == p->left && p == g->left) {
            /* Zig-Zig（左-左） */
            rightRotate(g);
            rightRotate(p);
        } else if (x == p->right && p == g->right) {
            /* Zig-Zig（右-右） */
            leftRotate(g);
            leftRotate(p);
        } else if (x == p->right && p == g->left) {
            /* Zig-Zag（右-左） */
            leftRotate(p);
            rightRotate(g);
        } else {
            /* Zig-Zag（左-右） */
            rightRotate(p);
            leftRotate(g);
        }
    }
    m_stats.totalSplays++;
}

/**
 * @brief 查找节点（不伸展）
 * @param key 查找键
 * @return 节点指针，不存在返回nullptr
 */
SplayTree4::Node* SplayTree4::findNode(double key) const
{
    Node* cur = m_root;
    while (cur) {
        if (key < cur->key) cur = cur->left;
        else if (key > cur->key) cur = cur->right;
        else return cur;
    }
    return nullptr;
}

/**
 * @brief 插入键值对
 * @param key 键
 * @param value 值
 *
 * 标准BST插入后执行伸展操作将新节点移至根。
 */
void SplayTree4::insert(double key, int value)
{
    QElapsedTimer timer;
    timer.start();

    Node* newNode = new Node{key, value, 1, nullptr, nullptr, nullptr};

    if (!m_root) {
        m_root = newNode;
    } else {
        Node* cur = m_root;
        Node* parent = nullptr;
        while (cur) {
            parent = cur;
            if (key < cur->key) cur = cur->left;
            else if (key > cur->key) cur = cur->right;
            else {
                /* 键已存在，更新值 */
                cur->value = value;
                delete newNode;
                splay(cur);
                m_timeSum += timer.elapsed();
                m_stats.totalInsertions++;
                m_stats.avgProcessingTimeMs = m_timeSum /
                    (m_stats.totalInsertions + m_stats.totalDeletions);
                emit treeModified("update", m_size);
                return;
            }
        }
        newNode->parent = parent;
        if (key < parent->key) parent->left = newNode;
        else parent->right = newNode;

        /* 更新路径上的子树大小 */
        Node* p = parent;
        while (p) { updateSize(p); p = p->parent; }

        splay(newNode);
    }
    m_size++;

    m_timeSum += timer.elapsed();
    m_stats.totalInsertions++;
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalInsertions + m_stats.totalDeletions);

    emit treeModified("insert", m_size);
}

/**
 * @brief 删除指定键
 * @param key 要删除的键
 * @return 是否成功删除
 */
bool SplayTree4::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    Node* node = findNode(key);
    if (!node) return false;

    splay(node);

    Node* leftTree = node->left;
    Node* rightTree = node->right;

    if (!leftTree) {
        m_root = rightTree;
        if (m_root) m_root->parent = nullptr;
    } else if (!rightTree) {
        m_root = leftTree;
        m_root->parent = nullptr;
    } else {
        /* 找左子树最大节点作为新根 */
        leftTree->parent = nullptr;
        Node* maxLeft = leftTree;
        while (maxLeft->right) maxLeft = maxLeft->right;
        splay(maxLeft);
        m_root->right = rightTree;
        rightTree->parent = m_root;
    }
    updateSize(m_root);
    delete node;
    m_size--;

    m_timeSum += timer.elapsed();
    m_stats.totalDeletions++;
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalInsertions + m_stats.totalDeletions);

    emit treeModified("remove", m_size);
    return true;
}

/**
 * @brief 查找键对应的值
 * @param key 查找键
 * @return 对应值，不存在返回0
 */
int SplayTree4::find(double key)
{
    QElapsedTimer timer;
    timer.start();

    Node* node = findNode(key);
    if (node) {
        splay(node);
        m_timeSum += timer.elapsed();
        m_stats.totalSplays++;
        m_stats.avgProcessingTimeMs = m_timeSum /
            (m_stats.totalInsertions + m_stats.totalDeletions + m_stats.totalSplays);
        return node->value;
    }
    return 0;
}

/**
 * @brief 查找第k小的键
 * @param k 排名（0-indexed）
 * @return 第k小的键
 */
double SplayTree4::kth(int k) const
{
    if (k < 0 || k >= m_size) return 0.0;
    Node* cur = m_root;
    while (cur) {
        int leftSize = cur->left ? cur->left->subtreeSize : 0;
        if (k < leftSize) cur = cur->left;
        else if (k == leftSize) return cur->key;
        else { k -= leftSize + 1; cur = cur->right; }
    }
    return 0.0;
}

/**
 * @brief 计算键的排名
 * @param key 目标键
 * @return 小于key的元素数量
 */
int SplayTree4::rank(double key) const
{
    int r = 0;
    Node* cur = m_root;
    while (cur) {
        if (key < cur->key) {
            cur = cur->left;
        } else if (key > cur->key) {
            r += (cur->left ? cur->left->subtreeSize : 0) + 1;
            cur = cur->right;
        } else {
            r += cur->left ? cur->left->subtreeSize : 0;
            break;
        }
    }
    return r;
}

/**
 * @brief 清空树
 */
void SplayTree4::clear()
{
    destroyTree(m_root);
    m_root = nullptr;
    m_size = 0;
}

/**
 * @brief 中序遍历
 * @return 有序键值对列表
 */
QVector<QPair<double,int>> SplayTree4::inOrderTraversal() const
{
    QVector<QPair<double,int>> result;
    result.reserve(m_size);

    /* 迭代式中序遍历 */
    QVector<Node*> stack;
    Node* cur = m_root;
    while (cur || !stack.isEmpty()) {
        while (cur) {
            stack.append(cur);
            cur = cur->left;
        }
        cur = stack.takeLast();
        result.append({cur->key, cur->value});
        cur = cur->right;
    }
    return result;
}

/**
 * @brief 重置所有统计计数器
 */
void SplayTree4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @file SplayTree.cpp
 * @brief Splay树实现
 */

#include "utils/splay/SplayTree.h"

#include <QElapsedTimer>

/** @brief 构造函数 @param parent 父对象 */
SplayTree::SplayTree(QObject* parent)
    : QObject(parent)
    , m_root(nullptr)
    , m_size(0)
    , m_timeSum(0.0)
{
}

/** @brief 析构函数 */
SplayTree::~SplayTree()
{
    destroyTree(m_root);
}

/** @brief 插入键值对 */
void SplayTree::insert(double key, double value)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_root) {
        m_root = new Node{key, value, nullptr, nullptr, nullptr};
        m_size = 1;
    } else {
        Node* cur = m_root;
        Node* parent = nullptr;

        while (cur) {
            parent = cur;
            if (key < cur->key) {
                cur = cur->left;
            } else if (key > cur->key) {
                cur = cur->right;
            } else {
                cur->value = value;
                splay(cur);
                double elapsed = static_cast<double>(timer.elapsed());
                m_timeSum += elapsed;
                ++m_stats.totalInserts;
                double total = static_cast<double>(m_stats.totalInserts + m_stats.totalDeletes + m_stats.totalSearches);
                m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;
                emit operationCompleted("insert", key);
                return;
            }
        }

        Node* node = new Node{key, value, nullptr, nullptr, parent};
        if (key < parent->key)
            parent->left = node;
        else
            parent->right = node;
        ++m_size;
        splay(node);
    }

    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    ++m_stats.totalInserts;
    double total = static_cast<double>(m_stats.totalInserts + m_stats.totalDeletes + m_stats.totalSearches);
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit operationCompleted("insert", key);
}

/** @brief 删除键 */
bool SplayTree::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_root) return false;

    /* 先splay到根 */
    const double* val = find(key);
    if (!val) {
        double elapsed = static_cast<double>(timer.elapsed());
        m_timeSum += elapsed;
        ++m_stats.totalDeletes;
        double total = static_cast<double>(m_stats.totalInserts + m_stats.totalDeletes + m_stats.totalSearches);
        m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;
        return false;
    }

    /* 此时key在根节点 */
    Node* leftTree = m_root->left;
    Node* rightTree = m_root->right;

    if (leftTree) leftTree->parent = nullptr;
    if (rightTree) rightTree->parent = nullptr;

    delete m_root;
    --m_size;

    if (!leftTree) {
        m_root = rightTree;
    } else if (!rightTree) {
        m_root = leftTree;
    } else {
        /* 找左子树最大节点 */
        Node* maxNode = leftTree;
        while (maxNode->right) maxNode = maxNode->right;
        m_root = leftTree;
        splay(maxNode);
        m_root->right = rightTree;
        rightTree->parent = m_root;
    }

    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    ++m_stats.totalDeletes;
    double total = static_cast<double>(m_stats.totalInserts + m_stats.totalDeletes + m_stats.totalSearches);
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit operationCompleted("delete", key);
    return true;
}

/** @brief 查找键 */
const double* SplayTree::find(double key)
{
    Node* cur = m_root;
    Node* last = nullptr;

    while (cur) {
        last = cur;
        if (key < cur->key) {
            cur = cur->left;
        } else if (key > cur->key) {
            cur = cur->right;
        } else {
            splay(cur);
            ++m_stats.totalSearches;
            return &cur->value;
        }
    }

    if (last) splay(last);
    ++m_stats.totalSearches;
    return nullptr;
}

/** @brief 中序遍历 */
QVector<QPair<double, double>> SplayTree::inorder() const
{
    QVector<QPair<double, double>> result;
    result.reserve(m_size);
    inorderNode(m_root, result);
    return result;
}

/** @brief 重置统计 */
void SplayTree::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief Splay操作 */
void SplayTree::splay(Node* x)
{
    while (x->parent) {
        Node* p = x->parent;
        Node* g = p->parent;

        if (!g) {
            /* Zig */
            if (x == p->left) rotateRight(p);
            else rotateLeft(p);
        } else if (x == p->left && p == g->left) {
            /* Zig-Zig */
            rotateRight(g);
            rotateRight(p);
        } else if (x == p->right && p == g->right) {
            /* Zag-Zag */
            rotateLeft(g);
            rotateLeft(p);
        } else if (x == p->right && p == g->left) {
            /* Zag-Zig */
            rotateLeft(p);
            rotateRight(g);
        } else {
            /* Zig-Zag */
            rotateRight(p);
            rotateLeft(g);
        }
    }
    m_root = x;
}

/** @brief 左旋 */
void SplayTree::rotateLeft(Node* x)
{
    Node* y = x->right;
    if (!y) return;
    x->right = y->left;
    if (y->left) y->left->parent = x;
    y->parent = x->parent;
    if (!x->parent) {
        m_root = y;
    } else if (x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }
    y->left = x;
    x->parent = y;
}

/** @brief 右旋 */
void SplayTree::rotateRight(Node* x)
{
    Node* y = x->left;
    if (!y) return;
    x->left = y->right;
    if (y->right) y->right->parent = x;
    y->parent = x->parent;
    if (!x->parent) {
        m_root = y;
    } else if (x == x->parent->right) {
        x->parent->right = y;
    } else {
        x->parent->left = y;
    }
    y->right = x;
    x->parent = y;
}

/** @brief 递归中序遍历 */
void SplayTree::inorderNode(Node* root,
                             QVector<QPair<double, double>>& result) const
{
    if (!root) return;
    inorderNode(root->left, result);
    result.append({root->key, root->value});
    inorderNode(root->right, result);
}

/** @brief 递归销毁 */
void SplayTree::destroyTree(Node* root)
{
    if (!root) return;
    destroyTree(root->left);
    destroyTree(root->right);
    delete root;
}

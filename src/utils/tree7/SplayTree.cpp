/**
 * @file SplayTree.cpp
 * @brief Splay伸展树实现 — Zig/Zig-Zig/Zig-Zag旋转
 */

#include "SplayTree.h"

#include <QElapsedTimer>

/* ---------- 构造/析构 ---------- */

SplayTree::SplayTree(QObject* parent)
    : QObject(parent)
{
}

SplayTree::~SplayTree()
{
    destroyTree(m_root);
}

/* ---------- 插入 ---------- */

void SplayTree::insert(int key, const QString& value)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_root) {
        m_root = new Node{key, value, nullptr, nullptr, nullptr};
        m_size++;
        m_stats.totalInserts++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalInserts;
        emit inserted(key);
        return;
    }

    /* BST查找插入位置 */
    Node* current = m_root;
    Node* parent = nullptr;

    while (current) {
        parent = current;
        if (key < current->key) {
            current = current->left;
        } else if (key > current->key) {
            current = current->right;
        } else {
            /* 键已存在，更新值并splay */
            current->value = value;
            splay(current);
            m_stats.totalInserts++;
            m_timeSum += timer.elapsed();
            m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalInserts;
            emit inserted(key);
            return;
        }
    }

    /* 创建新节点 */
    Node* newNode = new Node{key, value, nullptr, nullptr, parent};
    if (key < parent->key) {
        parent->left = newNode;
    } else {
        parent->right = newNode;
    }

    m_size++;

    /* Splay新节点到根 */
    splay(newNode);

    m_stats.totalInserts++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalInserts;

    emit inserted(key);
}

/* ---------- 删除 ---------- */

bool SplayTree::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_root) {
        m_stats.totalDeletes++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum /
            (m_stats.totalInserts + m_stats.totalDeletes + m_stats.totalSearches);
        emit removed(key, false);
        return false;
    }

    /* 先splay目标键到根 */
    Node* current = m_root;
    while (current) {
        if (key < current->key) {
            current = current->left;
        } else if (key > current->key) {
            current = current->right;
        } else {
            break;
        }
    }

    if (!current) {
        m_stats.totalDeletes++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum /
            (m_stats.totalInserts + m_stats.totalDeletes + m_stats.totalSearches);
        emit removed(key, false);
        return false;
    }

    splay(current);

    /* 现在current是根节点 */
    Node* leftTree = m_root->left;
    Node* rightTree = m_root->right;

    /* 断开左右子树 */
    if (leftTree) leftTree->parent = nullptr;
    if (rightTree) rightTree->parent = nullptr;

    delete m_root;
    m_size--;

    if (!leftTree) {
        /* 无左子树，右子树直接成为根 */
        m_root = rightTree;
    } else if (!rightTree) {
        /* 无右子树，左子树直接成为根 */
        m_root = leftTree;
    } else {
        /* 合并: 左子树最大节点splay到根，接上右子树 */
        m_root = leftTree;
        Node* maxNode = findMax(leftTree);
        splay(maxNode);
        m_root->right = rightTree;
        rightTree->parent = m_root;
    }

    m_stats.totalDeletes++;
    m_timeSum += timer.elapsed();
    int totalOps = m_stats.totalInserts + m_stats.totalDeletes +
                   m_stats.totalSearches;
    m_stats.avgProcessingTimeMs = (totalOps > 0)
        ? m_timeSum / totalOps : 0.0;

    emit removed(key, true);
    return true;
}

/* ---------- 查找 ---------- */

QString SplayTree::search(int key)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_root) {
        m_stats.totalSearches++;
        m_timeSum += timer.elapsed();
        int totalOps = m_stats.totalInserts + m_stats.totalDeletes +
                       m_stats.totalSearches;
        m_stats.avgProcessingTimeMs = (totalOps > 0)
            ? m_timeSum / totalOps : 0.0;
        emit searched(key, false);
        return QString();
    }

    Node* current = m_root;
    while (current) {
        if (key < current->key) {
            current = current->left;
        } else if (key > current->key) {
            current = current->right;
        } else {
            splay(current);
            m_stats.totalSearches++;
            m_timeSum += timer.elapsed();
            int totalOps = m_stats.totalInserts + m_stats.totalDeletes +
                           m_stats.totalSearches;
            m_stats.avgProcessingTimeMs = (totalOps > 0)
                ? m_timeSum / totalOps : 0.0;
            emit searched(key, true);
            return current->value;
        }
    }

    /* 未找到: splay最后访问的节点 */
    m_stats.totalSearches++;
    m_timeSum += timer.elapsed();
    int totalOps = m_stats.totalInserts + m_stats.totalDeletes +
                   m_stats.totalSearches;
    m_stats.avgProcessingTimeMs = (totalOps > 0)
        ? m_timeSum / totalOps : 0.0;

    emit searched(key, false);
    return QString();
}

/* ---------- Contains ---------- */

bool SplayTree::contains(int key)
{
    return !search(key).isNull() || (m_root && m_root->key == key);
}

/* ---------- Splay旋转操作 ---------- */

void SplayTree::splay(Node* x)
{
    if (!x) return;

    while (x->parent) {
        Node* p = x->parent;
        Node* g = p->parent;

        if (!g) {
            /* Zig: x是根的直接子节点 */
            if (x == p->left) {
                rotateRight(p);
            } else {
                rotateLeft(p);
            }
            m_stats.totalRotations++;
        } else if ((x == p->left) && (p == g->left)) {
            /* Zig-Zig: 同侧左 */
            rotateRight(g);
            rotateRight(p);
            m_stats.totalRotations += 2;
        } else if ((x == p->right) && (p == g->right)) {
            /* Zig-Zig: 同侧右 */
            rotateLeft(g);
            rotateLeft(p);
            m_stats.totalRotations += 2;
        } else if ((x == p->right) && (p == g->left)) {
            /* Zig-Zag: 异侧 */
            rotateLeft(p);
            rotateRight(g);
            m_stats.totalRotations += 2;
        } else {
            /* Zig-Zag: 异侧(镜像) */
            rotateRight(p);
            rotateLeft(g);
            m_stats.totalRotations += 2;
        }
    }

    m_root = x;
}

/* ---------- 左旋 ---------- */

void SplayTree::rotateLeft(Node* x)
{
    if (!x || !x->right) return;

    Node* y = x->right;
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

/* ---------- 右旋 ---------- */

void SplayTree::rotateRight(Node* x)
{
    if (!x || !x->left) return;

    Node* y = x->left;
    x->left = y->right;

    if (y->right) y->right->parent = x;

    y->parent = x->parent;
    if (!x->parent) {
        m_root = y;
    } else if (x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }

    y->right = x;
    x->parent = y;
}

/* ---------- 遍历 ---------- */

QVector<QPair<int, QString>> SplayTree::inOrderTraversal() const
{
    QVector<QPair<int, QString>> result;
    inOrderHelper(m_root, result);
    return result;
}

void SplayTree::inOrderHelper(Node* node,
                               QVector<QPair<int, QString>>& result) const
{
    if (!node) return;
    inOrderHelper(node->left, result);
    result.append({node->key, node->value});
    inOrderHelper(node->right, result);
}

/* ---------- 区间查询 ---------- */

QVector<QPair<int, QString>> SplayTree::rangeQuery(int lo, int hi) const
{
    QVector<QPair<int, QString>> result;
    rangeHelper(m_root, lo, hi, result);
    return result;
}

void SplayTree::rangeHelper(Node* node, int lo, int hi,
                              QVector<QPair<int, QString>>& result) const
{
    if (!node) return;
    if (node->key > lo) rangeHelper(node->left, lo, hi, result);
    if (node->key >= lo && node->key <= hi) {
        result.append({node->key, node->value});
    }
    if (node->key < hi) rangeHelper(node->right, lo, hi, result);
}

/* ---------- 辅助方法 ---------- */

int SplayTree::size() const { return m_size; }
bool SplayTree::isEmpty() const { return m_root == nullptr; }

int SplayTree::height() const { return heightHelper(m_root); }

int SplayTree::heightHelper(Node* node) const
{
    if (!node) return 0;
    return 1 + qMax(heightHelper(node->left), heightHelper(node->right));
}

void SplayTree::destroyTree(Node* node)
{
    if (!node) return;
    destroyTree(node->left);
    destroyTree(node->right);
    delete node;
}

SplayTree::Node* SplayTree::findMax(Node* node) const
{
    while (node && node->right) node = node->right;
    return node;
}

/* ---------- 统计 ---------- */

SplayTree::Stats SplayTree::stats() const { return m_stats; }

void SplayTree::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

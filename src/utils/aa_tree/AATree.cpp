/**
 * @file AATree.cpp
 * @brief AA树实现
 */

#include "AATree.h"
#include <QElapsedTimer>
#include <algorithm>

AATree::AATree(QObject* parent)
    : QObject(parent)
    , m_root(nullptr)
    , m_size(0)
    , m_timeSum(0.0)
    , m_totalOps(0)
{
}

AATree::~AATree()
{
    destroyTree(m_root);
}

void AATree::insert(double key)
{
    QElapsedTimer timer;
    timer.start();

    m_root = insertNode(m_root, key);
    m_stats.totalInserts++;

    m_timeSum += timer.elapsed();
    m_totalOps++;
    m_stats.avgProcessingTimeMs =
        (m_totalOps > 0) ? m_timeSum / m_totalOps : 0.0;
}

void AATree::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    m_root = removeNode(m_root, key);
    m_stats.totalRemoves++;

    m_timeSum += timer.elapsed();
    m_totalOps++;
    m_stats.avgProcessingTimeMs =
        (m_totalOps > 0) ? m_timeSum / m_totalOps : 0.0;
}

bool AATree::contains(double key) const
{
    QElapsedTimer timer;
    timer.start();

    Node* current = m_root;
    while (current != nullptr) {
        if (key < current->key) {
            current = current->left;
        } else if (key > current->key) {
            current = current->right;
        } else {
            m_stats.totalSearches++;
            m_timeSum += timer.elapsed();
            m_totalOps++;
            m_stats.avgProcessingTimeMs =
                (m_totalOps > 0) ? m_timeSum / m_totalOps : 0.0;
            return true;
        }
    }

    m_stats.totalSearches++;
    m_timeSum += timer.elapsed();
    m_totalOps++;
    m_stats.avgProcessingTimeMs =
        (m_totalOps > 0) ? m_timeSum / m_totalOps : 0.0;
    return false;
}

QVector<double> AATree::inOrder() const
{
    QVector<double> result;
    result.reserve(m_size);
    inOrderHelper(m_root, result);
    return result;
}

void AATree::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_totalOps = 0;
}

/* ===== 私有方法实现 ===== */

AATree::Node* AATree::skew(Node* node)
{
    if (node == nullptr || node->left == nullptr)
        return node;

    /* 如果左子节点level与当前节点相同(左向水平链接)，右旋 */
    if (node->left->level == node->level) {
        Node* left = node->left;
        node->left = left->right;
        left->right = node;
        emit rebalanced(left->level);
        return left;
    }
    return node;
}

AATree::Node* AATree::split(Node* node)
{
    if (node == nullptr || node->right == nullptr || node->right->right == nullptr)
        return node;

    /* 如果右子节点的右子节点level与当前节点相同(连续右向水平链接)，左旋 */
    if (node->right->right->level == node->level) {
        Node* right = node->right;
        node->right = right->left;
        right->left = node;
        right->level++;
        emit rebalanced(right->level);
        return right;
    }
    return node;
}

AATree::Node* AATree::insertNode(Node* node, double key)
{
    if (node == nullptr) {
        m_size++;
        return new Node(key);
    }

    if (key < node->key) {
        node->left = insertNode(node->left, key);
    } else if (key > node->key) {
        node->right = insertNode(node->right, key);
    } else {
        /* 重复键值不插入 */
        return node;
    }

    /* 再平衡: 先skew再split */
    node = skew(node);
    node = split(node);
    return node;
}

AATree::Node* AATree::removeNode(Node* node, double key)
{
    if (node == nullptr)
        return nullptr;

    if (key < node->key) {
        node->left = removeNode(node->left, key);
    } else if (key > node->key) {
        node->right = removeNode(node->right, key);
    } else {
        /* 找到要删除的节点 */
        if (node->isLeaf()) {
            delete node;
            m_size--;
            return nullptr;
        } else if (node->left == nullptr) {
            /* 只有右子树: 用后继替换 */
            Node* successor = findMin(node->right);
            node->key = successor->key;
            node->right = removeNode(node->right, successor->key);
        } else {
            /* 有左子树: 用前驱替换 */
            Node* pred = findMin(node->right);
            node->key = pred->key;
            node->right = removeNode(node->right, pred->key);
        }
    }

    /* 降低level并再平衡 */
    node = decreaseLevel(node);
    node = skew(node);
    if (node->right != nullptr) {
        node->right = skew(node->right);
        if (node->right->right != nullptr) {
            node->right->right = skew(node->right->right);
        }
    }
    node = split(node);
    if (node->right != nullptr) {
        node->right = split(node->right);
    }
    return node;
}

AATree::Node* AATree::findMin(Node* node) const
{
    if (node == nullptr) return nullptr;
    while (node->left != nullptr)
        node = node->left;
    return node;
}

AATree::Node* AATree::decreaseLevel(Node* node)
{
    if (node == nullptr) return nullptr;

    int shouldBe = 1;
    if (node->left != nullptr)
        shouldBe = std::min(shouldBe, node->left->level + 1);
    if (node->right != nullptr)
        shouldBe = std::min(shouldBe, node->right->level + 1);

    if (node->level > shouldBe) {
        node->level = shouldBe;
        if (node->right != nullptr && node->right->level > node->level) {
            node->right->level = node->level;
        }
    }
    return node;
}

void AATree::inOrderHelper(Node* node, QVector<double>& result) const
{
    if (node == nullptr) return;
    inOrderHelper(node->left, result);
    result.append(node->key);
    inOrderHelper(node->right, result);
}

void AATree::destroyTree(Node* node)
{
    if (node == nullptr) return;
    destroyTree(node->left);
    destroyTree(node->right);
    delete node;
}

/**
 * @file ScapegoatTree3.cpp
 * @brief 替罪羊树(增强版)实现
 */

#include "utils/tree29/ScapegoatTree3.h"

#include <QElapsedTimer>
#include <algorithm>

ScapegoatTree3::ScapegoatTree3(QObject* parent)
    : QObject(parent)
    , m_root(nullptr)
    , m_size(0)
    , m_alpha(0.7)
    , m_timeSum(0.0)
{
}

ScapegoatTree3::~ScapegoatTree3()
{
    destroyTree(m_root);
}

void ScapegoatTree3::setAlpha(double alpha)
{
    m_alpha = qBound(0.5, alpha, 0.99);
}

void ScapegoatTree3::insert(double key)
{
    QElapsedTimer timer;
    timer.start();

    bool rebuilt = false;
    m_root = insertInternal(m_root, key, rebuilt, 0);
    ++m_size;
    ++m_stats.totalInserts;

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / (m_stats.totalInserts + m_stats.totalDeletes + m_stats.totalQueries);
}

void ScapegoatTree3::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    int oldSize = m_size;
    m_root = removeInternal(m_root, key);
    if (m_size < oldSize) {
        --m_size;
        /* 全局检查: 若删除后总节点过少则重建 */
        if (m_root && m_root->subSize > 0) {
            double ratio = static_cast<double>(m_size) / m_root->subSize;
            if (ratio < m_alpha * m_alpha) {
                QVector<Node*> sorted;
                flatten(m_root, sorted);
                m_root = buildBalanced(sorted, 0, sorted.size() - 1);
                ++m_stats.totalRebuilds;
                emit rebuilt(sorted.size(), 0);
            }
        }
    }

    ++m_stats.totalDeletes;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / (m_stats.totalInserts + m_stats.totalDeletes + m_stats.totalQueries);
}

bool ScapegoatTree3::contains(double key) const
{
    QElapsedTimer timer;
    timer.start();

    Node* cur = m_root;
    while (cur) {
        if (key < cur->key) {
            cur = cur->left;
        } else if (key > cur->key) {
            cur = cur->right;
        } else {
            /* 查询统计在非const方法中更新，这里用const_cast */
            const_cast<ScapegoatTree3*>(this)->m_stats.totalQueries++;
            return true;
        }
    }
    const_cast<ScapegoatTree3*>(this)->m_stats.totalQueries++;
    return false;
}

QVector<double> ScapegoatTree3::rangeQuery(double lo, double hi) const
{
    QVector<double> result;
    rangeCollect(m_root, lo, hi, result);
    return result;
}

double ScapegoatTree3::selectKth(int k) const
{
    if (k < 1 || k > m_size) return 0.0;
    Node* cur = m_root;
    while (cur) {
        int leftSize = (cur->left) ? cur->left->subSize : 0;
        if (k <= leftSize) {
            cur = cur->left;
        } else if (k == leftSize + 1) {
            return cur->key;
        } else {
            k -= leftSize + 1;
            cur = cur->right;
        }
    }
    return 0.0;
}

int ScapegoatTree3::rank(double key) const
{
    int r = 0;
    Node* cur = m_root;
    while (cur) {
        int leftSize = (cur->left) ? cur->left->subSize : 0;
        if (key < cur->key) {
            cur = cur->left;
        } else if (key > cur->key) {
            r += leftSize + 1;
            cur = cur->right;
        } else {
            return r + leftSize + 1;
        }
    }
    return 0;
}

QVector<double> ScapegoatTree3::inOrder() const
{
    QVector<double> result;
    result.reserve(m_size);
    rangeCollect(m_root, std::numeric_limits<double>::lowest(),
                 std::numeric_limits<double>::max(), result);
    return result;
}

bool ScapegoatTree3::validateBalance() const
{
    return validateNode(m_root);
}

ScapegoatTree3::Node* ScapegoatTree3::insertInternal(Node* node, double key,
    bool& rebuilt, int depth)
{
    if (node == nullptr) return new Node(key);

    if (key < node->key) {
        node->left = insertInternal(node->left, key, rebuilt, depth + 1);
    } else if (key > node->key) {
        node->right = insertInternal(node->right, key, rebuilt, depth + 1);
    } else {
        return node;
    }

    updateSubSize(node);

    /* 检查α不平衡并重建 */
    if (!rebuilt && isUnbalanced(node)) {
        double leftRatio = (node->left) ?
            static_cast<double>(node->left->subSize) / node->subSize : 0.0;
        double rightRatio = (node->right) ?
            static_cast<double>(node->right->subSize) / node->subSize : 0.0;
        emit imbalanceDetected(leftRatio, rightRatio);
        node = rebuildSubtree(node, depth);
        ++m_stats.totalRebuilds;
        rebuilt = true;
        emit rebuilt(node->subSize, depth);
    }
    return node;
}

ScapegoatTree3::Node* ScapegoatTree3::removeInternal(Node* node, double key)
{
    if (node == nullptr) return nullptr;

    if (key < node->key) {
        node->left = removeInternal(node->left, key);
    } else if (key > node->key) {
        node->right = removeInternal(node->right, key);
    } else {
        if (node->left == nullptr) {
            Node* right = node->right;
            delete node;
            return right;
        }
        if (node->right == nullptr) {
            Node* left = node->left;
            delete node;
            return left;
        }
        Node* successor = node->right;
        while (successor->left) successor = successor->left;
        node->key = successor->key;
        node->right = removeInternal(node->right, successor->key);
    }

    if (node) updateSubSize(node);
    return node;
}

bool ScapegoatTree3::isUnbalanced(Node* node) const
{
    if (node == nullptr) return false;
    int leftSize = (node->left) ? node->left->subSize : 0;
    int rightSize = (node->right) ? node->right->subSize : 0;
    int total = node->subSize;
    if (total < 2) return false;
    double threshold = m_alpha * static_cast<double>(total);
    return static_cast<double>(leftSize) > threshold ||
           static_cast<double>(rightSize) > threshold;
}

ScapegoatTree3::Node* ScapegoatTree3::rebuildSubtree(Node* node, int depth)
{
    Q_UNUSED(depth)
    QVector<Node*> sorted;
    flatten(node, sorted);
    return buildBalanced(sorted, 0, sorted.size() - 1);
}

void ScapegoatTree3::flatten(Node* node, QVector<Node*>& sorted)
{
    if (node == nullptr) return;
    flatten(node->left, sorted);
    sorted.append(node);
    flatten(node->right, sorted);
    node->left = nullptr;
    node->right = nullptr;
}

ScapegoatTree3::Node* ScapegoatTree3::buildBalanced(const QVector<Node*>& sorted,
    int lo, int hi)
{
    if (lo > hi) return nullptr;
    int mid = (lo + hi) / 2;
    Node* node = sorted[mid];
    node->left = buildBalanced(sorted, lo, mid - 1);
    node->right = buildBalanced(sorted, mid + 1, hi);
    updateSubSize(node);
    node->weight = node->subSize;
    return node;
}

void ScapegoatTree3::updateSubSize(Node* node)
{
    if (node == nullptr) return;
    int leftSize = (node->left) ? node->left->subSize : 0;
    int rightSize = (node->right) ? node->right->subSize : 0;
    node->subSize = 1 + leftSize + rightSize;
    node->weight = 1 + leftSize + rightSize;
}

void ScapegoatTree3::destroyTree(Node* node)
{
    if (node == nullptr) return;
    destroyTree(node->left);
    destroyTree(node->right);
    delete node;
}

void ScapegoatTree3::rangeCollect(Node* node, double lo, double hi,
    QVector<double>& result) const
{
    if (node == nullptr) return;
    if (node->key > lo) rangeCollect(node->left, lo, hi, result);
    if (node->key >= lo && node->key <= hi) result.append(node->key);
    if (node->key < hi) rangeCollect(node->right, lo, hi, result);
}

bool ScapegoatTree3::validateNode(Node* node) const
{
    if (node == nullptr) return true;
    int expected = 1;
    if (node->left) {
        if (!validateNode(node->left)) return false;
        expected += node->left->subSize;
    }
    if (node->right) {
        if (!validateNode(node->right)) return false;
        expected += node->right->subSize;
    }
    if (node->subSize != expected) return false;
    return !isUnbalanced(node);
}

void ScapegoatTree3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

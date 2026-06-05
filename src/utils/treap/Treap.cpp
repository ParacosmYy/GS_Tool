/**
 * @file Treap.cpp
 * @brief Treap(树堆)实现
 */

#include "utils/treap/Treap.h"

#include <QElapsedTimer>

/** @brief 构造函数 @param parent 父对象 */
Treap::Treap(QObject* parent)
    : QObject(parent)
    , m_root(nullptr)
    , m_size(0)
    , m_timeSum(0.0)
{
}

/** @brief 析构函数 */
Treap::~Treap()
{
    destroyTree(m_root);
}

/** @brief 插入键值对 */
void Treap::insert(double key, double value)
{
    QElapsedTimer timer;
    timer.start();

    m_root = insertNode(m_root, key, value);

    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    ++m_stats.totalInserts;
    double total = static_cast<double>(m_stats.totalInserts + m_stats.totalDeletes + m_stats.totalSearches);
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit insertCompleted(key);
}

/** @brief 删除键 */
bool Treap::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    int oldSize = m_size;
    m_root = deleteNode(m_root, key);
    bool success = (m_size < oldSize);

    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    ++m_stats.totalDeletes;
    double total = static_cast<double>(m_stats.totalInserts + m_stats.totalDeletes + m_stats.totalSearches);
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit deleteCompleted(key, success);
    return success;
}

/** @brief 查找键 */
const double* Treap::find(double key) const
{
    return findNode(m_root, key);
}

/** @brief 范围查询 */
QVector<QPair<double, double>> Treap::rangeQuery(double lo, double hi) const
{
    QVector<QPair<double, double>> result;
    rangeQueryNode(m_root, lo, hi, result);
    return result;
}

/** @brief 中序遍历 */
QVector<QPair<double, double>> Treap::inorder() const
{
    QVector<QPair<double, double>> result;
    result.reserve(m_size);
    inorderNode(m_root, result);
    return result;
}

/** @brief 重置统计 */
void Treap::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 右旋 */
Treap::Node* Treap::rotateRight(Node* root)
{
    Node* left = root->left;
    root->left = left->right;
    left->right = root;
    return left;
}

/** @brief 左旋 */
Treap::Node* Treap::rotateLeft(Node* root)
{
    Node* right = root->right;
    root->right = right->left;
    right->left = root;
    return right;
}

/** @brief 递归插入 */
Treap::Node* Treap::insertNode(Node* root, double key, double value)
{
    if (!root) {
        m_size++;
        Node* node = new Node{key, value,
                              QRandomGenerator::global()->bounded(1000000),
                              nullptr, nullptr};
        return node;
    }

    if (key < root->key) {
        root->left = insertNode(root->left, key, value);
        if (root->left->priority > root->priority)
            root = rotateRight(root);
    } else if (key > root->key) {
        root->right = insertNode(root->right, key, value);
        if (root->right->priority > root->priority)
            root = rotateLeft(root);
    } else {
        root->value = value;
    }
    return root;
}

/** @brief 递归删除 */
Treap::Node* Treap::deleteNode(Node* root, double key)
{
    if (!root) return nullptr;

    if (key < root->key) {
        root->left = deleteNode(root->left, key);
    } else if (key > root->key) {
        root->right = deleteNode(root->right, key);
    } else {
        if (!root->left || !root->right) {
            Node* temp = root->left ? root->left : root->right;
            delete root;
            m_size--;
            return temp;
        }
        if (root->left->priority > root->right->priority) {
            root = rotateRight(root);
            root->right = deleteNode(root->right, key);
        } else {
            root = rotateLeft(root);
            root->left = deleteNode(root->left, key);
        }
    }
    return root;
}

/** @brief 递归查找 */
const double* Treap::findNode(Node* root, double key) const
{
    if (!root) return nullptr;
    if (key < root->key) return findNode(root->left, key);
    if (key > root->key) return findNode(root->right, key);
    return &root->value;
}

/** @brief 递归范围查询 */
void Treap::rangeQueryNode(Node* root, double lo, double hi,
                            QVector<QPair<double, double>>& result) const
{
    if (!root) return;
    if (lo < root->key) rangeQueryNode(root->left, lo, hi, result);
    if (lo <= root->key && root->key <= hi)
        result.append({root->key, root->value});
    if (hi > root->key) rangeQueryNode(root->right, lo, hi, result);
}

/** @brief 递归中序遍历 */
void Treap::inorderNode(Node* root,
                         QVector<QPair<double, double>>& result) const
{
    if (!root) return;
    inorderNode(root->left, result);
    result.append({root->key, root->value});
    inorderNode(root->right, result);
}

/** @brief 递归销毁 */
void Treap::destroyTree(Node* root)
{
    if (!root) return;
    destroyTree(root->left);
    destroyTree(root->right);
    delete root;
}

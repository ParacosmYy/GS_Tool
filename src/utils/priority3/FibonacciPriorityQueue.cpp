/**
 * @file FibonacciPriorityQueue.cpp
 * @brief 斐波那契堆优先队列实现
 */

#include "FibonacciPriorityQueue.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---------- 构造/析构 ---------- */

FibonacciPriorityQueue::FibonacciPriorityQueue(QObject* parent)
    : QObject(parent), m_minNode(nullptr), m_size(0)
{
}

FibonacciPriorityQueue::~FibonacciPriorityQueue()
{
    freeAll();
}

/* ---------- 插入 ---------- */

void* FibonacciPriorityQueue::insert(double key, const QVariant& payload)
{
    QElapsedTimer timer;
    timer.start();

    FibNode* node = new FibNode(key, payload);
    node->left = node;
    node->right = node; /* 自循环 */

    addToRootList(node);

    /* 更新最小节点 */
    if (m_minNode == nullptr || key < m_minNode->key) {
        m_minNode = node;
    }
    ++m_size;

    m_stats.totalInsertions++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalInsertions > 0)
        ? m_timeSum / m_stats.totalInsertions : 0.0;

    emit elementInserted(key, m_size);
    return static_cast<void*>(node);
}

/* ---------- 提取最小 ---------- */

bool FibonacciPriorityQueue::extractMin(double* key, QVariant* payload)
{
    QElapsedTimer timer;
    timer.start();

    if (m_minNode == nullptr) return false;

    FibNode* minNode = m_minNode;

    /* 将最小节点的所有子节点添加到根链表 */
    if (minNode->child != nullptr) {
        FibNode* child = minNode->child;
        FibNode* curr = child;
        do {
            FibNode* next = curr->right;
            curr->parent = nullptr;
            addToRootList(curr);
            curr = next;
        } while (curr != child);
    }

    /* 从根链表中移除最小节点 */
    removeFromList(minNode);

    if (minNode == minNode->right) {
        /* 只有这一个节点 */
        m_minNode = nullptr;
    } else {
        m_minNode = minNode->right;
        consolidate();
    }

    --m_size;

    if (key) *key = minNode->key;
    if (payload) *payload = minNode->payload;
    delete minNode;

    m_stats.totalExtractions++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalExtractions > 0)
        ? m_timeSum / m_stats.totalExtractions : 0.0;

    emit elementExtracted(key ? *key : 0.0, m_size);
    return true;
}

/* ---------- decrease-key ---------- */

bool FibonacciPriorityQueue::decreaseKey(void* handle, double newKey)
{
    QElapsedTimer timer;
    timer.start();

    FibNode* node = static_cast<FibNode*>(handle);
    if (node == nullptr) return false;
    if (newKey >= node->key) return false; /* 必须减小 */

    node->key = newKey;
    FibNode* parent = node->parent;

    if (parent != nullptr && node->key < parent->key) {
        /* 从父节点剪切 */
        removeFromList(node);
        parent->degree--;
        if (parent->child == node) {
            parent->child = (node->right != node) ? node->right : nullptr;
        }
        node->parent = nullptr;
        node->marked = false;
        addToRootList(node);

        cascadingCut(parent);
    }

    /* 更新最小指针 */
    if (node->key < m_minNode->key) {
        m_minNode = node;
    }

    m_stats.totalDecreaseKeys++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalDecreaseKeys > 0)
        ? m_timeSum / m_stats.totalDecreaseKeys : 0.0;

    return true;
}

/* ---------- 合并 ---------- */

void FibonacciPriorityQueue::merge(FibonacciPriorityQueue& other)
{
    QElapsedTimer timer;
    timer.start();

    if (other.m_minNode == nullptr) return;

    if (m_minNode == nullptr) {
        m_minNode = other.m_minNode;
    } else {
        /* 拼接两个根链表 */
        FibNode* a = m_minNode;
        FibNode* b = other.m_minNode;
        FibNode* aRight = a->right;
        FibNode* bRight = b->right;

        a->right = bRight;
        bRight->left = a;
        b->right = aRight;
        aRight->left = b;

        if (other.m_minNode->key < m_minNode->key) {
            m_minNode = other.m_minNode;
        }
    }

    m_size += other.m_size;
    other.m_minNode = nullptr;
    other.m_size = 0;

    m_stats.totalMerges++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalMerges > 0)
        ? m_timeSum / m_stats.totalMerges : 0.0;
}

/* ---------- 查看最小 ---------- */

double FibonacciPriorityQueue::peekMinKey() const
{
    return m_minNode ? m_minNode->key : qQNaN();
}

QVariant FibonacciPriorityQueue::peekMinPayload() const
{
    return m_minNode ? m_minNode->payload : QVariant();
}

/* ---------- 统计 ---------- */

FibonacciPriorityQueue::Stats FibonacciPriorityQueue::stats() const
{
    return m_stats;
}

void FibonacciPriorityQueue::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/* ---------- 私有: 加入根链表 ---------- */

void FibonacciPriorityQueue::addToRootList(FibNode* node)
{
    if (m_minNode == nullptr) {
        m_minNode = node;
        node->left = node;
        node->right = node;
        return;
    }

    node->right = m_minNode->right;
    node->left = m_minNode;
    m_minNode->right->left = node;
    m_minNode->right = node;
}

/* ---------- 私有: 从链表中移除 ---------- */

void FibonacciPriorityQueue::removeFromList(FibNode* node)
{
    node->left->right = node->right;
    node->right->left = node->left;
    node->left = node;
    node->right = node;
}

/* ---------- 私有: 合并 ---------- */

void FibonacciPriorityQueue::consolidate()
{
    /* 最大度数上界: floor(log_phi(n)) */
    int maxDegree = 0;
    if (m_size > 1) {
        double phi = (1.0 + qSqrt(5.0)) / 2.0;
        maxDegree = static_cast<int>(qLn(static_cast<double>(m_size))
                      / qLn(phi)) + 1;
    }

    QVector<FibNode*> degreeTable(maxDegree + 1, nullptr);

    /* 收集所有根节点 */
    QVector<FibNode*> roots;
    FibNode* curr = m_minNode;
    if (curr != nullptr) {
        do {
            roots.append(curr);
            curr = curr->right;
        } while (curr != m_minNode);
    }

    /* 合并度相同的树 */
    for (FibNode* root : roots) {
        FibNode* x = root;
        int d = x->degree;

        while (d < degreeTable.size() && degreeTable[d] != nullptr) {
            FibNode* y = degreeTable[d];
            if (x->key > y->key) {
                std::swap(x, y);
            }
            linkTrees(y, x);
            degreeTable[d] = nullptr;
            ++d;
        }

        if (d >= degreeTable.size()) {
            degreeTable.resize(d + 1, nullptr);
        }
        degreeTable[d] = x;
    }

    /* 重建根链表并找最小 */
    m_minNode = nullptr;
    for (FibNode* node : degreeTable) {
        if (node != nullptr) {
            node->left = node;
            node->right = node;
            addToRootList(node);
            if (m_minNode == nullptr || node->key < m_minNode->key) {
                m_minNode = node;
            }
        }
    }
}

/* ---------- 私有: 链接树 ---------- */

void FibonacciPriorityQueue::linkTrees(FibNode* child, FibNode* parent)
{
    removeFromList(child);
    child->parent = parent;

    if (parent->child == nullptr) {
        parent->child = child;
        child->left = child;
        child->right = child;
    } else {
        child->right = parent->child->right;
        child->left = parent->child;
        parent->child->right->left = child;
        parent->child->right = child;
    }

    parent->degree++;
    child->marked = false;
}

/* ---------- 私有: 级联切断 ---------- */

void FibonacciPriorityQueue::cascadingCut(FibNode* node)
{
    FibNode* parent = node->parent;
    if (parent != nullptr) {
        if (!node->marked) {
            node->marked = true;
        } else {
            removeFromList(node);
            parent->degree--;
            if (parent->child == node) {
                parent->child = (node->right != node) ? node->right : nullptr;
            }
            node->parent = nullptr;
            node->marked = false;
            addToRootList(node);
            cascadingCut(parent);
        }
    }
}

/* ---------- 私有: 释放 ---------- */

void FibonacciPriorityQueue::freeNode(FibNode* node)
{
    if (node == nullptr) return;
    FibNode* child = node->child;
    if (child != nullptr) {
        FibNode* curr = child;
        do {
            FibNode* next = curr->right;
            freeNode(curr);
            curr = next;
        } while (curr != child);
    }
    delete node;
}

void FibonacciPriorityQueue::freeAll()
{
    if (m_minNode == nullptr) return;

    FibNode* curr = m_minNode;
    QVector<FibNode*> roots;
    do {
        roots.append(curr);
        curr = curr->right;
    } while (curr != m_minNode);

    for (FibNode* root : roots) {
        root->left = root;
        root->right = root;
        freeNode(root);
    }

    m_minNode = nullptr;
    m_size = 0;
}

/**
 * @file SkewHeap.cpp
 * @brief 斜堆合并优先队列实现
 */

#include "SkewHeap.h"
#include <QElapsedTimer>

SkewHeap::SkewHeap(QObject* parent)
    : QObject(parent)
    , m_root(nullptr)
    , m_size(0)
    , m_timeSum(0.0)
{
}

SkewHeap::~SkewHeap()
{
    destroyTree(m_root);
}

void SkewHeap::insert(double key, int value)
{
    QElapsedTimer timer;
    timer.start();

    Node* newNode = new Node(key, value);
    m_root = mergeNodes(m_root, newNode);
    m_size++;

    m_stats.totalInserts++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalInserts + m_stats.totalExtracts + m_stats.totalMerges;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;
}

bool SkewHeap::extractMin(double& key, int& value)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_root) return false;

    key = m_root->key;
    value = m_root->value;
    Node* oldRoot = m_root;
    m_root = mergeNodes(m_root->left, m_root->right);
    delete oldRoot;
    m_size--;

    m_stats.totalExtracts++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalInserts + m_stats.totalExtracts + m_stats.totalMerges;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    return true;
}

double SkewHeap::peekMin() const
{
    return m_root ? m_root->key : 0.0;
}

void SkewHeap::merge(SkewHeap& other)
{
    QElapsedTimer timer;
    timer.start();

    m_root = mergeNodes(m_root, other.m_root);
    m_size += other.m_size;
    other.m_root = nullptr;
    other.m_size = 0;

    m_stats.totalMerges++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalInserts + m_stats.totalExtracts + m_stats.totalMerges;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    emit mergeCompleted(m_size);
}

int SkewHeap::size() const { return m_size; }
bool SkewHeap::isEmpty() const { return m_root == nullptr; }

void SkewHeap::clear()
{
    destroyTree(m_root);
    m_root = nullptr;
    m_size = 0;
}

SkewHeap::Node* SkewHeap::mergeNodes(Node* a, Node* b)
{
    if (!a) return b;
    if (!b) return a;

    if (a->key > b->key) std::swap(a, b);

    /* 递归合并右子树,然后交换左右子树 */
    a->right = mergeNodes(a->right, b);
    std::swap(a->left, a->right);

    return a;
}

void SkewHeap::destroyTree(Node* node)
{
    if (!node) return;
    destroyTree(node->left);
    destroyTree(node->right);
    delete node;
}

SkewHeap::Stats SkewHeap::stats() const { return m_stats; }

void SkewHeap::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

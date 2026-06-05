/**
 * @file PairingHeap.cpp
 * @brief 配对堆实现
 */

#include "PairingHeap.h"
#include <QElapsedTimer>
#include <QVector>
#include <climits>

PairingHeap::PairingHeap(QObject* parent)
    : QObject(parent)
    , m_root(nullptr)
    , m_count(0)
    , m_timeSum(0.0)
{
}

PairingHeap::~PairingHeap()
{
    deleteTree(m_root);
}

void PairingHeap::deleteTree(Node* node)
{
    if (!node) return;
    deleteTree(node->child);
    deleteTree(node->sibling);
    delete node;
}

PairingHeap::Node* PairingHeap::mergeTrees(Node* a, Node* b)
{
    if (!a) return b;
    if (!b) return a;

    if (a->key > b->key) std::swap(a, b);

    b->sibling = a->child;
    if (a->child) a->child->prev = b;
    b->prev = a;
    a->child = b;
    a->sibling = nullptr;
    return a;
}

PairingHeap::Handle PairingHeap::insert(double key, const QVariant& data)
{
    QElapsedTimer timer;
    timer.start();

    Node* node = new Node{key, data, nullptr, nullptr, nullptr};
    m_root = mergeTrees(m_root, node);
    m_count++;

    m_stats.totalInserts++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalInserts + m_stats.totalExtracts + m_stats.totalMerges;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    return static_cast<Handle>(node);
}

double PairingHeap::findMin() const
{
    return m_root ? m_root->key : std::numeric_limits<double>::max();
}

QPair<double, QVariant> PairingHeap::extractMin()
{
    QElapsedTimer timer;
    timer.start();

    if (!m_root) return {0.0, QVariant()};

    QPair<double, QVariant> result = {m_root->key, m_root->data};
    Node* oldRoot = m_root;
    m_root = twoPassMerge(m_root->child);
    delete oldRoot;
    m_count--;

    m_stats.totalExtracts++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalInserts + m_stats.totalExtracts + m_stats.totalMerges;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    emit minExtracted(result.first);
    return result;
}

PairingHeap::Node* PairingHeap::twoPassMerge(Node* node)
{
    if (!node || !node->sibling) return node;

    /* 第一遍: 从左到右两两合并 */
    QVector<Node*> trees;
    Node* cur = node;
    while (cur) {
        Node* next = cur->sibling;
        cur->prev = nullptr;
        cur->sibling = nullptr;
        if (next) {
            Node* nextNext = next->sibling;
            next->prev = nullptr;
            next->sibling = nullptr;
            trees.append(mergeTrees(cur, next));
            cur = nextNext;
        } else {
            trees.append(cur);
            cur = nullptr;
        }
    }

    /* 第二遍: 从右到左依次合并 */
    Node* result = trees.last();
    for (int i = trees.size() - 2; i >= 0; --i)
        result = mergeTrees(trees[i], result);

    return result;
}

void PairingHeap::decreaseKey(Handle h, double newKey)
{
    QElapsedTimer timer;
    timer.start();

    Node* node = static_cast<Node*>(h);
    if (!node || newKey > node->key) return;

    node->key = newKey;

    if (node == m_root) return;

    /* 从父/兄弟链中断开 */
    if (node->prev) {
        if (node->prev->child == node)
            node->prev->child = node->sibling;
        else
            node->prev->sibling = node->sibling;
    }
    if (node->sibling) node->sibling->prev = node->prev;

    node->sibling = nullptr;
    node->prev = nullptr;
    m_root = mergeTrees(m_root, node);

    m_stats.totalDecreaseKeys++;
    m_timeSum += timer.elapsed();
}

void PairingHeap::remove(Handle h)
{
    Node* node = static_cast<Node*>(h);
    if (!node) return;

    decreaseKey(h, std::numeric_limits<double>::lowest());
    extractMin();
}

void PairingHeap::merge(PairingHeap& other)
{
    QElapsedTimer timer;
    timer.start();

    m_root = mergeTrees(m_root, other.m_root);
    m_count += other.m_count;
    other.m_root = nullptr;
    other.m_count = 0;

    m_stats.totalMerges++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalInserts + m_stats.totalExtracts + m_stats.totalMerges;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;
}

bool PairingHeap::isEmpty() const { return m_root == nullptr; }
int PairingHeap::size() const { return m_count; }
PairingHeap::Stats PairingHeap::stats() const { return m_stats; }

void PairingHeap::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

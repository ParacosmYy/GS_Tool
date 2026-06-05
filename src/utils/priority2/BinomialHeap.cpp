/**
 * @file BinomialHeap.cpp
 * @brief 二项堆实现
 */

#include "BinomialHeap.h"
#include <QElapsedTimer>
#include <algorithm>
#include <climits>

BinomialHeap::BinomialHeap(QObject* parent)
    : QObject(parent), m_head(nullptr), m_count(0), m_timeSum(0.0)
{
}

BinomialHeap::~BinomialHeap()
{
    clearTree(m_head);
}

void BinomialHeap::clearTree(Node* node)
{
    if (!node) return;
    clearTree(node->child);
    clearTree(node->sibling);
    delete node;
}

void BinomialHeap::linkTree(Node* child, Node* parent)
{
    child->parent = parent;
    child->sibling = parent->child;
    parent->child = child;
    parent->degree++;
}

BinomialHeap::Node* BinomialHeap::mergeRoots(Node* h1, Node* h2)
{
    if (!h1) return h2;
    if (!h2) return h1;

    Node* head = nullptr;
    Node** pos = &head;

    while (h1 && h2) {
        if (h1->degree <= h2->degree) {
            *pos = h1;
            h1 = h1->sibling;
        } else {
            *pos = h2;
            h2 = h2->sibling;
        }
        pos = &((*pos)->sibling);
    }
    *pos = h1 ? h1 : h2;
    return head;
}

BinomialHeap::Node* BinomialHeap::unionHeaps(Node* h1, Node* h2)
{
    Node* head = mergeRoots(h1, h2);
    if (!head) return nullptr;

    Node* prev = nullptr;
    Node* curr = head;
    Node* next = curr->sibling;

    while (next) {
        bool mergeNeeded = false;
        if (curr->degree == next->degree) {
            if (next->sibling && next->sibling->degree == curr->degree) {
                /* 三个同度: 跳过第一个 */
                mergeNeeded = false;
            } else {
                mergeNeeded = true;
            }
        }

        if (mergeNeeded) {
            if (curr->key <= next->key) {
                curr->sibling = next->sibling;
                linkTree(next, curr);
            } else {
                if (!prev) head = next;
                else prev->sibling = next;
                linkTree(curr, next);
                curr = next;
            }
            next = curr->sibling;
        } else {
            prev = curr;
            curr = next;
            next = next->sibling;
        }
    }
    return head;
}

void BinomialHeap::insert(double key, const QVariant& data)
{
    QElapsedTimer timer;
    timer.start();

    Node* newNode = new Node{key, data, 0, nullptr, nullptr, nullptr};
    m_head = unionHeaps(m_head, newNode);
    m_count++;

    m_stats.totalInserts++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalInserts + m_stats.totalExtracts + m_stats.totalMerges;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;
}

double BinomialHeap::findMin() const
{
    Node* minNode = findMinNode();
    return minNode ? minNode->key : std::numeric_limits<double>::max();
}

BinomialHeap::Node* BinomialHeap::findMinNode() const
{
    if (!m_head) return nullptr;
    Node* minN = m_head;
    Node* cur = m_head->sibling;
    while (cur) {
        if (cur->key < minN->key) minN = cur;
        cur = cur->sibling;
    }
    return minN;
}

BinomialHeap::Node* BinomialHeap::reverseList(Node* node)
{
    Node* prev = nullptr;
    Node* curr = node;
    while (curr) {
        Node* next = curr->sibling;
        curr->sibling = prev;
        curr->parent = nullptr;
        prev = curr;
        curr = next;
    }
    return prev;
}

QPair<double, QVariant> BinomialHeap::extractMin()
{
    QElapsedTimer timer;
    timer.start();

    if (!m_head) return {0.0, QVariant()};

    /* 找最小节点及其前驱 */
    Node* minPrev = nullptr;
    Node* minNode = m_head;
    Node* prev = nullptr;
    Node* cur = m_head;

    while (cur) {
        if (cur->key < minNode->key) {
            minNode = cur;
            minPrev = prev;
        }
        prev = cur;
        cur = cur->sibling;
    }

    /* 从链表中移除minNode */
    if (minPrev) minPrev->sibling = minNode->sibling;
    else m_head = minNode->sibling;

    /* 反转子节点链表 */
    Node* childList = reverseList(minNode->child);

    QPair<double, QVariant> result = {minNode->key, minNode->data};

    m_head = unionHeaps(m_head, childList);
    delete minNode;
    m_count--;

    m_stats.totalExtracts++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalInserts + m_stats.totalExtracts + m_stats.totalMerges;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    emit minExtracted(result.first);
    return result;
}

void BinomialHeap::merge(BinomialHeap& other)
{
    QElapsedTimer timer;
    timer.start();

    m_head = unionHeaps(m_head, other.m_head);
    m_count += other.m_count;
    other.m_head = nullptr;
    other.m_count = 0;

    m_stats.totalMerges++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalInserts + m_stats.totalExtracts + m_stats.totalMerges;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;
}

void BinomialHeap::decreaseKey(Node* node, double newKey)
{
    if (!node || newKey > node->key) return;
    node->key = newKey;
    while (node->parent && node->key < node->parent->key) {
        std::swap(node->key, node->parent->key);
        std::swap(node->data, node->parent->data);
        node = node->parent;
    }
    m_stats.totalDecreaseKeys++;
}

void BinomialHeap::remove(Node* node)
{
    decreaseKey(node, std::numeric_limits<double>::lowest());
    extractMin();
}

bool BinomialHeap::isEmpty() const { return m_head == nullptr; }
int BinomialHeap::size() const { return m_count; }
BinomialHeap::Stats BinomialHeap::stats() const { return m_stats; }

void BinomialHeap::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

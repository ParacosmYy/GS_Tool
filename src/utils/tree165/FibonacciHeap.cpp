/**
 * @file FibonacciHeap.cpp
 * @brief FibonacciHeap 实现
 *
 * 实现斐波那契堆：O(1)插入/降键/合并，lazy consolidate
 * delete-min，级联切断维护堆性质。
 */

#include "utils/tree165/FibonacciHeap.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

FibonacciHeap::FibonacciHeap(QObject* parent)
    : QObject(parent)
{
}

FibonacciHeap::~FibonacciHeap()
{
    clear();
}

void FibonacciHeap::insertIntoRootList(Node* node)
{
    node->parent = nullptr;
    node->marked = false;

    if (!m_min) {
        m_min = node;
        node->left = node;
        node->right = node;
    } else {
        /* Insert to the right of m_min */
        node->left = m_min;
        node->right = m_min->right;
        m_min->right->left = node;
        m_min->right = node;
    }
}

void FibonacciHeap::removeFromList(Node* node)
{
    node->left->right = node->right;
    node->right->left = node->left;
    /* Don't clear node's own pointers; caller handles that */
}

void FibonacciHeap::updateMin()
{
    if (!m_min) return;
    Node* start = m_min;
    Node* curr = m_min->right;
    Node* best = m_min;

    while (curr != start) {
        if (curr->key < best->key) best = curr;
        curr = curr->right;
    }
    m_min = best;
}

void FibonacciHeap::link(Node* child, Node* parent)
{
    /* Remove child from root list */
    removeFromList(child);

    /* Make child a child of parent */
    child->parent = parent;
    child->marked = false;

    if (!parent->child) {
        parent->child = child;
        child->left = child;
        child->right = child;
    } else {
        child->left = parent->child;
        child->right = parent->child->right;
        parent->child->right->left = child;
        parent->child->right = child;
    }
    parent->degree++;
}

void FibonacciHeap::consolidate()
{
    if (!m_min) return;

    /* Max degree <= floor(log_phi(n)) ≈ 1.445 * log2(n) */
    int maxDeg = 0;
    int size = m_stats.currentSize;
    while (size > 0) { maxDeg++; size >>= 1; }
    maxDeg = maxDeg * 2 + 2;

    QVector<Node*> degreeTable(maxDeg + 1, nullptr);

    /* Collect all root nodes */
    QVector<Node*> roots;
    Node* start = m_min;
    Node* curr = m_min;
    do {
        roots.append(curr);
        curr = curr->right;
    } while (curr != start);

    for (Node* node : roots) {
        int d = node->degree;
        while (d < degreeTable.size() && degreeTable[d] != nullptr) {
            Node* other = degreeTable[d];
            if (node->key > other->key) std::swap(node, other);
            link(other, node);
            degreeTable[d] = nullptr;
            d++;
        }
            if (d >= degreeTable.size()) degreeTable.resize(d + 1, nullptr);
        degreeTable[d] = node;
    }

    /* Rebuild root list and find new min */
    m_min = nullptr;
    for (Node* node : degreeTable) {
        if (node) {
            if (!m_min) {
                m_min = node;
                node->left = node;
                node->right = node;
            } else {
                insertIntoRootList(node);
                if (node->key < m_min->key) m_min = node;
            }
        }
    }
}

void FibonacciHeap::cut(Node* x, Node* parent)
{
    /* Remove x from parent's child list */
    if (parent->child == x) {
        if (x->right == x) {
            parent->child = nullptr;
        } else {
            parent->child = x->right;
        }
    }
    removeFromList(x);
    parent->degree--;

    /* Add x to root list */
    insertIntoRootList(x);
    x->parent = nullptr;
    x->marked = false;
}

void FibonacciHeap::cascadingCut(Node* x)
{
    Node* parent = x->parent;
    if (parent) {
        if (!x->marked) {
            x->marked = true;
        } else {
            cut(x, parent);
            cascadingCut(parent);
        }
    }
}

void FibonacciHeap::lazyMerge(FibonacciHeap& other)
{
    if (!other.m_min) return;

    if (!m_min) {
        m_min = other.m_min;
    } else {
        /* Concatenate root lists */
        Node* aRight = m_min->right;
        Node* bRight = other.m_min->right;

        m_min->right = bRight;
        bRight->left = m_min;
        other.m_min->right = aRight;
        aRight->left = other.m_min;

        if (other.m_min->key < m_min->key) m_min = other.m_min;
    }

    /* Transfer node map */
    for (auto it = other.m_nodeMap.begin(); it != other.m_nodeMap.end(); ++it) {
        m_nodeMap.insert(it.key(), it.value());
    }
    m_stats.currentSize += other.m_stats.currentSize;

    other.m_min = nullptr;
    other.m_nodeMap.clear();
    other.m_stats.currentSize = 0;
}

int FibonacciHeap::insert(double key, double value)
{
    QElapsedTimer timer;
    timer.start();

    Node* node = new Node{key, value, m_nextId, 0, false,
                          nullptr, nullptr, nullptr, nullptr};
    node->left = node;
    node->right = node;
    m_nodeMap[m_nextId] = node;

    insertIntoRootList(node);
    if (node->key < m_min->key) m_min = node;

    m_stats.totalInserts++;
    m_stats.currentSize++;
    m_timeSum += timer.elapsed();
    quint64 total = m_stats.totalInserts + m_stats.totalDeleteMins;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit insertCompleted(key, m_stats.currentSize);
    return m_nextId++;
}

bool FibonacciHeap::findMin(double& key, double& value) const
{
    if (!m_min) return false;
    key = m_min->key;
    value = m_min->value;
    return true;
}

bool FibonacciHeap::deleteMin(double& key, double& value)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_min) return false;

    key = m_min->key;
    value = m_min->value;
    Node* oldMin = m_min;
    m_nodeMap.remove(oldMin->id);

    /* Add all children to root list */
    if (oldMin->child) {
        Node* child = oldMin->child;
        QVector<Node*> children;
        Node* c = child;
        do {
            children.append(c);
            c = c->right;
        } while (c != child);

        for (Node* ch : children) {
            insertIntoRootList(ch);
        }
    }

    /* Remove oldMin from root list */
    if (oldMin->right == oldMin) {
        m_min = nullptr;
    } else {
        m_min = oldMin->right;
        removeFromList(oldMin);
        consolidate();
        m_stats.totalConsolidations++;
    }

    delete oldMin;

    m_stats.totalDeleteMins++;
    m_stats.currentSize--;
    m_timeSum += timer.elapsed();
    quint64 total = m_stats.totalInserts + m_stats.totalDeleteMins;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit deleteMinCompleted(key);
    return true;
}

bool FibonacciHeap::decreaseKey(int id, double newKey)
{
    QElapsedTimer timer;
    timer.start();

    auto it = m_nodeMap.find(id);
    if (it == m_nodeMap.end()) return false;

    Node* node = it.value();
    if (newKey >= node->key) return false;

    node->key = newKey;
    Node* parent = node->parent;

    if (parent && node->key < parent->key) {
        cut(node, parent);
        cascadingCut(parent);
    }

    if (node->key < m_min->key) m_min = node;

    m_stats.totalDecreaseKeys++;
    m_timeSum += timer.elapsed();
    quint64 total = m_stats.totalInserts + m_stats.totalDeleteMins;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    return true;
}

bool FibonacciHeap::remove(int id)
{
    auto it = m_nodeMap.find(id);
    if (it == m_nodeMap.end()) return false;

    /* Decrease key to -infinity then delete-min */
    decreaseKey(id, -std::numeric_limits<double>::max());

    double k, v;
    return deleteMin(k, v);
}

void FibonacciHeap::merge(FibonacciHeap& other)
{
    lazyMerge(other);
}

bool FibonacciHeap::isEmpty() const { return m_min == nullptr; }
int FibonacciHeap::size() const { return m_stats.currentSize; }

void FibonacciHeap::deleteTree(Node* node)
{
    if (!node) return;
    /* Delete children first */
    if (node->child) {
        Node* c = node->child;
        QVector<Node*> children;
        Node* curr = c;
        do {
            children.append(curr);
            curr = curr->right;
        } while (curr != c);
        for (Node* ch : children) deleteTree(ch);
    }
    m_nodeMap.remove(node->id);
    delete node;
}

void FibonacciHeap::clear()
{
    if (!m_min) return;

    QVector<Node*> roots;
    Node* curr = m_min;
    do {
        roots.append(curr);
        curr = curr->right;
    } while (curr != m_min);

    for (Node* r : roots) {
        r->left->right = r->right;
        r->right->left = r->left;
        deleteTree(r);
    }

    m_min = nullptr;
    m_nodeMap.clear();
    m_stats.currentSize = 0;
}

void FibonacciHeap::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

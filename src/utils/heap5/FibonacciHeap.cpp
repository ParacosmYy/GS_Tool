/**
 * @file FibonacciHeap.cpp
 * @brief 斐波那契堆实现
 */

#include "FibonacciHeap.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

FibonacciHeap::FibonacciHeap(QObject* parent)
    : QObject(parent)
    , m_min(nullptr)
    , m_n(0)
    , m_timeSum(0.0)
{
}

FibonacciHeap::~FibonacciHeap()
{
    if (m_min) {
        Node* current = m_min;
        do {
            Node* next = current->right;
            destroyTree(current);
            current = next;
        } while (current != m_min);
    }
}

void FibonacciHeap::destroyTree(Node* root)
{
    if (!root) return;
    Node* child = root->child;
    if (child) {
        Node* c = child;
        do {
            Node* next = c->right;
            destroyTree(c);
            c = next;
        } while (c != child);
    }
    delete root;
}

FibonacciHeap::Node* FibonacciHeap::insert(double key, int data)
{
    QElapsedTimer timer;
    timer.start();

    Node* node = new Node{key, data, nullptr, nullptr, nullptr, nullptr, 0, false};
    node->left = node;
    node->right = node;

    if (!m_min) {
        m_min = node;
    } else {
        node->left = m_min;
        node->right = m_min->right;
        m_min->right->left = node;
        m_min->right = node;
        if (key < m_min->key) m_min = node;
    }
    m_n++;

    m_stats.totalInserts++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalInserts + m_stats.totalExtracts + m_stats.totalDecreaseKeys;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    return node;
}

FibonacciHeap::Node* FibonacciHeap::min() const { return m_min; }

QPair<double, int> FibonacciHeap::extractMin()
{
    QElapsedTimer timer;
    timer.start();

    if (!m_min) return {0, 0};

    Node* z = m_min;
    if (z->child) {
        Node* child = z->child;
        Node* start = child;
        do {
            Node* next = child->right;
            child->left = m_min;
            child->right = m_min->right;
            m_min->right->left = child;
            m_min->right = child;
            child->parent = nullptr;
            child = next;
        } while (child != start);
    }

    z->left->right = z->right;
    z->right->left = z->left;

    if (z == z->right) {
        m_min = nullptr;
    } else {
        m_min = z->right;
        consolidate();
    }
    m_n--;

    QPair<double, int> result = {z->key, z->data};
    delete z;

    m_stats.totalExtracts++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalInserts + m_stats.totalExtracts + m_stats.totalDecreaseKeys;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    emit extracted(result.first);
    return result;
}

void FibonacciHeap::decreaseKey(Node* node, double newKey)
{
    if (newKey > node->key) return;

    node->key = newKey;
    Node* y = node->parent;

    if (y && node->key < y->key) {
        cut(node, y);
        cascadingCut(y);
    }

    if (node->key < m_min->key) m_min = node;

    m_stats.totalDecreaseKeys++;
}

void FibonacciHeap::remove(Node* node)
{
    decreaseKey(node, -std::numeric_limits<double>::max());
    extractMin();
}

bool FibonacciHeap::isEmpty() const { return m_n == 0; }
int FibonacciHeap::size() const { return m_n; }

void FibonacciHeap::consolidate()
{
    int maxDeg = static_cast<int>(std::log2(m_n)) + 2;
    QVector<Node*> A(maxDeg, nullptr);

    QVector<Node*> roots;
    Node* start = m_min;
    Node* curr = start;
    do {
        roots.append(curr);
        curr = curr->right;
    } while (curr != start);

    for (Node* w : roots) {
        Node* x = w;
        int d = x->degree;
        while (d < maxDeg && A[d]) {
            Node* y = A[d];
            if (x->key > y->key) std::swap(x, y);
            link(y, x);
            A[d] = nullptr;
            d++;
        }
        if (d < maxDeg) A[d] = x;
    }

    m_min = nullptr;
    for (int i = 0; i < maxDeg; ++i) {
        if (A[i]) {
            if (!m_min) {
                m_min = A[i];
                m_min->left = m_min;
                m_min->right = m_min;
            } else {
                A[i]->left = m_min;
                A[i]->right = m_min->right;
                m_min->right->left = A[i];
                m_min->right = A[i];
                if (A[i]->key < m_min->key) m_min = A[i];
            }
        }
    }
}

void FibonacciHeap::link(Node* y, Node* x)
{
    y->left->right = y->right;
    y->right->left = y->left;

    if (!x->child) {
        x->child = y;
        y->left = y;
        y->right = y;
    } else {
        y->left = x->child;
        y->right = x->child->right;
        x->child->right->left = y;
        x->child->right = y;
    }
    y->parent = x;
    x->degree++;
    y->marked = false;
}

void FibonacciHeap::cut(Node* x, Node* y)
{
    if (x->right == x) {
        y->child = nullptr;
    } else {
        x->left->right = x->right;
        x->right->left = x->left;
        if (y->child == x) y->child = x->right;
    }
    y->degree--;

    x->left = m_min;
    x->right = m_min->right;
    m_min->right->left = x;
    m_min->right = x;
    x->parent = nullptr;
    x->marked = false;
}

void FibonacciHeap::cascadingCut(Node* y)
{
    Node* z = y->parent;
    if (z) {
        if (!y->marked) {
            y->marked = true;
        } else {
            cut(y, z);
            cascadingCut(z);
        }
    }
}

FibonacciHeap::Stats FibonacciHeap::stats() const { return m_stats; }

void FibonacciHeap::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

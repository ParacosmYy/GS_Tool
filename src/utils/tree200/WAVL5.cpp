/**
 * @file WAVL5.cpp
 * @brief WAVL5 实现
 *
 * 实现弱AVL树：秩差不变量维护、旋转再平衡、摊还常数级更新操作。
 */

#include "utils/tree200/WAVL5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

WAVL5::WAVL5(QObject *parent) : QObject(parent) {}
WAVL5::~WAVL5() { delete m_root; }

/* ---- Rank helpers ---- */

int WAVL5::getRank(const Node* n) { return n ? n->rank : -1; }

int WAVL5::rankDiff(const Node* parent, const Node* child)
{
    return parent ? getRank(parent) - getRank(child) : 0;
}

/* ---- Rotations ---- */

WAVL5::Node* WAVL5::rotateLeft(Node* x)
{
    Node* y = x->right;
    x->right = y->left;
    if (y->left) y->left->parent = x;
    y->parent = x->parent;
    if (!x->parent) m_root = y;
    else if (x == x->parent->left) x->parent->left = y;
    else x->parent->right = y;
    y->left = x;
    x->parent = y;
    return y;
}

WAVL5::Node* WAVL5::rotateRight(Node* y)
{
    Node* x = y->left;
    y->left = x->right;
    if (x->right) x->right->parent = y;
    x->parent = y->parent;
    if (!y->parent) m_root = x;
    else if (y == y->parent->left) y->parent->left = x;
    else y->parent->right = x;
    x->right = y;
    y->parent = x;
    return x;
}

/* ---- Rebalance after insert ---- */

void WAVL5::rebalanceInsert(Node* node)
{
    // After insertion, node has rank 0, parent has rank >= 0
    // Walk up fixing rank-difference violations
    while (node && node->parent) {
        Node* p = node->parent;
        int diff = rankDiff(p, node);

        if (diff == 0) {
            // 0-child violation: need to promote parent
            p->rank++;
            node = p;
        } else if (diff == 1) {
            // 1-child: balanced, done
            break;
        } else {
            // Should not happen after proper promotion
            break;
        }

        // Check if promoted node causes imbalance with its sibling
        if (node->parent) {
            Node* sib = (node == node->parent->left)
                        ? node->parent->right : node->parent->left;
            int sibDiff = rankDiff(node->parent, sib);

            if (sibDiff == 0 && rankDiff(node->parent, node) == 0) {
                // Double 0-child: promote parent again
                node->parent->rank++;
                node = node->parent;
            } else if (sibDiff >= 2 && rankDiff(node->parent, node) == 0) {
                // Need rotation
                bool isLeft = (node == node->parent->left);
                Node* child = isLeft ? node->right : node->left;

                if (child && rankDiff(node, child) == 2) {
                    // Single rotation
                    node->parent->rank--;
                    if (isLeft) rotateLeft(node->parent);
                    else rotateRight(node->parent);
                } else if (child) {
                    // Double rotation
                    child->rank++;
                    node->rank--;
                    node->parent->rank--;
                    if (isLeft) {
                        rotateRight(node);
                        rotateLeft(child->parent);
                    } else {
                        rotateLeft(node);
                        rotateRight(child->parent);
                    }
                }
                break;
            }
        }
    }
}

/* ---- Rebalance after remove ---- */

void WAVL5::rebalanceRemove(Node* parent, Node* node)
{
    while (parent) {
        Node* sib = (node == parent->left) ? parent->right : parent->left;
        int myDiff = rankDiff(parent, node);
        int sibDiff = rankDiff(parent, sib);

        if (myDiff <= 2) break; // No violation

        if (sibDiff == 1) {
            // Demote parent
            parent->rank--;
            node = parent;
            parent = parent->parent;
        } else if (sibDiff == 2) {
            break; // Balanced
        } else {
            // sibDiff == 0 or sib has 1-child children -> rotate
            bool isLeft = (node == parent->left);
            if (sib) {
                int sibLDiff = rankDiff(sib, sib->left);
                int sibRDiff = rankDiff(sib, sib->right);

                if (sibLDiff == 1 && sibRDiff == 1) {
                    // Both children of sibling are 1-children: demote sibling and parent
                    sib->rank--;
                    parent->rank--;
                    node = parent;
                    parent = parent->parent;
                } else if ((isLeft && sibLDiff == 1) || (!isLeft && sibRDiff == 1)) {
                    // Single rotation
                    if (isLeft) rotateLeft(parent); else rotateRight(parent);
                    sib->rank++;
                    parent->rank -= 2;
                    break;
                } else {
                    // Double rotation
                    Node* innerChild = isLeft ? sib->left : sib->right;
                    if (innerChild) {
                        innerChild->rank += 2;
                        sib->rank--;
                        parent->rank -= 2;
                        if (isLeft) {
                            rotateRight(sib);
                            rotateLeft(parent);
                        } else {
                            rotateLeft(sib);
                            rotateRight(parent);
                        }
                    }
                    break;
                }
            } else {
                break;
            }
        }
    }
}

/* ---- Insert ---- */

void WAVL5::insert(double key, double value)
{
    QElapsedTimer timer;
    timer.start();

    Node* newNode = new Node;
    newNode->key = key;
    newNode->value = value;
    newNode->rank = 0;

    if (!m_root) {
        m_root = newNode;
        newNode->rank = 1;
    } else {
        Node* cur = m_root;
        Node* par = nullptr;
        while (cur) {
            par = cur;
            if (key < cur->key) cur = cur->left;
            else if (key > cur->key) cur = cur->right;
            else { cur->value = value; delete newNode; goto done; }
        }

        newNode->parent = par;
        if (key < par->key) par->left = newNode;
        else par->right = newNode;

        rebalanceInsert(newNode);
    }

done:
    m_stats.totalOperations++;
    m_stats.nodeCount = size();
    m_stats.treeHeight = computeHeight(m_root);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit treeUpdated("insert", size(), computeHeight(m_root), timer.elapsed());
}

/* ---- Transplant ---- */

void WAVL5::transplant(Node* u, Node* v)
{
    if (!u->parent) m_root = v;
    else if (u == u->parent->left) u->parent->left = v;
    else u->parent->right = v;
    if (v) v->parent = u->parent;
}

/* ---- Tree minimum ---- */

WAVL5::Node* WAVL5::treeMinimum(Node* n)
{
    while (n && n->left) n = n->left;
    return n;
}

/* ---- Remove ---- */

bool WAVL5::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    Node* z = findNode(key);
    if (!z) return false;

    Node* rebalParent = nullptr;
    Node* rebalNode = nullptr;

    if (!z->left) {
        rebalParent = z->parent;
        rebalNode = z->right;
        transplant(z, z->right);
    } else if (!z->right) {
        rebalParent = z->parent;
        rebalNode = z->left;
        transplant(z, z->left);
    } else {
        Node* y = treeMinimum(z->right);
        if (y->parent != z) {
            rebalParent = y->parent;
            rebalNode = y->right;
            transplant(y, y->right);
            y->right = z->right;
            y->right->parent = y;
        } else {
            rebalParent = y;
            rebalNode = y->right;
        }
        transplant(z, y);
        y->left = z->left;
        y->left->parent = y;
        y->rank = z->rank;
    }

    z->left = nullptr; z->right = nullptr;
    delete z;

    if (rebalParent) rebalanceRemove(rebalParent, rebalNode);

    m_stats.totalOperations++;
    m_stats.nodeCount = size();
    m_stats.treeHeight = computeHeight(m_root);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit treeUpdated("remove", size(), computeHeight(m_root), timer.elapsed());
    return true;
}

/* ---- Find node ---- */

WAVL5::Node* WAVL5::findNode(double key) const
{
    Node* cur = m_root;
    while (cur) {
        if (key < cur->key) cur = cur->left;
        else if (key > cur->key) cur = cur->right;
        else return cur;
    }
    return nullptr;
}

/* ---- Find / Contains ---- */

double WAVL5::find(double key, double defaultValue) const
{
    Node* n = findNode(key);
    return n ? n->value : defaultValue;
}

bool WAVL5::contains(double key) const { return findNode(key) != nullptr; }

/* ---- In-order traversal ---- */

void WAVL5::inOrder(Node* n, QVector<QPair<double, double>>& result) const
{
    if (!n) return;
    inOrder(n->left, result);
    result.append({n->key, n->value});
    inOrder(n->right, result);
}

QVector<QPair<double, double>> WAVL5::toVector() const
{
    QVector<QPair<double, double>> result;
    inOrder(m_root, result);
    return result;
}

/* ---- Range query ---- */

void WAVL5::rangeInOrder(Node* n, double lo, double hi,
                          QVector<QPair<double, double>>& result) const
{
    if (!n) return;
    if (n->key > lo) rangeInOrder(n->left, lo, hi, result);
    if (n->key >= lo && n->key <= hi) result.append({n->key, n->value});
    if (n->key < hi) rangeInOrder(n->right, lo, hi, result);
}

QVector<QPair<double, double>> WAVL5::rangeQuery(double lo, double hi) const
{
    QVector<QPair<double, double>> result;
    rangeInOrder(m_root, lo, hi, result);
    return result;
}

/* ---- Min / Max ---- */

double WAVL5::minimumKey() const
{
    if (!m_root) return 0.0;
    Node* n = treeMinimum(m_root);
    return n ? n->key : 0.0;
}

double WAVL5::maximumKey() const
{
    if (!m_root) return 0.0;
    Node* n = m_root;
    while (n->right) n = n->right;
    return n->key;
}

/* ---- Size / Empty ---- */

int WAVL5::size() const
{
    int count = 0;
    QVector<Node*> stack;
    if (m_root) stack.append(m_root);
    while (!stack.isEmpty()) {
        Node* n = stack.takeLast();
        count++;
        if (n->left) stack.append(n->left);
        if (n->right) stack.append(n->right);
    }
    return count;
}

bool WAVL5::isEmpty() const { return m_root == nullptr; }

/* ---- Height ---- */

int WAVL5::computeHeight(const Node* n)
{
    if (!n) return 0;
    return 1 + qMax(computeHeight(n->left), computeHeight(n->right));
}

/* ---- Reset ---- */

void WAVL5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    delete m_root;
    m_root = nullptr;
}

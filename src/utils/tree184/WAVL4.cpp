/**
 * @file WAVL4.cpp
 * @brief WAVL4 实现
 *
 * 实现弱AVL树：秩平衡不变量、自底向上再平衡、join/split操作。
 */

#include "utils/tree184/WAVL4.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

WAVL4::WAVL4(QObject *parent) : QObject(parent) {}
WAVL4::~WAVL4() { deleteTree(m_root); }

void WAVL4::deleteTree(Node* n)
{
    if (!n) return;
    deleteTree(n->left);
    deleteTree(n->right);
    delete n;
}

/* ---- Rank helpers ---- */

int WAVL4::nodeRank(Node* n) const
{
    return n ? n->rank : -1;
}

int WAVL4::rankDiff(Node* parent, Node* child) const
{
    return parent ? (parent->rank - nodeRank(child)) : 1;
}

/* ---- Rotations ---- */

WAVL4::Node* WAVL4::rotateRight(Node* x)
{
    if (!x || !x->left) return x;
    Node* y = x->left;
    x->left = y->right;
    if (y->right) y->right->parent = x;
    y->right = x;
    y->parent = x->parent;
    x->parent = y;
    return y;
}

WAVL4::Node* WAVL4::rotateLeft(Node* x)
{
    if (!x || !x->right) return x;
    Node* y = x->right;
    x->right = y->left;
    if (y->left) y->left->parent = x;
    y->left = x;
    y->parent = x->parent;
    x->parent = y;
    return y;
}

/* ---- Rank operations ---- */

void WAVL4::promote(Node* n) { if (n) n->rank++; }
void WAVL4::demote(Node* n) { if (n) n->rank--; }

/* ---- Rebalance after insert ---- */

void WAVL4::rebalanceInsert(Node* n)
{
    while (n && n->parent) {
        Node* p = n->parent;
        int rd = rankDiff(p, n);

        if (rd == 0) {
            // Rank difference 0: need to fix
            int rdOther;
            bool isLeft = (p->left == n);
            Node* other = isLeft ? p->right : p->left;
            rdOther = rankDiff(p, other);

            if (rdOther == 1) {
                // Case: (0,1) -> promote parent
                promote(p);
                n = p;
                continue;
            } else {
                // Case: (0,2) -> rotate
                int childRd = isLeft ? rankDiff(n, n->right) : rankDiff(n, n->left);

                if (childRd == 2) {
                    // Single rotation
                    if (isLeft) {
                        Node* gp = p->parent;
                        Node* newRoot = rotateRight(p);
                        if (gp) { if (gp->left == p) gp->left = newRoot; else gp->right = newRoot; }
                        else m_root = newRoot;
                        promote(p); demote(p);
                    } else {
                        Node* gp = p->parent;
                        Node* newRoot = rotateLeft(p);
                        if (gp) { if (gp->left == p) gp->left = newRoot; else gp->right = newRoot; }
                        else m_root = newRoot;
                        promote(p); demote(p);
                    }
                } else {
                    // Double rotation
                    if (isLeft) {
                        p->left = rotateLeft(n);
                        Node* gp = p->parent;
                        Node* newRoot = rotateRight(p);
                        if (gp) { if (gp->left == p) gp->left = newRoot; else gp->right = newRoot; }
                        else m_root = newRoot;
                        promote(newRoot);
                        demote(p); demote(p);
                    } else {
                        p->right = rotateRight(n);
                        Node* gp = p->parent;
                        Node* newRoot = rotateLeft(p);
                        if (gp) { if (gp->left == p) gp->left = newRoot; else gp->right = newRoot; }
                        else m_root = newRoot;
                        promote(newRoot);
                        demote(p); demote(p);
                    }
                }
                break;
            }
        } else {
            // Rank difference >= 1: invariant satisfied
            break;
        }
    }
}

/* ---- Rebalance after remove ---- */

void WAVL4::rebalanceRemove(Node* parent)
{
    while (parent) {
        int rdL = rankDiff(parent, parent->left);
        int rdR = rankDiff(parent, parent->right);

        if (rdL == 2 && rdR == 2) {
            // Both children have rank diff 2: demote
            demote(parent);
            parent = parent->parent;
            continue;
        }

        if (rdL == 3 || rdR == 3) {
            bool isLeftHeavy = (rdL == 3);
            Node* t = isLeftHeavy ? parent->right : parent->left;
            int tRdL = rankDiff(t, t->left);
            int tRdR = rankDiff(t, t->right);

            if (tRdL == 2 && tRdR == 2) {
                // Both children of sibling have rank diff 2
                demote(parent);
                demote(t);
                parent = parent->parent;
                continue;
            }

            // Rotate
            Node* gp = parent->parent;
            if (isLeftHeavy) {
                int childRd = rankDiff(t, t->left);
                if (childRd == 1) {
                    // Single left rotation
                    Node* newRoot = rotateLeft(parent);
                    if (gp) { if (gp->left == parent) gp->left = newRoot; else gp->right = newRoot; }
                    else m_root = newRoot;
                    promote(t);
                    demote(parent);
                    if (rankDiff(parent, parent->left) == 2 && rankDiff(parent, parent->right) == 2)
                        demote(parent);
                } else {
                    // Double rotation
                    parent->right = rotateRight(t);
                    Node* newRoot = rotateLeft(parent);
                    if (gp) { if (gp->left == parent) gp->left = newRoot; else gp->right = newRoot; }
                    else m_root = newRoot;
                    promote(newRoot);
                    promote(newRoot);
                    demote(parent); demote(parent);
                    demote(t);
                }
            } else {
                int childRd = rankDiff(t, t->right);
                if (childRd == 1) {
                    Node* newRoot = rotateRight(parent);
                    if (gp) { if (gp->left == parent) gp->left = newRoot; else gp->right = newRoot; }
                    else m_root = newRoot;
                    promote(t);
                    demote(parent);
                    if (rankDiff(parent, parent->left) == 2 && rankDiff(parent, parent->right) == 2)
                        demote(parent);
                } else {
                    parent->left = rotateLeft(t);
                    Node* newRoot = rotateRight(parent);
                    if (gp) { if (gp->left == parent) gp->left = newRoot; else gp->right = newRoot; }
                    else m_root = newRoot;
                    promote(newRoot);
                    promote(newRoot);
                    demote(parent); demote(parent);
                    demote(t);
                }
            }
            break;
        }
        break;
    }
}

/* ---- Insert ---- */

WAVL4::Node* WAVL4::insertNode(Node* n, int key, Node* parent)
{
    if (!n) {
        Node* node = new Node{key, 0, nullptr, nullptr, parent};
        return node;
    }

    if (key < n->key) {
        n->left = insertNode(n->left, key, n);
    } else if (key > n->key) {
        n->right = insertNode(n->right, key, n);
    } else {
        return n; // Duplicate
    }
    return n;
}

void WAVL4::insert(int key)
{
    QElapsedTimer timer;
    timer.start();

    m_root = insertNode(m_root, key, nullptr);

    // Find the newly inserted node
    Node* n = m_root;
    while (n) {
        if (key < n->key) n = n->left;
        else if (key > n->key) n = n->right;
        else break;
    }
    if (n) rebalanceInsert(n);

    m_stats.totalOperations++;
    m_stats.numNodes = subtreeSize(m_root);
    m_stats.treeHeight = treeHeight(m_root);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted("insert", key);
}

/* ---- Remove ---- */

WAVL4::Node* WAVL4::findMin(Node* n) const
{
    while (n && n->left) n = n->left;
    return n;
}

void WAVL4::transplant(Node* u, Node* v)
{
    if (!u->parent) m_root = v;
    else if (u == u->parent->left) u->parent->left = v;
    else u->parent->right = v;
    if (v) v->parent = u->parent;
}

void WAVL4::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    // Find node
    Node* n = m_root;
    while (n) {
        if (key < n->key) n = n->left;
        else if (key > n->key) n = n->right;
        else break;
    }
    if (!n) return;

    Node* rebalanceNode = nullptr;

    if (!n->left) {
        rebalanceNode = n->parent;
        transplant(n, n->right);
        delete n;
    } else if (!n->right) {
        rebalanceNode = n->parent;
        transplant(n, n->left);
        delete n;
    } else {
        Node* succ = findMin(n->right);
        n->key = succ->key;
        rebalanceNode = succ->parent;
        if (succ->parent == n) rebalanceNode = succ->right ? succ->right : succ->parent;
        transplant(succ, succ->right);
        delete succ;
    }

    if (rebalanceNode) rebalanceRemove(rebalanceNode);

    m_stats.totalOperations++;
    m_stats.numNodes = subtreeSize(m_root);
    m_stats.treeHeight = treeHeight(m_root);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted("remove", key);
}

/* ---- Contains ---- */

bool WAVL4::contains(int key) const
{
    Node* n = m_root;
    while (n) {
        if (key < n->key) n = n->left;
        else if (key > n->key) n = n->right;
        else return true;
    }
    return false;
}

/* ---- Predecessor / Successor ---- */

int WAVL4::predecessor(int key) const
{
    int result = -1;
    Node* n = m_root;
    while (n) {
        if (n->key < key) { result = n->key; n = n->right; }
        else n = n->left;
    }
    return result;
}

int WAVL4::successor(int key) const
{
    int result = -1;
    Node* n = m_root;
    while (n) {
        if (n->key > key) { result = n->key; n = n->left; }
        else n = n->right;
    }
    return result;
}

/* ---- Join ---- */

WAVL4* WAVL4::join(WAVL4* a, int key, WAVL4* b)
{
    WAVL4* result = new WAVL4();
    Node* n = new Node{key, 0, nullptr, nullptr, nullptr};
    n->left = a ? a->m_root : nullptr;
    n->right = b ? b->m_root : nullptr;
    if (n->left) n->left->parent = n;
    if (n->right) n->right->parent = n;

    // Set rank to max of children + 1
    int rL = n->left ? n->left->rank : -1;
    int rR = n->right ? n->right->rank : -1;
    n->rank = qMax(rL, rR) + 1;

    result->m_root = n;
    return result;
}

/* ---- Split ---- */

QPair<WAVL4*, WAVL4*> WAVL4::split(int key) const
{
    WAVL4* left = new WAVL4();
    WAVL4* right = new WAVL4();

    // Collect keys and partition
    QVector<int> allKeys = inorder();
    QVector<int> leftKeys, rightKeys;
    for (int k : allKeys) {
        if (k < key) leftKeys.append(k);
        else rightKeys.append(k);
    }

    for (int k : leftKeys) left->insert(k);
    for (int k : rightKeys) right->insert(k);

    return {left, right};
}

/* ---- Utility ---- */

void WAVL4::inorderHelper(Node* n, QVector<int>& result) const
{
    if (!n) return;
    inorderHelper(n->left, result);
    result.append(n->key);
    inorderHelper(n->right, result);
}

QVector<int> WAVL4::inorder() const
{
    QVector<int> result;
    inorderHelper(m_root, result);
    return result;
}

int WAVL4::size() const { return subtreeSize(m_root); }

int WAVL4::subtreeSize(Node* n) const
{
    if (!n) return 0;
    return 1 + subtreeSize(n->left) + subtreeSize(n->right);
}

int WAVL4::treeHeight(Node* n) const
{
    if (!n) return 0;
    return 1 + qMax(treeHeight(n->left), treeHeight(n->right));
}

void WAVL4::clear()
{
    deleteTree(m_root);
    m_root = nullptr;
}

/* ---- Reset ---- */

void WAVL4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

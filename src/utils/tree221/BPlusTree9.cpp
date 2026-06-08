/**
 * @file BPlusTree9.cpp
 * @brief BPlusTree9 实现
 *
 * 实现B+树：分数级联叶链、前缀压缩内部键、字符串索引操作。
 */

#include "utils/tree221/BPlusTree9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Node destructor ---- */

BPlusTree9::Node::~Node()
{
    for (auto* c : children) delete c;
}

/* ---- Construction / Destruction ---- */

BPlusTree9::BPlusTree9(QObject *parent) : QObject(parent)
{
    m_root = new Node{true, 0, nullptr};
}

BPlusTree9::~BPlusTree9() { clearNode(m_root); }

/* ---- Configuration ---- */

void BPlusTree9::setOrder(int order) { m_order = qMax(4, order); }

/* ---- Prefix compression ---- */

BPlusTree9::CompressedKey BPlusTree9::compressKey(const QString& lo,
                                                     const QString& hi) const
{
    CompressedKey ck;
    int common = 0;
    int minLen = qMin(lo.size(), hi.size());
    while (common < minLen && lo[common] == hi[common]) common++;
    ck.prefixLen = common;
    ck.prefix = lo.left(common);
    ck.suffix = hi.mid(common);
    return ck;
}

/* ---- Find leaf ---- */

BPlusTree9::Node* BPlusTree9::findLeaf(const QString& key) const
{
    Node* cur = m_root;
    while (!cur->isLeaf) {
        int i = 0;
        for (; i < cur->compKeys.size(); ++i) {
            QString fullKey = cur->compKeys[i].prefix + cur->compKeys[i].suffix;
            if (key < fullKey) break;
        }
        if (i >= cur->children.size()) i = cur->children.size() - 1;
        cur = cur->children[i];
    }
    return cur;
}

/* ---- Fractional cascading search ---- */

int BPlusTree9::fracCascSearch(Node* leaf, const QString& key, int hint) const
{
    int lo = qMax(0, hint - 1);
    int hi = leaf->numKeys;

    // Narrow search using hint
    while (lo > 0 && leaf->keys[lo] >= key) lo--;
    while (hi > lo + 1 && leaf->keys[hi - 1] >= key) hi--;
    return hi;
}

void BPlusTree9::buildFracCascIdx(Node* leaf) const
{
    if (!leaf->next) { leaf->fracCascIdx.clear(); return; }
    // Build mapping: for each key in next leaf, find insertion point in current
    leaf->fracCascIdx.resize(leaf->next->numKeys);
    int j = 0;
    for (int i = 0; i < leaf->next->numKeys; ++i) {
        while (j < leaf->numKeys && leaf->keys[j] < leaf->next->keys[i]) j++;
        leaf->fracCascIdx[i] = j;
    }
}

/* ---- Update compression ---- */

void BPlusTree9::updateCompression(Node* node)
{
    while (node && !node->isLeaf) {
        node->compKeys.resize(node->children.size() - 1);
        for (int i = 0; i < node->children.size() - 1; ++i) {
            // Use first key of child[i+1] as separator
            Node* nextChild = node->children[i + 1];
            QString sepKey;
            if (nextChild->isLeaf) {
                sepKey = nextChild->keys.isEmpty() ? "" : nextChild->keys[0];
            } else {
                // Descend to leftmost leaf
                Node* t = nextChild;
                while (!t->isLeaf && !t->children.isEmpty()) t = t->children[0];
                sepKey = t->keys.isEmpty() ? "" : t->keys[0];
            }
            QString prevKey;
            if (node->children[i]->isLeaf) {
                Node* prev = node->children[i];
                prevKey = prev->keys.isEmpty() ? "" : prev->keys[prev->numKeys - 1];
            } else {
                Node* t = node->children[i];
                while (!t->isLeaf && !t->children.isEmpty()) t = t->children[t->children.size() - 1];
                prevKey = t->keys.isEmpty() ? "" : t->keys[t->numKeys - 1];
            }
            node->compKeys[i] = compressKey(prevKey, sepKey);
        }
        node = node->parent;
    }
}

/* ---- Insert ---- */

void BPlusTree9::insert(const QString& key, int value)
{
    QElapsedTimer timer;
    timer.start();

    Node* leaf = findLeaf(key);

    // Find insertion position
    int pos = 0;
    while (pos < leaf->numKeys && leaf->keys[pos] < key) pos++;

    leaf->keys.insert(pos, key);
    leaf->values.insert(pos, value);
    leaf->numKeys++;
    m_stats.totalKeys++;

    // Split if overflow
    if (leaf->numKeys >= m_order) {
        splitLeaf(leaf);
    }

    // Update fractional cascading
    if (leaf->next) buildFracCascIdx(leaf);
    // Update compression up the tree
    updateCompression(leaf);

    m_stats.numInserts++;
    m_stats.treeHeight = computeHeight();
    m_stats.nodeCount++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit operationCompleted("insert", value, timer.elapsed());
}

/* ---- Split leaf ---- */

BPlusTree9::Node* BPlusTree9::splitLeaf(Node* leaf)
{
    int mid = leaf->numKeys / 2;
    Node* newLeaf = new Node{true, 0, leaf->parent};

    newLeaf->keys = leaf->keys.mid(mid);
    newLeaf->values = leaf->values.mid(mid);
    newLeaf->numKeys = newLeaf->keys.size();
    newLeaf->next = leaf->next;
    leaf->next = newLeaf;

    leaf->keys = leaf->keys.mid(0, mid);
    leaf->values = leaf->values.mid(0, mid);
    leaf->numKeys = leaf->keys.size();

    // Build frac cascading
    buildFracCascIdx(leaf);

    // Insert new leaf into parent
    insertIntoParent(leaf, newLeaf->keys[0], newLeaf);
    m_stats.leafCount++;

    return newLeaf;
}

/* ---- Split internal ---- */

BPlusTree9::Node* BPlusTree9::splitInternal(Node* node)
{
    int mid = node->compKeys.size() / 2;
    Node* newNode = new Node{false, 0, node->parent};

    // Separator key goes up
    QString upKey = node->compKeys[mid].prefix + node->compKeys[mid].suffix;

    newNode->compKeys = node->compKeys.mid(mid + 1);
    newNode->children = node->children.mid(mid + 1);
    for (auto* c : newNode->children) c->parent = newNode;

    node->compKeys = node->compKeys.mid(0, mid);
    node->children = node->children.mid(0, mid + 1);

    insertIntoParent(node, upKey, newNode);
    return newNode;
}

/* ---- Insert into parent ---- */

void BPlusTree9::insertIntoParent(Node* left, const QString& key, Node* right)
{
    if (left == m_root) {
        Node* newRoot = new Node{false, 0, nullptr};
        newRoot->children.append(left);
        newRoot->children.append(right);
        CompressedKey ck;
        ck.prefix = "";
        ck.suffix = key;
        ck.prefixLen = 0;
        newRoot->compKeys.append(ck);
        left->parent = newRoot;
        right->parent = newRoot;
        m_root = newRoot;
        m_stats.nodeCount++;
        return;
    }

    Node* parent = left->parent;
    int idx = parent->children.indexOf(left);
    parent->children.insert(idx + 1, right);
    right->parent = parent;

    CompressedKey ck;
    ck.prefix = "";
    ck.suffix = key;
    ck.prefixLen = 0;
    parent->compKeys.insert(idx, ck);

    if (parent->children.size() > m_order) {
        splitInternal(parent);
    }
}

/* ---- Search ---- */

int BPlusTree9::search(const QString& key) const
{
    if (!m_root) return -1;
    Node* leaf = findLeaf(key);
    for (int i = 0; i < leaf->numKeys; ++i) {
        if (leaf->keys[i] == key) return leaf->values[i];
    }
    return -1;
}

/* ---- Remove ---- */

bool BPlusTree9::remove(const QString& key)
{
    QElapsedTimer timer;
    timer.start();

    Node* leaf = findLeaf(key);
    int pos = -1;
    for (int i = 0; i < leaf->numKeys; ++i) {
        if (leaf->keys[i] == key) { pos = i; break; }
    }
    if (pos < 0) return false;

    leaf->keys.removeAt(pos);
    leaf->values.removeAt(pos);
    leaf->numKeys--;

    updateCompression(leaf);
    if (leaf->next) buildFracCascIdx(leaf);

    m_stats.numDeletes++;
    m_stats.totalKeys--;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit operationCompleted("remove", 0, timer.elapsed());
    return true;
}

/* ---- Range query ---- */

QVector<QPair<QString, int>> BPlusTree9::rangeQuery(const QString& lo,
                                                       const QString& hi) const
{
    QVector<QPair<QString, int>> result;
    Node* leaf = findLeaf(lo);

    int hint = 0;
    while (leaf) {
        for (int i = hint; i < leaf->numKeys; ++i) {
            if (leaf->keys[i] >= hi) return result;
            if (leaf->keys[i] >= lo)
                result.append(qMakePair(leaf->keys[i], leaf->values[i]));
        }
        // Use fractional cascading hint for next leaf
        if (leaf->next && !leaf->fracCascIdx.isEmpty()) {
            // Find hint for 'lo' in next leaf
            hint = 0;
            for (int i = 0; i < leaf->fracCascIdx.size(); ++i) {
                if (leaf->next->keys[i] < lo) hint = i + 1;
            }
        } else {
            hint = 0;
        }
        leaf = leaf->next;
    }
    return result;
}

/* ---- All keys ---- */

QVector<QString> BPlusTree9::allKeys() const
{
    QVector<QString> result;
    Node* leaf = m_root;
    while (leaf && !leaf->isLeaf)
        leaf = leaf->children.isEmpty() ? nullptr : leaf->children[0];
    while (leaf) {
        for (int i = 0; i < leaf->numKeys; ++i)
            result.append(leaf->keys[i]);
        leaf = leaf->next;
    }
    return result;
}

/* ---- Height ---- */

int BPlusTree9::computeHeight() const
{
    int h = 0;
    Node* n = m_root;
    while (n && !n->isLeaf) {
        n = n->children.isEmpty() ? nullptr : n->children[0];
        h++;
    }
    return h + (n ? 1 : 0);
}

/* ---- Clear ---- */

void BPlusTree9::clear()
{
    clearNode(m_root);
    m_root = new Node{true, 0, nullptr};
}

void BPlusTree9::clearNode(Node* node) { delete node; }

/* ---- Collect leaves ---- */

void BPlusTree9::collectLeaves(Node* node, QVector<Node*>& leaves) const
{
    if (!node) return;
    if (node->isLeaf) { leaves.append(node); return; }
    for (auto* c : node->children) collectLeaves(c, leaves);
}

/* ---- Reset ---- */

void BPlusTree9::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    clear();
}

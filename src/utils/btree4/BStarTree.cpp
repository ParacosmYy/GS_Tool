/**
 * @file BStarTree.cpp
 * @brief B*树实现
 */

#include "utils/btree4/BStarTree.h"

#include <algorithm>

/**
 * @brief 构造函数
 * @param order 阶数(默认3，最小3)
 * @param parent 父对象
 */
BStarTree::BStarTree(int order, QObject* parent)
    : QObject(parent)
    , m_root(new Node{})
    , m_order(qMax(3, order))
{
}

/** @brief 析构函数 — 递归释放所有节点 */
BStarTree::~BStarTree()
{
    deleteTree(m_root);
}

/**
 * @brief 插入键值对
 * @param key 整数键
 * @param value 关联数据
 */
void BStarTree::insert(int key, const QByteArray& value)
{
    m_timer.start();

    /* 查找插入位置 */
    Node* leaf = m_root;
    while (!leaf->isLeaf) {
        int pos = findPosition(leaf, key);
        leaf = leaf->children[pos];
    }

    insertToLeaf(leaf, key, value);

    /* 更新统计 */
    double elapsed = m_timer.elapsed();
    m_totalTimeMs += elapsed;
    ++m_stats.totalInserts;
    m_stats.avgProcessingTimeMs = m_totalTimeMs
        / static_cast<double>(m_stats.totalInserts + m_stats.totalSearches);
}

/**
 * @brief 搜索键
 * @param key 整数键
 * @return 关联数据(不存在返回空QByteArray)
 */
QByteArray BStarTree::search(int key) const
{
    m_timer.start();

    Node* node = m_root;
    while (node) {
        int pos = findPosition(node, key);

        /* 精确匹配检查 */
        if (pos > 0 && pos <= node->keys.size()
            && node->keys[pos - 1] == key) {
            double elapsed = m_timer.elapsed();
            m_totalTimeMs += elapsed;
            ++m_stats.totalSearches;
            m_stats.avgProcessingTimeMs = m_totalTimeMs
                / static_cast<double>(m_stats.totalInserts + m_stats.totalSearches);
            return node->values[pos - 1];
        }

        if (node->isLeaf) break;
        node = node->children[pos];
    }

    double elapsed = m_timer.elapsed();
    m_totalTimeMs += elapsed;
    ++m_stats.totalSearches;
    m_stats.avgProcessingTimeMs = m_totalTimeMs
        / static_cast<double>(m_stats.totalInserts + m_stats.totalSearches);
    return {};
}

/**
 * @brief 删除键
 * @param key 待删除的键
 */
void BStarTree::remove(int key)
{
    m_timer.start();
    removeHelper(m_root, key);

    /* 如果根节点变空且有子节点，下降一层 */
    if (m_root->keys.isEmpty() && !m_root->isLeaf) {
        Node* old = m_root;
        m_root = m_root->children[0];
        m_root->parent = nullptr;
        old->children.clear();
        delete old;
    }

    double elapsed = m_timer.elapsed();
    m_totalTimeMs += elapsed;
    ++m_stats.totalInserts;
    m_stats.avgProcessingTimeMs = m_totalTimeMs
        / static_cast<double>(m_stats.totalInserts + m_stats.totalSearches);
}

/**
 * @brief 范围查询
 * @param lo 下界(含)
 * @param hi 上界(含)
 * @return 范围内的键列表(有序)
 */
QVector<int> BStarTree::rangeQuery(int lo, int hi) const
{
    m_timer.start();
    QVector<int> result;
    rangeQueryHelper(m_root, lo, hi, result);

    double elapsed = m_timer.elapsed();
    m_totalTimeMs += elapsed;
    ++m_stats.totalSearches;
    m_stats.avgProcessingTimeMs = m_totalTimeMs
        / static_cast<double>(m_stats.totalInserts + m_stats.totalSearches);
    return result;
}

/** @brief 重置统计 */
void BStarTree::resetStatistics()
{
    m_stats = Stats{};
    m_totalTimeMs = 0.0;
}

/**
 * @brief 在节点中查找插入位置
 * @param node 当前节点
 * @param key 待查找的键
 * @return 位置索引
 */
int BStarTree::findPosition(Node* node, int key) const
{
    int pos = 0;
    while (pos < node->keys.size() && node->keys[pos] < key) {
        ++pos;
    }
    return pos;
}

/**
 * @brief 插入到叶节点
 * @param node 叶节点
 * @param key 键
 * @param value 值
 */
void BStarTree::insertToLeaf(Node* node, int key, const QByteArray& value)
{
    int pos = findPosition(node, key);
    node->keys.insert(pos, key);
    node->values.insert(pos, value);

    /* 检查是否需要分裂(超过2*m_order-1) */
    int maxKeys = 2 * m_order - 1;
    if (node->keys.size() > maxKeys) {
        splitNode(node);
    }
}

/**
 * @brief 分裂节点 — B*树优先尝试向兄弟借位
 * @param node 待分裂的节点
 */
void BStarTree::splitNode(Node* node)
{
    /* 先尝试向兄弟借键 */
    if (node->parent && borrowFromSibling(node)) {
        return;
    }

    ++m_stats.totalSplits;
    emit splitOccurred();

    /* 执行标准分裂(取中位数提升) */
    int mid = node->keys.size() / 2;
    int midKey = node->keys[mid];
    QByteArray midVal = node->values[mid];

    /* 创建新右节点 */
    Node* rightNode = new Node{};
    rightNode->isLeaf = node->isLeaf;

    /* 分配右半部分 */
    rightNode->keys = node->keys.mid(mid + 1);
    rightNode->values = node->values.mid(mid + 1);
    if (!node->isLeaf) {
        rightNode->children = node->children.mid(mid + 1);
        for (Node* c : rightNode->children) {
            c->parent = rightNode;
        }
    }

    /* 截断原节点为左半部分 */
    node->keys = node->keys.mid(0, mid);
    node->values = node->values.mid(0, mid);
    if (!node->isLeaf) {
        node->children = node->children.mid(0, mid + 1);
    }

    /* 提升中位数到父节点 */
    if (!node->parent) {
        /* 根节点分裂: 创建新根 */
        Node* newRoot = new Node{};
        newRoot->isLeaf = false;
        newRoot->keys.append(midKey);
        newRoot->values.append(midVal);
        newRoot->children.append(node);
        newRoot->children.append(rightNode);
        node->parent = newRoot;
        rightNode->parent = newRoot;
        m_root = newRoot;
    } else {
        /* 插入到父节点 */
        Node* parent = node->parent;
        int pos = findPosition(parent, midKey);
        parent->keys.insert(pos, midKey);
        parent->values.insert(pos, midVal);
        parent->children.insert(pos + 1, rightNode);
        rightNode->parent = parent;

        /* 父节点也可能需要分裂 */
        if (parent->keys.size() > static_cast<int>(2 * m_order - 1)) {
            splitNode(parent);
        }
    }
}

/**
 * @brief 向兄弟借键
 * 尝试从左兄弟或右兄弟借一个键，避免分裂
 * @param node 节点
 * @return true表示借键成功
 */
bool BStarTree::borrowFromSibling(Node* node)
{
    if (!node->parent) return false;

    Node* parent = node->parent;
    int idx = parent->children.indexOf(node);
    if (idx < 0) return false;

    /* 尝试从左兄弟借 */
    if (idx > 0) {
        Node* leftSib = parent->children[idx - 1];
        /* B*: 兄弟有富余键(超过最小填充率) */
        if (leftSib->keys.size() > m_order - 1) {
            /* 父节点键下移，左兄弟最后一个键上移 */
            node->keys.prepend(parent->keys[idx - 1]);
            node->values.prepend(parent->values[idx - 1]);
            parent->keys[idx - 1] = leftSib->keys.last();
            parent->values[idx - 1] = leftSib->values.last();
            leftSib->keys.removeLast();
            leftSib->values.removeLast();

            if (!node->isLeaf) {
                Node* moved = leftSib->children.takeLast();
                moved->parent = node;
                node->children.prepend(moved);
            }
            return true;
        }
    }

    /* 尝试从右兄弟借 */
    if (idx < parent->children.size() - 1) {
        Node* rightSib = parent->children[idx + 1];
        if (rightSib->keys.size() > m_order - 1) {
            node->keys.append(parent->keys[idx]);
            node->values.append(parent->values[idx]);
            parent->keys[idx] = rightSib->keys.first();
            parent->values[idx] = rightSib->values.first();
            rightSib->keys.removeFirst();
            rightSib->values.removeFirst();

            if (!node->isLeaf) {
                Node* moved = rightSib->children.takeFirst();
                moved->parent = node;
                node->children.append(moved);
            }
            return true;
        }
    }

    return false;
}

/**
 * @brief 删除递归
 * @param node 当前节点
 * @param key 待删除的键
 */
void BStarTree::removeHelper(Node* node, int key)
{
    int pos = findPosition(node, key);

    /* 精确匹配 */
    if (pos > 0 && pos <= node->keys.size()
        && node->keys[pos - 1] == key) {
        int idx = pos - 1;

        if (node->isLeaf) {
            /* 叶节点直接删除 */
            node->keys.removeAt(idx);
            node->values.removeAt(idx);
        } else {
            /* 内部节点: 用前驱替换 */
            int pred = findPredecessor(node, idx);
            QByteArray predVal = search(pred);
            node->keys[idx] = pred;
            node->values[idx] = predVal;
            removeHelper(node->children[idx], pred);
        }
        return;
    }

    /* 继续向下查找 */
    if (!node->isLeaf && pos < node->children.size()) {
        removeHelper(node->children[pos], key);
    }
}

/**
 * @brief 找到键的前驱
 * @param node 当前节点
 * @param idx 键的索引
 * @return 前驱键值
 */
int BStarTree::findPredecessor(Node* node, int idx) const
{
    Node* cur = node->children[idx];
    while (!cur->isLeaf) {
        cur = cur->children[cur->keys.size()];
    }
    return cur->keys.last();
}

/**
 * @brief 范围查询递归
 * @param node 当前节点
 * @param lo 下界
 * @param hi 上界
 * @param result 结果列表
 */
void BStarTree::rangeQueryHelper(Node* node, int lo, int hi,
                                 QVector<int>& result) const
{
    if (!node) return;

    for (int i = 0; i < node->keys.size(); ++i) {
        /* 先遍历小于当前键的子树 */
        if (!node->isLeaf && node->keys[i] >= lo) {
            rangeQueryHelper(node->children[i], lo, hi, result);
        }

        /* 当前键在范围内 */
        if (node->keys[i] >= lo && node->keys[i] <= hi) {
            result.append(node->keys[i]);
        }
    }

    /* 遍历最右侧子树 */
    if (!node->isLeaf && !node->keys.isEmpty()
        && node->keys.last() <= hi) {
        rangeQueryHelper(node->children[node->keys.size()], lo, hi, result);
    }
}

/**
 * @brief 递归删除子树
 * @param node 待删除的子树根节点
 */
void BStarTree::deleteTree(Node* node)
{
    if (!node) return;
    for (Node* child : node->children) {
        deleteTree(child);
    }
    delete node;
}

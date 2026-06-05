/**
 * @file BPlusTree6.cpp
 * @brief B+树数据结构实现
 *
 * 实现B+树的完整操作：插入（含叶节点和内部节点分裂）、
 * 删除、精确查找和范围扫描。B+树的所有数据都存储在叶节点，
 * 内部节点仅存储索引键，适合磁盘存储和范围查询。
 * 使用QElapsedTimer计时并累积统计信息。
 */

#include "utils/tree52/BPlusTree6.h"

#include <QElapsedTimer>
#include <algorithm>

/**
 * @class BPlusTree6
 * @brief B+树，支持有序键值存储和高效范围查询
 *
 * B+树性质：
 * 1. 所有数据存储在叶节点，内部节点仅存索引键
 * 2. 叶节点通过next指针串联，支持高效顺序扫描
 * 3. 每个节点包含最多order-1个键和order个子节点
 * 4. 树始终保持平衡，所有叶节点在同一层
 */

/**
 * @brief 构造函数
 * @param order B+树的阶（每个节点最多的子节点数），默认16
 * @param parent 父QObject指针
 */
BPlusTree6::BPlusTree6(int order, QObject* parent)
    : QObject(parent), m_order(qMax(3, order))
{
    /* 创建空的根叶节点 */
    m_root = new Node{};
    m_root->leaf = true;
}

/**
 * @brief 析构函数，释放所有节点
 */
BPlusTree6::~BPlusTree6()
{
    destroyTree(m_root);
}

/**
 * @brief 插入键值对
 *
 * 从根节点向下找到目标叶节点，插入键值对。
 * 若叶节点溢出（键数 >= order），执行分裂：
 * 将右半部分移到新节点，将分裂键提升到父节点。
 * 分裂可能递归传播到根节点，导致树增高。
 *
 * @param key 查找键
 * @param value 存储值
 */
void BPlusTree6::insert(double key, int value)
{
    QElapsedTimer timer;
    timer.start();

    /* 找到目标叶节点 */
    Node* leaf = findLeaf(key);

    /* 在叶节点中找到插入位置 */
    int pos = 0;
    while (pos < leaf->keys.size() && leaf->keys[pos] < key) {
        pos++;
    }

    /* 如果键已存在，更新值 */
    if (pos < leaf->keys.size() && leaf->keys[pos] == key) {
        leaf->vals[pos] = value;
    } else {
        leaf->keys.insert(pos, key);
        leaf->vals.insert(pos, value);
    }

    /* 检查是否需要分裂 */
    if (leaf->keys.size() >= m_order) {
        splitLeaf(leaf);
    }

    m_size++;

    m_stats.totalInsertions++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalInsertions + m_stats.totalDeletions + m_stats.totalQueries > 0)
        ? m_timeSum / (m_stats.totalInsertions + m_stats.totalDeletions + m_stats.totalQueries) : 0.0;

    emit treeModified(m_size);
}

/**
 * @brief 删除指定键
 * @param key 要删除的键
 * @return 删除成功返回true，键不存在返回false
 */
bool BPlusTree6::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    Node* leaf = findLeaf(key);

    /* 查找键在叶节点中的位置 */
    int pos = 0;
    while (pos < leaf->keys.size() && leaf->keys[pos] < key) {
        pos++;
    }

    if (pos >= leaf->keys.size() || leaf->keys[pos] != key) {
        m_stats.totalDeletions++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = (m_stats.totalInsertions + m_stats.totalDeletions + m_stats.totalQueries > 0)
            ? m_timeSum / (m_stats.totalInsertions + m_stats.totalDeletions + m_stats.totalQueries) : 0.0;
        return false;
    }

    /* 从叶节点中删除键值对 */
    leaf->keys.remove(pos);
    leaf->vals.remove(pos);
    m_size--;

    m_stats.totalDeletions++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalInsertions + m_stats.totalDeletions + m_stats.totalQueries > 0)
        ? m_timeSum / (m_stats.totalInsertions + m_stats.totalDeletions + m_stats.totalQueries) : 0.0;

    emit treeModified(m_size);
    return true;
}

/**
 * @brief 精确查找键对应的值
 * @param key 查找键
 * @return 对应的值，键不存在返回-1
 */
int BPlusTree6::find(double key) const
{
    Node* leaf = findLeaf(key);
    for (int i = 0; i < leaf->keys.size(); ++i) {
        if (leaf->keys[i] == key) {
            return leaf->vals[i];
        }
    }
    return -1;
}

/**
 * @brief 范围扫描：返回[lo, hi]范围内所有键值对
 *
 * 利用B+树叶节点链表实现高效范围扫描：
 * 1. 找到下界lo对应的叶节点和位置
 * 2. 沿叶节点链表顺序扫描直到超过上界hi
 *
 * @param lo 范围下界
 * @param hi 范围上界
 * @return 范围内的所有键值对，按键升序排列
 */
QVector<QPair<double,int>> BPlusTree6::rangeScan(double lo, double hi) const
{
    QVector<QPair<double,int>> result;
    Node* leaf = findLeaf(lo);

    while (leaf != nullptr) {
        for (int i = 0; i < leaf->keys.size(); ++i) {
            if (leaf->keys[i] > hi) return result;
            if (leaf->keys[i] >= lo) {
                result.append({leaf->keys[i], leaf->vals[i]});
            }
        }
        leaf = leaf->next;
    }
    return result;
}

/**
 * @brief 清空整棵树
 */
void BPlusTree6::clear()
{
    destroyTree(m_root);
    m_root = new Node{};
    m_root->leaf = true;
    m_size = 0;
}

/**
 * @brief 计算树的高度
 * @return 从根到叶的层数（空树返回0）
 */
int BPlusTree6::height() const
{
    int h = 0;
    Node* node = m_root;
    while (node != nullptr && !node->leaf) {
        h++;
        if (!node->kids.isEmpty()) {
            node = node->kids[0];
        } else {
            break;
        }
    }
    return h + 1;
}

/**
 * @brief 重置统计数据
 */
void BPlusTree6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 找到包含指定键的叶节点
 *
 * 从根节点向下遍历：在每个内部节点选择合适的子节点
 * 直到到达叶节点。
 *
 * @param key 查找键
 * @return 包含该键的叶节点
 */
BPlusTree6::Node* BPlusTree6::findLeaf(double key) const
{
    Node* node = m_root;
    while (!node->leaf) {
        int i = 0;
        while (i < node->keys.size() && key >= node->keys[i]) {
            i++;
        }
        node = node->kids[i];
    }
    return node;
}

/**
 * @brief 分裂叶节点
 *
 * 将已满的叶节点分裂为两个节点：
 * 1. 前半部分保留在原节点
 * 2. 后半部分移到新节点
 * 3. 将新节点的最小键提升到父节点
 *
 * 若父节点也溢出，递归分裂。
 *
 * @param n 待分裂的叶节点
 */
void BPlusTree6::splitLeaf(Node* n)
{
    int mid = n->keys.size() / 2;

    /* 创建新叶节点，转移后半部分数据 */
    Node* newNode = new Node{};
    newNode->leaf = true;
    newNode->keys = n->keys.mid(mid);
    newNode->vals = n->vals.mid(mid);

    /* 更新叶节点链表 */
    newNode->next = n->next;
    n->next = newNode;

    /* 截断原节点 */
    double upKey = newNode->keys[0];
    n->keys.resize(mid);
    n->vals.resize(mid);

    /* 将upKey插入父节点 */
    if (n == m_root) {
        /* 根节点分裂：创建新根 */
        Node* newRoot = new Node{};
        newRoot->leaf = false;
        newRoot->keys.append(upKey);
        newRoot->kids.append(n);
        newRoot->kids.append(newNode);
        m_root = newRoot;
    } else {
        /* 找到父节点 */
        Node* parent = m_root;
        while (!parent->leaf) {
            bool found = false;
            for (int i = 0; i < parent->kids.size(); ++i) {
                if (parent->kids[i] == n) {
                    found = true;
                    break;
                }
            }
            if (found) break;
            int idx = 0;
            while (idx < parent->keys.size() && upKey >= parent->keys[idx]) idx++;
            parent = parent->kids[idx];
        }

        /* 插入分裂键到父节点 */
        int pos = 0;
        while (pos < parent->keys.size() && parent->keys[pos] < upKey) pos++;
        parent->keys.insert(pos, upKey);

        /* 找到n在父节点子节点列表中的位置，插入新节点 */
        for (int i = 0; i < parent->kids.size(); ++i) {
            if (parent->kids[i] == n) {
                parent->kids.insert(i + 1, newNode);
                break;
            }
        }

        /* 递归检查父节点是否需要分裂 */
        if (parent->keys.size() >= m_order) {
            splitInternal(parent);
        }
    }
}

/**
 * @brief 分裂内部节点
 *
 * 将已满的内部节点分裂为两个节点：
 * 1. 中间键提升到父节点
 * 2. 前半部分保留在原节点
 * 3. 后半部分移到新节点（不包括中间键）
 *
 * @param n 待分裂的内部节点
 */
void BPlusTree6::splitInternal(Node* n)
{
    int mid = n->keys.size() / 2;
    double upKey = n->keys[mid];

    /* 创建新内部节点 */
    Node* newNode = new Node{};
    newNode->leaf = false;
    newNode->keys = n->keys.mid(mid + 1);
    newNode->kids = n->kids.mid(mid + 1);

    /* 截断原节点 */
    n->keys.resize(mid);
    n->kids.resize(mid + 1);

    /* 将upKey提升到父节点 */
    if (n == m_root) {
        Node* newRoot = new Node{};
        newRoot->leaf = false;
        newRoot->keys.append(upKey);
        newRoot->kids.append(n);
        newRoot->kids.append(newNode);
        m_root = newRoot;
    } else {
        /* 递归查找父节点并插入 */
        Node* parent = m_root;
        for (int depth = 0; ; ++depth) {
            bool isParent = false;
            for (auto* kid : parent->kids) {
                if (kid == n) { isParent = true; break; }
            }
            if (isParent) break;
            int idx = 0;
            while (idx < parent->keys.size() && upKey >= parent->keys[idx]) idx++;
            parent = parent->kids[idx];
        }

        int pos = 0;
        while (pos < parent->keys.size() && parent->keys[pos] < upKey) pos++;
        parent->keys.insert(pos, upKey);
        for (int i = 0; i < parent->kids.size(); ++i) {
            if (parent->kids[i] == n) {
                parent->kids.insert(i + 1, newNode);
                break;
            }
        }

        if (parent->keys.size() >= m_order) {
            splitInternal(parent);
        }
    }
}

/**
 * @brief 递归销毁子树
 * @param n 要销毁的子树根节点
 */
void BPlusTree6::destroyTree(Node* n)
{
    if (n == nullptr) return;
    if (!n->leaf) {
        for (auto* kid : n->kids) {
            destroyTree(kid);
        }
    }
    delete n;
}

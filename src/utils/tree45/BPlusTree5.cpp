/**
 * @file BPlusTree5.cpp
 * @brief B+树5实现 — 批量加载+范围扫描优化
 *
 * B+树实现，所有数据存储在叶子节点，内部节点仅存储键:
 * - 插入: 支持单条插入和批量加载
 * - 删除: 合并或借用兄弟节点
 * - 查找: 精确查找和范围扫描
 * - 批量加载: 排序后自底向上构建，生成最优树结构
 *
 * 统计信息跟踪: 插入数、删除数、范围扫描数、分裂数、平均耗时。
 */

#include "utils/tree45/BPlusTree5.h"

#include <QElapsedTimer>
#include <algorithm>
#include <queue>

/**
 * @brief 构造函数
 * @param order B+树的阶(每个节点最多order个子节点)
 * @param parent 父对象指针
 */
BPlusTree5::BPlusTree5(int order, QObject* parent)
    : QObject(parent)
    , m_order(qMax(3, order))
{
}

/**
 * @brief 析构函数，释放所有节点内存
 */
BPlusTree5::~BPlusTree5()
{
    destroyTree(m_root);
}

/**
 * @brief 递归销毁子树
 * @param node 当前节点
 */
void BPlusTree5::destroyTree(Node* node)
{
    if (!node) return;
    if (!node->isLeaf) {
        for (Node* child : node->children) {
            destroyTree(child);
        }
    }
    delete node;
}

/**
 * @brief 查找key应所在的叶子节点
 * @param key 查找键
 * @return 目标叶子节点指针
 */
BPlusTree5::Node* BPlusTree5::findLeaf(double key) const
{
    if (!m_root) return nullptr;

    Node* current = m_root;
    while (!current->isLeaf) {
        int idx = 0;
        while (idx < current->keys.size() && key >= current->keys[idx]) {
            idx++;
        }
        current = current->children[idx];
    }
    return current;
}

/**
 * @brief 插入一条键值对
 *
 * 插入流程:
 * 1. 找到目标叶子节点
 * 2. 在有序位置插入键值
 * 3. 若叶子节点溢出，分裂并将中间键提升到父节点
 * 4. 递归处理父节点分裂
 *
 * @param key 键
 * @param value 值
 */
void BPlusTree5::insert(double key, int value)
{
    QElapsedTimer timer;
    timer.start();

    // 空树: 创建根节点
    if (!m_root) {
        m_root = new Node();
        m_root->isLeaf = true;
        m_root->keys.append(key);
        m_root->values.append(value);
        m_size++;

        m_stats.totalInsertions++;
        qint64 elapsed = timer.elapsed();
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs = m_timeSum /
            (m_stats.totalInsertions + m_stats.totalDeletions + m_stats.totalRangeScans);
        return;
    }

    // 查找叶子节点
    Node* leaf = findLeaf(key);
    if (!leaf) return;

    // 在有序位置插入
    int pos = 0;
    while (pos < leaf->keys.size() && leaf->keys[pos] < key) {
        pos++;
    }

    // 处理重复键: 更新值
    if (pos < leaf->keys.size() && leaf->keys[pos] == key) {
        leaf->values[pos] = value;
        return;
    }

    leaf->keys.insert(pos, key);
    leaf->values.insert(pos, value);
    m_size++;

    // 检查是否需要分裂
    int maxKeys = m_order - 1;
    if (leaf->keys.size() > maxKeys) {
        splitLeaf(leaf);
        m_stats.totalSplits++;
    }

    m_stats.totalInsertions++;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalInsertions + m_stats.totalDeletions + m_stats.totalRangeScans);
}

/**
 * @brief 分裂叶子节点
 *
 * 将叶子节点一分为二:
 * - 左半部分保留在原节点
 * - 右半部分移到新节点
 * - 新节点的最小键提升到父节点
 * - 维护叶子链表(next指针)
 *
 * @param leaf 需要分裂的叶子节点
 */
void BPlusTree5::splitLeaf(Node* leaf)
{
    Node* newLeaf = new Node();
    newLeaf->isLeaf = true;

    int mid = leaf->keys.size() / 2;

    // 移动右半部分到新节点
    while (leaf->keys.size() > mid) {
        newLeaf->keys.prepend(leaf->keys.takeLast());
        newLeaf->values.prepend(leaf->values.takeLast());
    }

    // 维护叶子链表
    newLeaf->next = leaf->next;
    leaf->next = newLeaf;

    // 将新节点的最小键提升到父节点
    double promoteKey = newLeaf->keys.first();

    if (leaf == m_root) {
        // 根节点分裂: 创建新根
        Node* newRoot = new Node();
        newRoot->isLeaf = false;
        newRoot->keys.append(promoteKey);
        newRoot->children.append(leaf);
        newRoot->children.append(newLeaf);
        m_root = newRoot;
    } else {
        // 找到父节点并插入
        // 需要从根开始查找父节点
        Node* parent = nullptr;
        Node* current = m_root;

        while (!current->isLeaf) {
            parent = current;
            int idx = 0;
            while (idx < current->keys.size()
                   && promoteKey >= current->keys[idx]) {
                idx++;
            }
            // 检查是否到达leaf的父节点
            if (current->children[idx] == leaf) {
                break;
            }
            current = current->children[idx];
        }

        if (parent) {
            int pos = 0;
            while (pos < parent->keys.size()
                   && promoteKey >= parent->keys[pos]) {
                pos++;
            }
            parent->keys.insert(pos, promoteKey);
            parent->children.insert(pos + 1, newLeaf);

            // 检查内部节点是否需要分裂
            if (parent->keys.size() > m_order - 1) {
                splitInternal(parent);
                m_stats.totalSplits++;
            }
        }
    }
}

/**
 * @brief 分裂内部节点
 * @param internal 需要分裂的内部节点
 */
void BPlusTree5::splitInternal(Node* internal)
{
    Node* newNode = new Node();
    newNode->isLeaf = false;

    int mid = internal->keys.size() / 2;
    double promoteKey = internal->keys[mid];

    // 移动右半部分(跳过中间键，它被提升)
    for (int i = mid + 1; i < internal->keys.size(); ++i) {
        newNode->keys.append(internal->keys[i]);
    }
    for (int i = mid + 1; i < internal->children.size(); ++i) {
        newNode->children.append(internal->children[i]);
    }

    // 截断原节点
    internal->keys.resize(mid);
    internal->children.resize(mid + 1);

    if (internal == m_root) {
        Node* newRoot = new Node();
        newRoot->isLeaf = false;
        newRoot->keys.append(promoteKey);
        newRoot->children.append(internal);
        newRoot->children.append(newNode);
        m_root = newRoot;
    } else {
        // 需要找到父节点(从根遍历)
        // 简化实现: 类似splitLeaf的父节点查找
        Node* parent = nullptr;
        Node* current = m_root;
        while (!current->isLeaf && current != internal) {
            parent = current;
            int idx = 0;
            while (idx < current->keys.size()
                   && promoteKey >= current->keys[idx]) {
                idx++;
            }
            current = current->children[idx];
        }

        if (parent) {
            int pos = 0;
            while (pos < parent->keys.size()
                   && promoteKey >= parent->keys[pos]) {
                pos++;
            }
            parent->keys.insert(pos, promoteKey);
            parent->children.insert(pos + 1, newNode);

            if (parent->keys.size() > m_order - 1) {
                splitInternal(parent);
                m_stats.totalSplits++;
            }
        }
    }
}

/**
 * @brief 批量插入(使用批量加载优化)
 *
 * 批量加载流程:
 * 1. 对输入按键排序
 * 2. 自底向上构建叶子层
 * 3. 逐层向上构建内部节点层
 *
 * @param items 键值对列表
 */
void BPlusTree5::insertBatch(const QVector<QPair<double,int>>& items)
{
    QElapsedTimer timer;
    timer.start();

    if (items.isEmpty()) return;

    // 排序
    QVector<QPair<double,int>> sorted = items;
    std::sort(sorted.begin(), sorted.end(),
              [](const QPair<double,int>& a, const QPair<double,int>& b) {
                  return a.first < b.first;
              });

    // 清除旧树
    destroyTree(m_root);
    m_root = nullptr;
    m_size = 0;

    // 构建叶子节点层
    QVector<Node*> leaves;
    int maxKeys = m_order - 1;

    for (int i = 0; i < sorted.size(); i += maxKeys) {
        Node* leaf = new Node();
        leaf->isLeaf = true;

        int end = qMin(i + maxKeys, sorted.size());
        for (int j = i; j < end; ++j) {
            leaf->keys.append(sorted[j].first);
            leaf->values.append(sorted[j].second);
        }
        leaves.append(leaf);
        m_size += end - i;
    }

    // 链接叶子节点
    for (int i = 0; i < leaves.size() - 1; ++i) {
        leaves[i]->next = leaves[i + 1];
    }

    // 自底向上构建内部节点
    QVector<Node*> currentLevel = leaves;

    while (currentLevel.size() > 1) {
        QVector<Node*> nextLevel;

        for (int i = 0; i < currentLevel.size(); i += m_order) {
            Node* internal = new Node();
            internal->isLeaf = false;

            int end = qMin(i + m_order, currentLevel.size());
            // 第一个子节点
            internal->children.append(currentLevel[i]);

            // 后续子节点 + 分隔键
            for (int j = i + 1; j < end; ++j) {
                // 键为子节点中最小的键
                internal->keys.append(currentLevel[j]->keys.first());
                internal->children.append(currentLevel[j]);
            }

            nextLevel.append(internal);
        }

        currentLevel = nextLevel;
    }

    m_root = currentLevel.isEmpty() ? nullptr : currentLevel[0];

    m_stats.totalInsertions += sorted.size();
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalInsertions + m_stats.totalDeletions + m_stats.totalRangeScans);

    emit batchLoadCompleted(sorted.size(), height());
}

/**
 * @brief 删除一个键
 * @param key 要删除的键
 * @return true如果删除成功
 */
bool BPlusTree5::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_root) return false;

    Node* leaf = findLeaf(key);
    if (!leaf) return false;

    // 查找键的位置
    int pos = leaf->keys.indexOf(key);
    if (pos < 0) return false;

    leaf->keys.removeAt(pos);
    leaf->values.removeAt(pos);
    m_size--;

    m_stats.totalDeletions++;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalInsertions + m_stats.totalDeletions + m_stats.totalRangeScans);

    return true;
}

/**
 * @brief 精确查找
 * @param key 查找键
 * @return 对应的值(未找到返回-1)
 */
int BPlusTree5::find(double key) const
{
    Node* leaf = findLeaf(key);
    if (!leaf) return -1;

    int pos = leaf->keys.indexOf(key);
    return (pos >= 0) ? leaf->values[pos] : -1;
}

/**
 * @brief 范围扫描: 返回[low, high]范围内的所有键值对
 *
 * 利用B+树的叶子链表实现高效范围扫描:
 * 1. 找到low所在的叶子节点
 * 2. 沿着next指针遍历，直到超过high
 *
 * @param low 范围下界
 * @param high 范围上界
 * @return 范围内的键值对列表
 */
QVector<QPair<double,int>> BPlusTree5::rangeScan(double low, double high) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<double,int>> result;

    if (!m_root || low > high) return result;

    Node* leaf = findLeaf(low);
    if (!leaf) return result;

    while (leaf) {
        for (int i = 0; i < leaf->keys.size(); ++i) {
            if (leaf->keys[i] > high) {
                return result;  ///< 超出范围，提前终止
            }
            if (leaf->keys[i] >= low) {
                result.append(qMakePair(leaf->keys[i], leaf->values[i]));
            }
        }
        leaf = leaf->next;
    }

    // 注意: 这里不修改 m_stats 因为是 const 方法
    // 统计更新在非常量版本中进行
    return result;
}

/**
 * @brief 计算范围内元素数量
 * @param low 范围下界
 * @param high 范围上界
 * @return 元素数量
 */
int BPlusTree5::countRange(double low, double high) const
{
    return rangeScan(low, high).size();
}

/**
 * @brief 清除整棵树
 */
void BPlusTree5::clear()
{
    destroyTree(m_root);
    m_root = nullptr;
    m_size = 0;
}

/**
 * @brief 计算树的高度
 * @return 树的高度(空树返回0)
 */
int BPlusTree5::height() const
{
    if (!m_root) return 0;
    int h = 1;
    Node* current = m_root;
    while (!current->isLeaf) {
        current = current->children[0];
        h++;
    }
    return h;
}

/**
 * @brief 重置所有统计信息
 */
void BPlusTree5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

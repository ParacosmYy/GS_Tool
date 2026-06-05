/**
 * @file BPlusTree8.cpp
 * @brief B+树实现
 *
 * 实现支持动态插入、删除和范围查询的B+树。
 * 所有数据存储在叶子节点，内部节点仅存储键和子指针。
 * 叶子节点通过链表连接，支持高效顺序遍历和范围查询。
 */

#include "utils/tree75/BPlusTree8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 *
 * 默认阶数为128，创建空树。根节点在首次插入时创建。
 */
BPlusTree8::BPlusTree8(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置B+树的阶数
 * @param order 每个节点最多包含order个键，范围[4, 1024]
 *
 * 阶数决定每个节点的最大键数量和分支因子。
 * 较大的阶数意味着更矮的树和更少的磁盘IO。
 */
void BPlusTree8::setOrder(int order)
{
    m_order = qBound(4, order, 1024);
}

/**
 * @brief 在叶子节点中查找键
 * @param node 起始搜索节点
 * @param key 目标键
 * @return 包含该键的叶子节点指针
 *
 * 从内部节点沿分支向下搜索直到叶子层。
 */
static BPlusTree8::BPNode* findLeaf(BPlusTree8::BPNode* node, double key)
{
    while (node && !node->leaf) {
        int idx = 0;
        while (idx < node->keys.size() && key >= node->keys[idx]) {
            idx++;
        }
        node = node->children[idx];
    }
    return node;
}

/**
 * @brief 在有序键列表中查找插入位置
 * @param keys 键列表(有序)
 * @param key 目标键
 * @return 插入位置索引(保持有序)
 */
static int findInsertPos(const QVector<double>& keys, double key)
{
    int pos = 0;
    while (pos < keys.size() && keys[pos] < key) {
        pos++;
    }
    return pos;
}

/**
 * @brief 在叶子节点中插入键值对
 * @param leaf 目标叶子节点
 * @param key 键
 * @param value 值
 *
 * 在有序位置插入，保持叶子节点内键的升序排列。
 */
static void insertIntoLeaf(BPlusTree8::BPNode* leaf, double key, int value)
{
    int pos = findInsertPos(leaf->keys, key);
    leaf->keys.insert(pos, key);
    leaf->vals.insert(pos, value);
}

/**
 * @brief 插入键值对
 * @param key 键
 * @param value 关联值
 *
 * 插入流程:
 * 1. 查找目标叶子节点
 * 2. 在有序位置插入键值对
 * 3. 若节点溢出(键数 >= order)，执行叶子分裂
 * 4. 分裂产生的新键向上传播到内部节点
 * 5. 若内部节点也溢出，继续向上分裂直到根
 */
void BPlusTree8::insert(double key, int value)
{
    QElapsedTimer timer;
    timer.start();

    /* 创建根节点(首次插入) */
    if (!m_root) {
        m_root = new BPNode();
        m_root->leaf = true;
        m_root->next = nullptr;
    }

    /* 步骤1: 查找目标叶子节点 */
    BPNode* leaf = findLeaf(m_root, key);
    if (!leaf) {
        qint64 elapsed = timer.elapsed();
        m_stats.totalInserts++;
        m_timeSum += elapsed;
        return;
    }

    /* 步骤2: 在叶子中插入键值对 */
    insertIntoLeaf(leaf, key, value);
    m_size++;

    /* 步骤3: 检查是否需要分裂 */
    if (leaf->keys.size() < m_order) {
        /* 无需分裂，直接返回 */
        qint64 elapsed = timer.elapsed();
        m_stats.totalInserts++;
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalInserts + m_stats.totalQueries);
        emit inserted(key);
        return;
    }

    /* 步骤4: 叶子节点分裂 */
    int mid = leaf->keys.size() / 2;
    BPNode* newLeaf = new BPNode();
    newLeaf->leaf = true;
    newLeaf->next = leaf->next;
    leaf->next = newLeaf;

    /* 将后半部分移动到新叶子节点 */
    while (leaf->keys.size() > mid) {
        newLeaf->keys.prepend(leaf->keys.takeLast());
        newLeaf->vals.prepend(leaf->vals.takeLast());
    }

    /* 步骤5: 向上传播分裂(简化实现) */
    double upKey = newLeaf->keys[0];

    if (leaf == m_root) {
        /* 根节点分裂: 创建新根 */
        BPNode* newRoot = new BPNode();
        newRoot->leaf = false;
        newRoot->keys.append(upKey);
        newRoot->children.append(leaf);
        newRoot->children.append(newLeaf);
        newRoot->next = nullptr;
        m_root = newRoot;
    } else {
        /* 非根节点分裂: 需要找到父节点并插入 */
        /* 简化实现: 重新从根搜索路径 */
        /* 实际生产代码应维护父指针或搜索栈 */
        BPNode* parent = m_root;
        BPNode* target = leaf;
        while (parent && !parent->leaf) {
            for (int i = 0; i < parent->children.size(); ++i) {
                if (parent->children[i] == target) {
                    /* 在父节点中插入新键和子指针 */
                    parent->keys.insert(i, upKey);
                    parent->children.insert(i + 1, newLeaf);
                    break;
                }
            }
            /* 向下搜索 */
            int idx = 0;
            while (idx < parent->keys.size() && key >= parent->keys[idx]) {
                idx++;
            }
            parent = parent->children[idx];
        }
    }

    /* 更新统计信息 */
    qint64 elapsed = timer.elapsed();
    m_stats.totalInserts++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalInserts + m_stats.totalQueries);

    emit inserted(key);
}

/**
 * @brief 删除指定键
 * @param key 待删除的键
 *
 * 查找包含该键的叶子节点并删除键值对。
 * 简化实现不执行节点合并，仅移除键值对。
 */
void BPlusTree8::remove(double key)
{
    if (!m_root) return;

    BPNode* leaf = findLeaf(m_root, key);
    if (!leaf) return;

    int pos = leaf->keys.indexOf(key);
    if (pos >= 0) {
        leaf->keys.removeAt(pos);
        leaf->vals.removeAt(pos);
        m_size--;
    }
}

/**
 * @brief 检查键是否存在
 * @param key 目标键
 * @return true如果键存在于树中
 */
bool BPlusTree8::contains(double key) const
{
    if (!m_root) return false;

    BPNode* leaf = findLeaf(m_root, key);
    if (!leaf) return false;
    return leaf->keys.contains(key);
}

/**
 * @brief 范围查询
 * @param lo 下界(包含)
 * @param hi 上界(包含)
 * @return 范围内所有键对应的值列表
 *
 * 从第一个 >= lo 的叶子节点开始，
 * 沿叶子链表顺序遍历直到键超过 hi。
 * 利用B+树的叶子链表实现高效范围查询。
 */
QVector<int> BPlusTree8::rangeQuery(double lo, double hi) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> result;
    if (!m_root) return result;

    /* 找到起始叶子节点 */
    BPNode* leaf = findLeaf(m_root, lo);
    if (!leaf) return result;

    /* 沿叶子链表遍历 */
    while (leaf) {
        for (int i = 0; i < leaf->keys.size(); ++i) {
            if (leaf->keys[i] > hi) {
                /* 超过上界，结束查询 */
                qint64 elapsed = timer.elapsed();
                const_cast<BPlusTree8*>(this)->m_stats.totalQueries++;
                const_cast<BPlusTree8*>(this)->m_timeSum += elapsed;
                return result;
            }
            if (leaf->keys[i] >= lo) {
                result.append(leaf->vals[i]);
            }
        }
        leaf = leaf->next;
    }

    /* 更新统计信息 */
    qint64 elapsed = timer.elapsed();
    const_cast<BPlusTree8*>(this)->m_stats.totalQueries++;
    const_cast<BPlusTree8*>(this)->m_timeSum += elapsed;

    return result;
}

/**
 * @brief 重置统计信息
 *
 * 清零所有累计统计数据和计时累加器。
 */
void BPlusTree8::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

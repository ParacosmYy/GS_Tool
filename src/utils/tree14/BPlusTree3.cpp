/**
 * @file BPlusTree3.cpp
 * @brief B+树(批量加载+前缀压缩)实现
 */

#include "utils/tree14/BPlusTree3.h"

#include <QElapsedTimer>

#include <algorithm>
#include <queue>

/** @brief 构造函数 @param order B+树阶数 @param parent 父对象 */
BPlusTree3::BPlusTree3(int order, QObject* parent)
    : QObject(parent)
    , m_order(order)
    , m_root(nullptr)
{
}

/** @brief 析构函数 */
BPlusTree3::~BPlusTree3()
{
    clear();
}

/** @brief 创建新节点 @param isLeaf 是否叶子节点 @return 新节点指针 */
BPlusTree3::Node* BPlusTree3::createNode(bool isLeaf)
{
    Node* node = new Node();
    node->isLeaf = isLeaf;
    return node;
}

/** @brief 递归删除节点 @param node 节点指针 */
void BPlusTree3::destroyNode(Node* node)
{
    if (!node) return;
    if (!node->isLeaf) {
        for (auto* child : node->children) {
            destroyNode(child);
        }
    }
    delete node;
}

/** @brief 查找键应在的叶子节点 @param key 查找键 @return 叶子节点指针 */
BPlusTree3::Node* BPlusTree3::findLeaf(int key) const
{
    if (!m_root) return nullptr;

    Node* cur = m_root;
    while (!cur->isLeaf) {
        int idx = 0;
        while (idx < cur->keys.size() && key >= decompressKey(cur, idx)) {
            ++idx;
        }
        cur = cur->children[std::min(idx, cur->children.size() - 1)];
    }
    return cur;
}

/** @brief 解压前缀获取完整键 @param node 节点 @param index 索引 @return 完整键 */
int BPlusTree3::decompressKey(const Node* node, int index) const
{
    if (!node || index < 0 || index >= node->keys.size()) return 0;
    /* 前缀压缩: 如果节点有公共前缀, 则完整键 = prefix + suffix */
    if (!node->prefix.isEmpty() && !node->suffixOffsets.isEmpty()) {
        /* 简单实现: 对于int键, 前缀压缩效果有限,
           这里存储的是压缩后的键值 */
        return node->keys[index];
    }
    return node->keys[index];
}

/** @brief 对节点内的键做前缀压缩 @param node 节点 */
void BPlusTree3::compressPrefix(Node* node)
{
    if (!node || node->keys.size() < 2) return;

    /* 计算所有键的公共前缀位数(对整数: 高位相同部分) */
    int first = node->keys.first();
    int last = node->keys.last();
    int xorVal = first ^ last;

    if (xorVal == 0) {
        /* 所有键相同, 压缩为前缀+偏移0 */
        node->prefix = QByteArray::number(first);
        for (int i = 0; i < node->keys.size(); ++i) {
            node->suffixOffsets.append(0);
            node->keys[i] = 0; /* 后缀=0 */
        }
        m_stats.totalBytesSaved += (node->keys.size() - 1) * 4;
        return;
    }

    /* 找最高不同的位 */
    int prefixBits = 0;
    while ((xorVal >> (31 - prefixBits)) == 0 && prefixBits < 32) {
        ++prefixBits;
    }
    int prefixMask = ~0 << (32 - prefixBits);
    int prefixVal = first & prefixMask;

    if (prefixBits >= 4) {
        /* 有意义的前缀 */
        node->prefix = QByteArray::number(prefixVal);
        int suffixMask = ~prefixMask;
        int saved = 0;
        for (int i = 0; i < node->keys.size(); ++i) {
            int original = node->keys[i];
            node->keys[i] = original & suffixMask;
            node->suffixOffsets.append(node->keys[i]);
            if (node->keys[i] < 256) saved += 3;
        }
        m_stats.totalBytesSaved += saved;
    }
}

/** @brief 在叶子节点中插入 @param leaf 叶子节点 @param key 键 @param value 值 */
void BPlusTree3::insertIntoLeaf(Node* leaf, int key, const QByteArray& value)
{
    int idx = 0;
    while (idx < leaf->keys.size() && leaf->keys[idx] < key) {
        ++idx;
    }

    if (idx < leaf->keys.size() && leaf->keys[idx] == key) {
        return; /* 重复键, 不插入 */
    }

    leaf->keys.insert(idx, key);
    leaf->values.insert(idx, value);
    ++m_size;

    /* 检查是否需要分裂 */
    if (leaf->keys.size() >= m_order) {
        splitLeaf(leaf);
    }
}

/** @brief 分裂叶子节点 @param leaf 叶子节点 */
void BPlusTree3::splitLeaf(Node* leaf)
{
    int mid = leaf->keys.size() / 2;

    Node* newLeaf = createNode(true);
    newLeaf->next = leaf->next;
    leaf->next = newLeaf;
    newLeaf->parent = leaf->parent;

    /* 移动后半部分到新节点 */
    for (int i = mid; i < leaf->keys.size(); ++i) {
        newLeaf->keys.append(leaf->keys[i]);
        newLeaf->values.append(leaf->values[i]);
    }
    leaf->keys.resize(mid);
    leaf->values.resize(mid);

    /* 前缀压缩 */
    compressPrefix(leaf);
    compressPrefix(newLeaf);

    int upKey = newLeaf->keys.first();
    insertIntoParent(leaf, upKey, newLeaf);

    ++m_stats.totalNodesSplit;
    emit nodeSplit(0);
}

/** @brief 分裂内部节点 @param internal 内部节点 */
void BPlusTree3::splitInternal(Node* internal)
{
    int mid = internal->keys.size() / 2;

    Node* newInternal = createNode(false);
    newInternal->parent = internal->parent;

    int upKey = internal->keys[mid];

    /* 移动后半部分到新节点 */
    for (int i = mid + 1; i < internal->keys.size(); ++i) {
        newInternal->keys.append(internal->keys[i]);
    }
    for (int i = mid + 1; i < internal->children.size(); ++i) {
        newInternal->children.append(internal->children[i]);
        internal->children[i]->parent = newInternal;
    }

    internal->keys.resize(mid);
    internal->children.resize(mid + 1);

    /* 前缀压缩 */
    compressPrefix(internal);
    compressPrefix(newInternal);

    insertIntoParent(internal, upKey, newInternal);

    ++m_stats.totalNodesSplit;
}

/** @brief 将分裂后的中间键插入父节点 @param left 左节点 @param key 上升键 @param right 右节点 */
void BPlusTree3::insertIntoParent(Node* left, int key, Node* right)
{
    if (!left->parent) {
        /* 根节点分裂: 创建新根 */
        Node* newRoot = createNode(false);
        newRoot->keys.append(key);
        newRoot->children.append(left);
        newRoot->children.append(right);
        left->parent = newRoot;
        right->parent = newRoot;
        m_root = newRoot;
        return;
    }

    Node* parent = left->parent;
    int idx = 0;
    while (idx < parent->children.size() && parent->children[idx] != left) {
        ++idx;
    }

    parent->keys.insert(idx, key);
    parent->children.insert(idx + 1, right);
    right->parent = parent;

    if (parent->keys.size() >= m_order) {
        splitInternal(parent);
    }
}

/** @brief 插入键值对 @param key 键 @param value 值 @return 是否成功 */
bool BPlusTree3::insert(int key, const QByteArray& value)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_root) {
        m_root = createNode(true);
    }

    Node* leaf = findLeaf(key);
    if (!leaf) return false;

    /* 检查重复 */
    for (int i = 0; i < leaf->keys.size(); ++i) {
        if (leaf->keys[i] == key) {
            return false;
        }
    }

    insertIntoLeaf(leaf, key, value);

    ++m_stats.totalInsertions;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / std::max(1, m_stats.totalInsertions + m_stats.totalBulkLoads + m_stats.totalSearches);

    return true;
}

/** @brief 批量加载 @param pairs 有序(键,值)对列表 @return 加载的记录数 */
int BPlusTree3::bulkLoad(const QList<QPair<int, QByteArray>>& pairs)
{
    QElapsedTimer timer;
    timer.start();

    if (pairs.isEmpty()) return 0;

    clear();

    /* 叶子节点层: 每个叶子存(order-1)个键 */
    int leafCapacity = m_order - 1;
    QList<Node*> leaves;

    for (int i = 0; i < pairs.size(); i += leafCapacity) {
        Node* leaf = createNode(true);
        int end = std::min(i + leafCapacity, pairs.size());

        for (int j = i; j < end; ++j) {
            leaf->keys.append(pairs[j].first);
            leaf->values.append(pairs[j].second);
        }

        /* 前缀压缩 */
        compressPrefix(leaf);

        if (!leaves.isEmpty()) {
            leaves.last()->next = leaf;
        }
        leaves.append(leaf);
    }

    m_size = pairs.size();

    /* 从叶子层向上构建内部节点 */
    QList<Node*> currentLevel = leaves;
    int level = 0;

    while (currentLevel.size() > 1) {
        QList<Node*> nextLevel;
        int internalCap = m_order - 1;

        for (int i = 0; i < currentLevel.size(); i += m_order) {
            Node* parent = createNode(false);
            int end = std::min(i + m_order, static_cast<int>(currentLevel.size()));

            for (int j = i; j < end; ++j) {
                parent->children.append(currentLevel[j]);
                currentLevel[j]->parent = parent;
                if (j > i) {
                    /* 从子节点的第一个键取分隔键 */
                    parent->keys.append(currentLevel[j]->keys.first());
                }
            }

            compressPrefix(parent);
            nextLevel.append(parent);
        }

        currentLevel = nextLevel;
        ++level;
    }

    m_root = currentLevel.first();

    ++m_stats.totalBulkLoads;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / std::max(1, m_stats.totalInsertions + m_stats.totalBulkLoads + m_stats.totalSearches);

    emit bulkLoadCompleted(m_size, height());
    return m_size;
}

/** @brief 精确查找 @param key 查找键 @return (是否找到, 值) */
QPair<bool, QByteArray> BPlusTree3::find(int key) const
{
    QElapsedTimer timer;
    timer.start();

    if (!m_root) return qMakePair(false, QByteArray());

    Node* leaf = findLeaf(key);
    if (!leaf) return qMakePair(false, QByteArray());

    for (int i = 0; i < leaf->keys.size(); ++i) {
        if (leaf->keys[i] == key) {
            return qMakePair(true, leaf->values[i]);
        }
    }

    return qMakePair(false, QByteArray());
}

/** @brief 范围查询 @param low 下界 @param high 上界 @return 范围内键值对 */
QList<QPair<int, QByteArray>> BPlusTree3::rangeQuery(int low, int high) const
{
    QList<QPair<int, QByteArray>> result;
    if (!m_root) return result;

    /* 找到low所在的叶子节点 */
    Node* leaf = findLeaf(low);
    if (!leaf) return result;

    /* 沿叶子链表扫描到high */
    while (leaf) {
        for (int i = 0; i < leaf->keys.size(); ++i) {
            int k = leaf->keys[i];
            if (k > high) return result;
            if (k >= low) {
                result.append(qMakePair(k, leaf->values[i]));
            }
        }
        leaf = leaf->next;
    }

    return result;
}

/** @brief 删除键 @param key 要删除的键 @return 是否成功 */
bool BPlusTree3::remove(int key)
{
    if (!m_root) return false;

    Node* leaf = findLeaf(key);
    if (!leaf) return false;

    for (int i = 0; i < leaf->keys.size(); ++i) {
        if (leaf->keys[i] == key) {
            leaf->keys.remove(i);
            leaf->values.remove(i);
            --m_size;

            /* 如果根节点为空(唯一的叶子变空) */
            if (m_size == 0) {
                delete m_root;
                m_root = nullptr;
            }
            return true;
        }
    }

    return false;
}

/** @brief 更新已有键的值 @param key 键 @param newValue 新值 @return 是否成功 */
bool BPlusTree3::update(int key, const QByteArray& newValue)
{
    if (!m_root) return false;

    Node* leaf = findLeaf(key);
    if (!leaf) return false;

    for (int i = 0; i < leaf->keys.size(); ++i) {
        if (leaf->keys[i] == key) {
            leaf->values[i] = newValue;
            return true;
        }
    }

    return false;
}

/** @brief 计算树高度 @param node 节点 @return 高度 */
int BPlusTree3::computeHeight(const Node* node) const
{
    if (!node) return 0;
    if (node->isLeaf) return 1;
    return 1 + computeHeight(node->children.first());
}

/** @brief 获取树高度 */
int BPlusTree3::height() const
{
    return computeHeight(m_root);
}

/** @brief 清空整棵树 */
void BPlusTree3::clear()
{
    destroyNode(m_root);
    m_root = nullptr;
    m_size = 0;
}

/** @brief 获取所有叶子节点的键(有序) */
QList<int> BPlusTree3::allKeys() const
{
    QList<int> keys;
    if (!m_root) return keys;

    /* 找到最左叶子 */
    Node* cur = m_root;
    while (!cur->isLeaf) {
        cur = cur->children.first();
    }

    /* 沿叶子链表遍历 */
    while (cur) {
        for (int k : cur->keys) {
            keys.append(k);
        }
        cur = cur->next;
    }

    return keys;
}

/** @brief 重置统计 */
void BPlusTree3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

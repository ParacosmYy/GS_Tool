/**
 * @file TrieMap.cpp
 * @brief Trie字典树映射实现
 */

#include "utils/dict/TrieMap.h"

#include <limits>

/** @brief 构造函数 @param parent 父对象 */
TrieMap::TrieMap(QObject* parent)
    : QObject(parent)
    , m_root(new Node{})
    , m_nodeCount(1)
{
}

/** @brief 析构函数 — 释放所有节点 */
TrieMap::~TrieMap()
{
    deleteSubtree(m_root);
}

/**
 * @brief 插入键值对
 * @param key 字符串键
 * @param value 关联值
 */
void TrieMap::insert(const QString& key, double value)
{
    m_timer.start();

    Node* current = m_root;
    for (int i = 0; i < key.length(); ++i) {
        QChar c = key[i];
        if (!current->children.contains(c)) {
            current->children[c] = new Node{};
            ++m_nodeCount;
        }
        current = current->children[c];
    }

    current->value = value;
    bool wasNew = !current->isEnd;
    current->isEnd = true;

    /* 更新统计 */
    double elapsed = m_timer.elapsed();
    m_totalTimeMs += elapsed;
    ++m_stats.totalInserts;
    m_stats.avgProcessingTimeMs = m_totalTimeMs
        / static_cast<double>(m_stats.totalInserts + m_stats.totalLookups);

    if (wasNew) {
        emit structureChanged(m_nodeCount);
    }
}

/**
 * @brief 查找键对应的值
 * @param key 字符串键
 * @return 关联值(不存在返回NaN)
 */
double TrieMap::lookup(const QString& key) const
{
    m_timer.start();

    const Node* current = m_root;
    for (int i = 0; i < key.length(); ++i) {
        QChar c = key[i];
        if (!current->children.contains(c)) {
            /* 未找到 */
            double elapsed = m_timer.elapsed();
            m_totalTimeMs += elapsed;
            ++m_stats.totalLookups;
            m_stats.avgProcessingTimeMs = m_totalTimeMs
                / static_cast<double>(m_stats.totalInserts + m_stats.totalLookups);
            return std::numeric_limits<double>::quiet_NaN();
        }
        current = current->children[c];
    }

    double elapsed = m_timer.elapsed();
    m_totalTimeMs += elapsed;
    ++m_stats.totalLookups;
    m_stats.avgProcessingTimeMs = m_totalTimeMs
        / static_cast<double>(m_stats.totalInserts + m_stats.totalLookups);

    return current->isEnd ? current->value
                          : std::numeric_limits<double>::quiet_NaN();
}

/**
 * @brief 查询指定前缀的所有键
 * @param prefix 前缀字符串
 * @return 匹配的键列表
 */
QVector<QString> TrieMap::keysWithPrefix(const QString& prefix) const
{
    m_timer.start();

    QVector<QString> result;

    /* 定位到前缀末尾节点 */
    const Node* current = m_root;
    for (int i = 0; i < prefix.length(); ++i) {
        QChar c = prefix[i];
        if (!current->children.contains(c)) {
            /* 前缀不存在 */
            double elapsed = m_timer.elapsed();
            m_totalTimeMs += elapsed;
            ++m_stats.totalLookups;
            m_stats.avgProcessingTimeMs = m_totalTimeMs
                / static_cast<double>(m_stats.totalInserts + m_stats.totalLookups);
            return result;
        }
        current = current->children[c];
    }

    /* 递归收集所有以该前缀开头的键 */
    collectKeys(const_cast<Node*>(current), prefix, result);

    double elapsed = m_timer.elapsed();
    m_totalTimeMs += elapsed;
    ++m_stats.totalLookups;
    m_stats.avgProcessingTimeMs = m_totalTimeMs
        / static_cast<double>(m_stats.totalInserts + m_stats.totalLookups);

    return result;
}

/**
 * @brief 删除键
 * @param key 待删除的键
 */
void TrieMap::remove(const QString& key)
{
    if (key.isEmpty()) return;
    m_timer.start();

    int oldCount = m_nodeCount;
    removeHelper(m_root, key, 0);

    double elapsed = m_timer.elapsed();
    m_totalTimeMs += elapsed;
    ++m_stats.totalInserts;  /* 删除也算修改操作 */
    m_stats.avgProcessingTimeMs = m_totalTimeMs
        / static_cast<double>(m_stats.totalInserts + m_stats.totalLookups);

    if (m_nodeCount != oldCount) {
        emit structureChanged(m_nodeCount);
    }
}

/** @brief 重置统计 */
void TrieMap::resetStatistics()
{
    m_stats = Stats{};
    m_totalTimeMs = 0.0;
}

/**
 * @brief 递归收集前缀下的所有键
 * @param node 当前节点
 * @param prefix 已累积的前缀
 * @param result 结果列表
 */
void TrieMap::collectKeys(Node* node, const QString& prefix,
                          QVector<QString>& result) const
{
    if (node->isEnd) {
        result.append(prefix);
    }

    for (auto it = node->children.constBegin();
         it != node->children.constEnd(); ++it) {
        collectKeys(it.value(), prefix + it.key(), result);
    }
}

/**
 * @brief 递归删除子树
 * @param node 待删除的子树根节点
 */
void TrieMap::deleteSubtree(Node* node)
{
    if (!node) return;
    for (auto it = node->children.constBegin();
         it != node->children.constEnd(); ++it) {
        deleteSubtree(it.value());
    }
    delete node;
}

/**
 * @brief 递归删除节点(修剪空分支)
 * @param node 当前节点
 * @param key 待删除的键
 * @param depth 当前深度
 * @return true表示该节点可被父节点删除
 */
bool TrieMap::removeHelper(Node* node, const QString& key, int depth)
{
    if (!node) return false;

    if (depth == key.length()) {
        /* 到达目标节点 */
        if (node->isEnd) {
            node->isEnd = false;
        }
        /* 如果没有子节点，可以删除 */
        return node->children.isEmpty();
    }

    QChar c = key[depth];
    if (!node->children.contains(c)) return false;

    bool shouldDelete = removeHelper(node->children[c], key, depth + 1);
    if (shouldDelete) {
        delete node->children[c];
        node->children.remove(c);
        --m_nodeCount;

        /* 如果本节点非终止且无其他子节点，也可删除 */
        return !node->isEnd && node->children.isEmpty();
    }
    return false;
}

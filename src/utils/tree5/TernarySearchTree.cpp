/**
 * @file TernarySearchTree.cpp
 * @brief 三叉搜索树实现 — 字符串存储与自动补全
 */

#include "utils/tree5/TernarySearchTree.h"

#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
TernarySearchTree::TernarySearchTree(QObject* parent)
    : QObject(parent)
    , m_root(nullptr)
    , m_count(0)
    , m_timeSum(0.0)
    , m_totalOps(0)
{
}

/** @brief 析构函数 */
TernarySearchTree::~TernarySearchTree()
{
    deleteTree(m_root);
}

/** @brief 插入一个字符串 @param word 待插入字符串 */
void TernarySearchTree::insert(const QString& word)
{
    if (word.isEmpty()) return;

    m_timer.start();

    /* 先检查是否已存在 */
    if (contains(word)) {
        return;
    }

    m_root = insertHelper(m_root, word, 0);
    ++m_count;

    ++m_stats.totalInserts;
    m_timeSum += static_cast<double>(m_timer.elapsed());
    m_totalOps = m_stats.totalInserts + m_stats.totalSearches;
    m_stats.avgProcessingTimeMs = m_timeSum / static_cast<double>(m_totalOps);

    emit wordInserted(word);
}

/** @brief 检查字符串是否存在 @param word 字符串 @return 是否存在 */
bool TernarySearchTree::contains(const QString& word) const
{
    m_timer.start();

    if (word.isEmpty()) {
        return false;
    }

    TSTNode* node = searchHelper(m_root, word, 0);
    bool found = (node != nullptr && node->isEnd);

    ++m_stats.totalSearches;
    m_timeSum += static_cast<double>(m_timer.elapsed());
    m_totalOps = m_stats.totalInserts + m_stats.totalSearches;
    if (m_totalOps > 0) {
        m_stats.avgProcessingTimeMs = m_timeSum / static_cast<double>(m_totalOps);
    }

    return found;
}

/** @brief 前缀搜索(自动补全) @param prefix 前缀 @return 匹配的字符串列表 */
QVector<QString> TernarySearchTree::search(const QString& prefix) const
{
    m_timer.start();

    QVector<QString> results;

    if (prefix.isEmpty()) {
        /* 空前缀: 返回所有字符串 */
        collectAll(m_root, QString(), results);
    } else {
        /* 找到前缀的结束节点 */
        TSTNode* node = searchHelper(m_root, prefix, 0);
        if (node != nullptr) {
            if (node->isEnd) {
                results.append(prefix);
            }
            /* 从eq分支继续收集所有后缀 */
            collectAll(node->eq, prefix, results);
        }
    }

    ++m_stats.totalSearches;
    m_timeSum += static_cast<double>(m_timer.elapsed());
    m_totalOps = m_stats.totalInserts + m_stats.totalSearches;
    if (m_totalOps > 0) {
        m_stats.avgProcessingTimeMs = m_timeSum / static_cast<double>(m_totalOps);
    }

    return results;
}

/** @brief 删除一个字符串(标记删除) @param word 待删除字符串 @return 是否成功 */
bool TernarySearchTree::remove(const QString& word)
{
    m_timer.start();

    if (word.isEmpty()) {
        return false;
    }

    TSTNode* node = searchHelper(m_root, word, 0);
    if (node == nullptr || !node->isEnd) {
        m_timeSum += static_cast<double>(m_timer.elapsed());
        return false;
    }

    /* 标记为非终止节点(惰性删除) */
    node->isEnd = false;
    --m_count;

    m_timeSum += static_cast<double>(m_timer.elapsed());
    m_totalOps = m_stats.totalInserts + m_stats.totalSearches;
    if (m_totalOps > 0) {
        m_stats.avgProcessingTimeMs = m_timeSum / static_cast<double>(m_totalOps);
    }

    return true;
}

/** @brief 重置统计 */
void TernarySearchTree::resetStatistics()
{
    m_stats    = Stats{};
    m_timeSum  = 0.0;
    m_totalOps = 0;
}

/** @brief 递归插入辅助 @param node 当前节点 @param word 字符串 @param idx 索引 @return 更新后的节点 */
TSTNode* TernarySearchTree::insertHelper(TSTNode* node,
                                         const QString& word, int idx)
{
    QChar c = word[idx];

    if (node == nullptr) {
        node = new TSTNode(c);
    }

    if (c < node->ch) {
        node->lo = insertHelper(node->lo, word, idx);
    } else if (c > node->ch) {
        node->hi = insertHelper(node->hi, word, idx);
    } else {
        /* 字符匹配 */
        if (idx < word.length() - 1) {
            node->eq = insertHelper(node->eq, word, idx + 1);
        } else {
            node->isEnd = true;
        }
    }

    return node;
}

/** @brief 递归查找辅助 @param node 当前节点 @param word 字符串 @param idx 索引 @return 结束节点 */
TSTNode* TernarySearchTree::searchHelper(TSTNode* node,
                                         const QString& word, int idx) const
{
    if (node == nullptr || idx >= word.length()) {
        return nullptr;
    }

    QChar c = word[idx];

    if (c < node->ch) {
        return searchHelper(node->lo, word, idx);
    } else if (c > node->ch) {
        return searchHelper(node->hi, word, idx);
    } else {
        if (idx == word.length() - 1) {
            return node;
        }
        return searchHelper(node->eq, word, idx + 1);
    }
}

/** @brief 收集以node为根的所有后缀 @param node 起始节点 @param prefix 前缀 @param results 结果列表 */
void TernarySearchTree::collectAll(TSTNode* node, const QString& prefix,
                                   QVector<QString>& results) const
{
    if (node == nullptr) return;

    /* 中序遍历: lo -> 当前 -> hi */
    collectAll(node->lo, prefix, results);

    QString newPrefix = prefix + node->ch;

    if (node->isEnd) {
        results.append(newPrefix);
    }

    collectAll(node->eq, newPrefix, results);
    collectAll(node->hi, prefix, results);
}

/** @brief 递归删除节点树 @param node 当前节点 */
void TernarySearchTree::deleteTree(TSTNode* node)
{
    if (node == nullptr) return;
    deleteTree(node->lo);
    deleteTree(node->eq);
    deleteTree(node->hi);
    delete node;
}

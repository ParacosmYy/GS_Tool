#include "SuffixTree7.h"
#include <QElapsedTimer>
#include <QMap>
#include <QtMath>
#include <algorithm>

/**
 * @brief 后缀树节点
 */
struct STNode7 {
    int start;          /* 边起始位置 */
    int end;            /* 边结束位置 */
    int suffixIndex;    /* 叶子节点的后缀起始 */
    QMap<QChar, STNode7*> children;
    STNode7* suffixLink;

    STNode7() : start(-1), end(-1), suffixIndex(-1), suffixLink(nullptr) {}
};

static QString stText7;
static STNode7* stRoot7 = nullptr;

/* 获取边的结束位置 */
static int stEdgeEnd7(STNode7* n, int globalEnd)
{
    if (n->end == -1) return globalEnd;
    return n->end;
}

/* 创建新节点 */
static STNode7* stNewNode7(int start, int end)
{
    STNode7* n = new STNode7();
    n->start = start;
    n->end = end;
    return n;
}

/* 递归收集所有后缀索引 */
static void stCollect7(STNode7* n, QVector<int>& positions)
{
    if (!n) return;
    if (n->children.isEmpty() && n->suffixIndex >= 0) {
        positions.append(n->suffixIndex);
        return;
    }
    for (auto it = n->children.begin(); it != n->children.end(); ++it)
        stCollect7(it.value(), positions);
}

/* 查找最长重复子串 */
static void stLongestRepeat7(STNode7* n, int depth, const QString& text,
    QString& bestStr, int& bestCount)
{
    if (!n) return;
    int edgeLen = stEdgeEnd7(n, text.size() - 1) - n->start + 1;
    if (n->start >= 0) depth += edgeLen;

    if (n->children.isEmpty()) return; /* 叶子 */

    /* 内部节点，统计叶子数 */
    int leafCount = 0;
    QVector<int> dummy;
    for (auto it = n->children.begin(); it != n->children.end(); ++it) {
        QVector<int> sub;
        stCollect7(it.value(), sub);
        leafCount += sub.size();
    }

    if (depth > bestStr.length() && leafCount >= 2) {
        /* 从根到当前节点拼出子串 */
        bestStr = text.mid(n->start - (depth - edgeLen), depth);
        bestCount = leafCount;
    }

    for (auto it = n->children.begin(); it != n->children.end(); ++it)
        stLongestRepeat7(it.value(), depth, text, bestStr, bestCount);
}

/* 统计不同子串 */
static int stCountDistinct7(STNode7* n, int globalEnd)
{
    if (!n) return 0;
    int edgeLen = stEdgeEnd7(n, globalEnd) - n->start + 1;
    if (n->children.isEmpty()) return edgeLen; /* 叶子贡献 */

    int count = 0;
    for (auto it = n->children.begin(); it != n->children.end(); ++it)
        count += stCountDistinct7(it.value(), globalEnd);
    return count + edgeLen;
}

/* 释放节点 */
static void stDelete7(STNode7* n)
{
    if (!n) return;
    for (auto it = n->children.begin(); it != n->children.end(); ++it)
        stDelete7(it.value());
    delete n;
}

/**
 * @brief 构造函数
 * @param parent 父对象指针
 */
SuffixTree7::SuffixTree7(QObject* parent) : QObject(parent) {}

/**
 * @brief 重置所有统计信息
 */
void SuffixTree7::resetStatistics() { m_stats = Stats(); m_timeSum = 0.0; }

/**
 * @brief 使用Ukkonen算法构建后缀树
 *
 * 使用简化版Ukkonen算法，对每个后缀逐个插入。
 * 时间复杂度O(n^2)，适用于中等长度文本。
 *
 * @param text 输入文本
 * @return 是否构建成功
 */
bool SuffixTree7::build(const QString& text)
{
    QElapsedTimer timer;
    timer.start();

    if (text.isEmpty()) {
        emit searchCompleted(0);
        return false;
    }

    /* 清理旧树 */
    stDelete7(stRoot7);
    stText7 = text;
    stRoot7 = new STNode7();

    int n = text.size();
    /* 逐后缀插入 */
    for (int i = 0; i < n; ++i) {
        STNode7* cur = stRoot7;
        int j = i;
        while (j < n) {
            QChar ch = text[j];
            if (!cur->children.contains(ch)) {
                STNode7* leaf = stNewNode7(j, n - 1);
                leaf->suffixIndex = i;
                cur->children[ch] = leaf;
                break;
            }
            STNode7* child = cur->children[ch];
            int edgeLen = stEdgeEnd7(child, n - 1) - child->start + 1;
            int k = 0;
            while (k < edgeLen && j + k < n && text[child->start + k] == text[j + k])
                ++k;
            if (k == edgeLen) {
                cur = child;
                j += k;
            } else {
                /* 分裂边 */
                STNode7* mid = stNewNode7(child->start, child->start + k - 1);
                cur->children[ch] = mid;
                child->start += k;
                mid->children[text[child->start]] = child;
                STNode7* leaf = stNewNode7(j + k, n - 1);
                leaf->suffixIndex = i;
                mid->children[text[j + k]] = leaf;
                break;
            }
        }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalSearchOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSearchOps;
    emit searchCompleted(0);
    return true;
}

/**
 * @brief 查找模式串在文本中的所有出现位置
 * @param pattern 待搜索的模式串
 * @return 所有匹配起始位置
 */
QVector<int> SuffixTree7::search(const QString& pattern)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> positions;
    if (!stRoot7 || pattern.isEmpty()) {
        emit searchCompleted(0);
        return positions;
    }

    STNode7* cur = stRoot7;
    int j = 0;
    int n = stText7.size();
    while (j < pattern.size()) {
        QChar ch = pattern[j];
        if (!cur->children.contains(ch)) break;
        STNode7* child = cur->children[ch];
        int edgeLen = stEdgeEnd7(child, n - 1) - child->start + 1;
        int k = 0;
        while (k < edgeLen && j < pattern.size() && stText7[child->start + k] == pattern[j]) {
            ++k; ++j;
        }
        if (j == pattern.size()) {
            stCollect7(child, positions);
            break;
        }
        if (k < edgeLen) break;
        cur = child;
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalSearchOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSearchOps;
    emit searchCompleted(positions.size());
    return positions;
}

/**
 * @brief 查找最长重复子串
 * @return 最长重复子串及其出现次数
 */
QPair<QString, int> SuffixTree7::longestRepeatedSubstring()
{
    QElapsedTimer timer;
    timer.start();

    QString best;
    int count = 0;
    if (stRoot7) stLongestRepeat7(stRoot7, 0, stText7, best, count);

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalSearchOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSearchOps;
    return {best, count};
}

/**
 * @brief 统计不同子串的总数
 * @return 不同子串数量
 */
int SuffixTree7::countDistinctSubstrings()
{
    QElapsedTimer timer;
    timer.start();

    int count = 0;
    if (stRoot7) count = stCountDistinct7(stRoot7, stText7.size() - 1);

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalSearchOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSearchOps;
    return count;
}

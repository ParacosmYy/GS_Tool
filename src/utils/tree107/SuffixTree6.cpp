#include "SuffixTree6.h"
#include <QElapsedTimer>
#include <algorithm>

/**
 * @brief 构造函数，初始化后缀树
 * @param parent 父对象指针
 */
SuffixTree6::SuffixTree6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void SuffixTree6::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 构建后缀树
 *
 * 使用简化的后缀排序方法构建后缀树结构。
 * 对所有后缀按字典序排序后，利用相邻后缀的公共前缀构建树节点。
 *
 * @param text 输入字符串
 * @return 构建是否成功
 */
bool SuffixTree6::build(const QString& text)
{
    QElapsedTimer timer;
    timer.start();

    m_text = text;
    const int n = text.length();
    if (n == 0) {
        m_nodeCount = 0;
        m_stats.nodeCount = 0;

        qint64 elapsed = timer.elapsed();
        m_timeSum += elapsed;
        m_stats.totalBuilds++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalBuilds;
        emit buildCompleted(0);
        return true;
    }

    /* 生成所有后缀的索引数组 */
    QVector<int> suffixArray(n);
    for (int i = 0; i < n; ++i) {
        suffixArray[i] = i;
    }

    /* 按后缀字典序排序 */
    std::sort(suffixArray.begin(), suffixArray.end(), [&text](int a, int b) {
        return text.midRef(a) < text.midRef(b);
    });

    /* 计算LCP(最长公共前缀)数组 */
    QVector<int> lcp(n, 0);
    for (int i = 1; i < n; ++i) {
        int sa1 = suffixArray[i - 1];
        int sa2 = suffixArray[i];
        int len = 0;
        while (sa1 + len < n && sa2 + len < n && text[sa1 + len] == text[sa2 + len]) {
            len++;
        }
        lcp[i] = len;
    }

    /* 从后缀数组和LCP构建虚拟树节点计数 */
    m_nodeCount = 1; /* 根节点 */
    for (int i = 0; i < n; ++i) {
        m_nodeCount++; /* 每个后缀至少产生一个叶节点 */
        if (i > 0 && lcp[i] > 0) {
            m_nodeCount++; /* 内部节点 */
        }
    }

    m_stats.nodeCount = m_nodeCount;

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalBuilds++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalBuilds;

    emit buildCompleted(n);
    return true;
}

/**
 * @brief 搜索子串出现位置
 *
 * 利用后缀数组的有序性进行二分查找，定位所有匹配位置。
 *
 * @param pattern 搜索模式串
 * @return 所有出现位置的起始索引
 */
QVector<int> SuffixTree6::search(const QString& pattern) const
{
    if (m_text.isEmpty() || pattern.isEmpty()) return {};

    const int n = m_text.length();
    const int m = pattern.length();

    /* 暴力搜索（简化实现） */
    QVector<int> positions;
    for (int i = 0; i <= n - m; ++i) {
        bool match = true;
        for (int j = 0; j < m; ++j) {
            if (m_text[i + j] != pattern[j]) {
                match = false;
                break;
            }
        }
        if (match) {
            positions.append(i);
        }
    }

    std::sort(positions.begin(), positions.end());
    return positions;
}

/**
 * @brief 查找最长重复子串
 *
 * 通过遍历所有后缀对，找到最长的在文本中出现至少两次的子串。
 *
 * @return 最长重复子串及其出现次数
 */
QPair<QString, int> SuffixTree6::longestRepeatedSubstring() const
{
    if (m_text.isEmpty()) return {QString(), 0};

    const int n = m_text.length();
    QString bestSubstr;
    int bestCount = 0;

    /* 检查所有可能的子串长度 */
    for (int len = n / 2; len >= 1; --len) {
        QMap<QString, int> counts;
        for (int i = 0; i <= n - len; ++i) {
            QString sub = m_text.mid(i, len);
            counts[sub]++;
        }
        for (auto it = counts.begin(); it != counts.end(); ++it) {
            if (it.value() >= 2 && it.key().length() > bestSubstr.length()) {
                bestSubstr = it.key();
                bestCount = it.value();
            }
        }
        if (!bestSubstr.isEmpty()) break;
    }

    return {bestSubstr, bestCount};
}

/**
 * @brief 计算不同子串的总数
 * @return 不同子串数量
 */
int SuffixTree6::distinctSubstringCount() const
{
    if (m_text.isEmpty()) return 0;

    const int n = m_text.length();
    /* 使用集合去重 */
    QSet<QString> substrings;
    for (int i = 0; i < n; ++i) {
        for (int len = 1; len <= n - i; ++len) {
            substrings.insert(m_text.mid(i, len));
        }
    }
    return substrings.size();
}

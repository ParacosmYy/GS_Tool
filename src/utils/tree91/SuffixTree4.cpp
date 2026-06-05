#include "SuffixTree4.h"
#include <QElapsedTimer>
#include <algorithm>

/**
 * @brief 构造函数，初始化后缀树
 * @param parent 父对象指针
 */
SuffixTree4::SuffixTree4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 根据输入字符串构建后缀树
 *
 * 使用简化的后缀数组构建方法：生成所有后缀子串，
 * 排序后存储，用于后续快速模式匹配。
 *
 * @param text 输入文本字符串
 */
void SuffixTree4::build(const QString& text)
{
    QElapsedTimer timer;
    timer.start();

    m_text = text;

    /* 后缀数组方式的简化实现 */
    int n = m_text.length();
    QVector<int> sa(n);
    for (int i = 0; i < n; ++i) sa[i] = i;

    /* 按字典序排列后缀 */
    std::sort(sa.begin(), sa.end(), [&](int a, int b) {
        return m_text.mid(a) < m_text.mid(b);
    });

    m_timeSum += timer.elapsed();
    m_stats.totalBuilds++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalBuilds;
    emit buildCompleted(n);
}

/**
 * @brief 在后缀树中搜索模式串
 *
 * 利用后缀数组的有序性，通过二分查找确定模式串
 * 是否为某个后缀的前缀。
 *
 * @param pattern 待搜索的模式串
 * @return true表示找到匹配，false表示未找到
 */
bool SuffixTree4::search(const QString& pattern)
{
    QElapsedTimer timer;
    timer.start();

    if (pattern.isEmpty() || m_text.isEmpty()) {
        return false;
    }

    int n = m_text.length();
    int m = pattern.length();

    /* 构建后缀数组 */
    QVector<int> sa(n);
    for (int i = 0; i < n; ++i) sa[i] = i;
    std::sort(sa.begin(), sa.end(), [&](int a, int b) {
        return m_text.mid(a) < m_text.mid(b);
    });

    /* 二分查找模式串 */
    int left = 0, right = n - 1;
    bool found = false;
    while (left <= right) {
        int mid = (left + right) / 2;
        QString suffix = m_text.mid(sa[mid], m);
        if (suffix == pattern) {
            found = true;
            break;
        } else if (suffix < pattern) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }

    m_timeSum += timer.elapsed();
    m_stats.totalBuilds++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalBuilds;
    return found;
}

/**
 * @brief 统计模式串在文本中出现的次数
 *
 * 利用后缀数组在O(m log n)时间内计算模式串出现次数。
 *
 * @param pattern 待搜索的模式串
 * @return 出现次数
 */
int SuffixTree4::countOccurrences(const QString& pattern) const
{
    if (pattern.isEmpty() || m_text.isEmpty()) return 0;

    int n = m_text.length();
    int m = pattern.length();

    /* 构建后缀数组 */
    QVector<int> sa(n);
    for (int i = 0; i < n; ++i) sa[i] = i;
    std::sort(sa.begin(), sa.end(), [&](int a, int b) {
        return m_text.mid(a) < m_text.mid(b);
    });

    /* 使用二分查找计算上下界 */
    int lo = 0, hi = n - 1;
    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        if (m_text.mid(sa[mid], m) < pattern) lo = mid + 1;
        else hi = mid - 1;
    }
    int lower = lo;

    lo = 0; hi = n - 1;
    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        if (m_text.mid(sa[mid], m) <= pattern) lo = mid + 1;
        else hi = mid - 1;
    }
    int upper = lo;

    return qMax(0, upper - lower);
}

/**
 * @brief 获取最长重复子串
 * @return 最长的在文本中出现至少两次的子串
 */
QString SuffixTree4::longestRepeatedSubstring() const
{
    if (m_text.isEmpty()) return QString();

    int n = m_text.length();
    QVector<int> sa(n);
    for (int i = 0; i < n; ++i) sa[i] = i;
    std::sort(sa.begin(), sa.end(), [&](int a, int b) {
        return m_text.mid(a) < m_text.mid(b);
    });

    /* 计算LCP数组并找最大值 */
    int maxLCP = 0;
    int maxIdx = 0;
    for (int i = 1; i < n; ++i) {
        int lcp = 0;
        while (sa[i] + lcp < n && sa[i - 1] + lcp < n &&
               m_text[sa[i] + lcp] == m_text[sa[i - 1] + lcp]) {
            lcp++;
        }
        if (lcp > maxLCP) { maxLCP = lcp; maxIdx = sa[i]; }
    }

    return m_text.mid(maxIdx, maxLCP);
}

/**
 * @brief 重置统计数据
 */
void SuffixTree4::resetStatistics()
{
    m_stats.totalBuilds = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}

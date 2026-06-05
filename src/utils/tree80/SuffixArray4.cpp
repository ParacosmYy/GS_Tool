/**
 * @file SuffixArray4.cpp
 * @brief 后缀数组实现 — SA-IS线性构建 + LCP + 模式搜索
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-06
 *
 * 使用SA-IS算法在O(n)时间内构建后缀数组，Kasai算法构建LCP数组，
 * 二分搜索实现O(m log n)模式匹配。
 */

#include "utils/tree80/SuffixArray4.h"

#include <QElapsedTimer>
#include <algorithm>

/** @brief 构造函数 */
SuffixArray4::SuffixArray4(QObject* parent) : QObject(parent) {}

/**
 * @brief 构建字符串的后缀数组(SA-IS算法)
 *
 * SA-IS步骤: 标记S/L-type → 识别LMS → 桶排序LMS →
 * 归纳排序(L-type,S-type) → 递归简化问题 → 最终归纳
 *
 * @param text 输入整数序列
 */
void SuffixArray4::build(const QVector<int>& text)
{
    QElapsedTimer timer;
    timer.start();

    m_text = text;
    int n = text.size();
    if (n == 0) { m_sa.clear(); emit arrayBuilt(0); return; }

    /* 确定字符集大小 */
    int maxVal = 0;
    for (int i = 0; i < n; ++i) maxVal = qMax(maxVal, text[i]);
    int alphabetSize = maxVal + 2;

    m_sa.resize(n);
    saisBuild(text, n, alphabetSize);

    m_stats.totalArraysBuilt++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    int totalOps = m_stats.totalArraysBuilt + m_stats.totalPatternSearches;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    emit arrayBuilt(n);
}

/**
 * @brief 构建LCP数组(Kasai算法)
 *
 * 核心性质: LCP[rank[i]] >= LCP[rank[i-1]] - 1，
 * 利用此性质避免重复比较，总复杂度O(n)。
 * @return LCP数组，lcp[i] = SA[i]与SA[i-1]的最长公共前缀长度
 */
QVector<int> SuffixArray4::buildLCP() const
{
    int n = m_text.size();
    QVector<int> lcp(n, 0);
    if (n <= 1 || m_sa.size() != n) return lcp;

    /* 构建rank数组 */
    QVector<int> rank(n);
    for (int i = 0; i < n; ++i) rank[m_sa[i]] = i;

    /* Kasai算法: 从h-1位置开始比较 */
    int h = 0;
    for (int i = 0; i < n; ++i) {
        if (rank[i] == 0) { h = 0; continue; }
        int j = m_sa[rank[i] - 1];
        while (i + h < n && j + h < n && m_text[i + h] == m_text[j + h]) h++;
        lcp[rank[i]] = h;
        if (h > 0) h--;
    }
    return lcp;
}

/**
 * @brief 搜索模式的所有出现位置
 *
 * 两次二分搜索: lower_bound找第一个>=模式的位置，
 * upper_bound找第一个>模式的位置，区间内即为匹配。
 * 时间复杂度: O(m log n + k)
 * @param pattern 待搜索的模式序列
 * @return 所有匹配起始位置(升序)
 */
QVector<int> SuffixArray4::search(const QVector<int>& pattern) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> result;
    int n = m_text.size(), m = pattern.size();
    if (n == 0 || m == 0 || m_sa.size() != n || m > n) return result;

    /* 比较后缀SA[saIdx]与模式的字典序 */
    auto cmpSuffix = [&](int saIdx) -> int {
        int suf = m_sa[saIdx];
        int len = qMin(m, n - suf);
        for (int i = 0; i < len; ++i) {
            if (m_text[suf + i] < pattern[i]) return -1;
            if (m_text[suf + i] > pattern[i]) return 1;
        }
        return (len < m) ? -1 : 0;
    };

    /* 二分搜索下界 */
    int lo = 0, hi = n;
    while (lo < hi) {
        int mid = (lo + hi) / 2;
        if (cmpSuffix(mid) < 0) lo = mid + 1; else hi = mid;
    }
    int lower = lo;

    /* 二分搜索上界 */
    auto cmpStrict = [&](int saIdx) -> int {
        int suf = m_sa[saIdx];
        int len = qMin(m, n - suf);
        for (int i = 0; i < len; ++i) {
            if (m_text[suf + i] > pattern[i]) return 1;
            if (m_text[suf + i] < pattern[i]) return -1;
        }
        return (len >= m) ? 1 : -1;
    };
    lo = lower; hi = n;
    while (lo < hi) {
        int mid = (lo + hi) / 2;
        if (cmpStrict(mid) <= 0) lo = mid + 1; else hi = mid;
    }

    for (int i = lower; i < lo; ++i) result.append(m_sa[i]);
    std::sort(result.begin(), result.end());

    m_stats.totalPatternSearches++;
    m_timeSum += timer.elapsed();
    int totalOps = m_stats.totalArraysBuilt + m_stats.totalPatternSearches;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    return result;
}

/** @brief 获取后缀数组副本 */
QVector<int> SuffixArray4::suffixArray() const { return m_sa; }

/** @brief 重置统计数据和内部存储 */
void SuffixArray4::resetStatistics()
{
    m_stats = Stats{}; m_timeSum = 0.0;
    m_sa.clear(); m_text.clear();
}

/* ─── SA-IS核心算法 ─── */

/**
 * @brief SA-IS构建后缀数组
 *
 * @param text 输入文本  @param n 文本长度  @param alphabetSize 字符集大小
 */
void SuffixArray4::saisBuild(const QVector<int>& text, int n, int alphabetSize)
{
    /* 步骤1: 标记S-type/L-type */
    QVector<bool> isSType(n, false);
    isSType[n - 1] = false;
    for (int i = n - 2; i >= 0; --i) {
        if (text[i] < text[i + 1]) isSType[i] = true;
        else if (text[i] == text[i + 1]) isSType[i] = isSType[i + 1];
        else isSType[i] = false;
    }

    /* 步骤2: 识别LMS位置(S-type且前一个为L-type) */
    QVector<int> lmsPositions;
    for (int i = 1; i < n; ++i)
        if (isSType[i] && !isSType[i - 1]) lmsPositions.append(i);
    int numLMS = lmsPositions.size();

    /* 步骤3: 构建桶 */
    QVector<int> bucketSize(alphabetSize, 0);
    for (int i = 0; i < n; ++i) bucketSize[text[i]]++;

    /* 步骤4: 将LMS放入桶尾 */
    for (int i = 0; i < n; ++i) m_sa[i] = -1;
    QVector<int> bucketTail(alphabetSize);
    int sum = 0;
    for (int i = 0; i < alphabetSize; ++i) { sum += bucketSize[i]; bucketTail[i] = sum - 1; }

    QVector<int> bucketPtr = bucketTail;
    for (int i = numLMS - 1; i >= 0; --i) {
        int c = text[lmsPositions[i]];
        m_sa[bucketPtr[c]--] = lmsPositions[i];
    }

    /* 归纳排序 */
    induceSortL(text, n, alphabetSize, bucketSize);
    induceSortS(text, n, alphabetSize, bucketSize);

    /* 步骤5: 为LMS子串分配名字 */
    QVector<int> lmsRank(n, -1);
    int rank = 0, prevLMS = -1;
    for (int i = 0; i < n; ++i) {
        if (m_sa[i] < 0) continue;
        bool isLMS = false;
        for (int j = 0; j < numLMS; ++j)
            if (lmsPositions[j] == m_sa[i]) { isLMS = true; break; }
        if (!isLMS) continue;

        if (prevLMS >= 0) {
            bool same = true;
            for (int j = 0; j < n && same; ++j) {
                int a = prevLMS + j, b = m_sa[i] + j;
                bool aLMS = (a > 0 && isSType[a] && !isSType[a - 1]);
                bool bLMS = (b > 0 && isSType[b] && !isSType[b - 1]);
                if (a >= n || b >= n) { same = (a >= n && b >= n); break; }
                if (text[a] != text[b]) { same = false; break; }
                if (aLMS || bLMS) { same = (aLMS && bLMS); break; }
            }
            if (!same) rank++;
        }
        for (int j = 0; j < numLMS; ++j)
            if (lmsPositions[j] == m_sa[i]) { lmsRank[j] = rank; break; }
        prevLMS = m_sa[i];
    }

    /* 步骤6: 递归(如果LMS子串不唯一) */
    if (rank + 1 < numLMS) {
        QVector<int> reducedText(numLMS);
        for (int i = 0; i < numLMS; ++i) reducedText[i] = lmsRank[i] + 1;
        SuffixArray4 subSA;
        subSA.m_sa.resize(numLMS);
        subSA.saisBuild(reducedText, numLMS, rank + 2);
        for (int i = 0; i < numLMS; ++i) lmsRank[i] = lmsPositions[subSA.m_sa[i]];
    }

    /* 步骤7: 最终归纳排序 */
    for (int i = 0; i < n; ++i) m_sa[i] = -1;
    bucketPtr = bucketTail;
    for (int i = numLMS - 1; i >= 0; --i) {
        int pos = lmsRank[i];
        m_sa[bucketPtr[text[pos]]--] = pos;
    }
    induceSortL(text, n, alphabetSize, bucketSize);
    induceSortS(text, n, alphabetSize, bucketSize);
}

/** @brief 归纳排序L-type: 从左到右扫描SA，填入L-type后缀 */
void SuffixArray4::induceSortL(const QVector<int>& text, int n,
                                int alphabetSize, const QVector<int>& bucketSize)
{
    QVector<int> bucketHead(alphabetSize, 0);
    int sum = 0;
    for (int i = 0; i < alphabetSize; ++i) { bucketHead[i] = sum; sum += bucketSize[i]; }

    for (int i = 0; i < n; ++i) {
        if (m_sa[i] <= 0) continue;
        int j = m_sa[i] - 1;
        /* j是L-type: text[j] > text[j+1]，或相等且j+1非S-type */
        bool isL = (j + 1 < n && text[j] > text[j + 1]) ||
                   (j + 1 < n && text[j] == text[j + 1] && !isSPosition(j + 1, n, text));
        if (isL) m_sa[bucketHead[text[j]]++] = j;
    }
}

/** @brief 归纳排序S-type: 从右到左扫描SA，填入S-type后缀 */
void SuffixArray4::induceSortS(const QVector<int>& text, int n,
                                int alphabetSize, const QVector<int>& bucketSize)
{
    QVector<int> bucketTail(alphabetSize, 0);
    int sum = 0;
    for (int i = 0; i < alphabetSize; ++i) { sum += bucketSize[i]; bucketTail[i] = sum - 1; }

    for (int i = n - 1; i >= 0; --i) {
        if (m_sa[i] <= 0) continue;
        int j = m_sa[i] - 1;
        bool isS = (j + 1 < n && text[j] < text[j + 1]) ||
                   (j + 1 < n && text[j] == text[j + 1] && isSPosition(j + 1, n, text));
        if (isS) m_sa[bucketTail[text[j]]--] = j;
    }
}

/** @brief 判断位置pos是否为S-type */
bool SuffixArray4::isSPosition(int pos, int n, const QVector<int>& text) const
{
    if (pos >= n - 1) return false;
    if (text[pos] < text[pos + 1]) return true;
    if (text[pos] > text[pos + 1]) return false;
    return isSPosition(pos + 1, n, text);
}

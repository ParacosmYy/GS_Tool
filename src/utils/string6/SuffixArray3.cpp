/**
 * @file SuffixArray3.cpp
 * @brief SA-IS线性时间后缀数组构造算法实现
 */

#include "utils/string6/SuffixArray3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

SuffixArray3::SuffixArray3(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

QVector<int> SuffixArray3::build(const QVector<int>& text, int n,
                                  int alphabetSize)
{
    QElapsedTimer timer;
    timer.start();

    if (n <= 0 || text.size() < n) return {};
    if (n == 1) return {0};

    QVector<int> sa(n, -1);
    saisCore(text, sa, n, alphabetSize);

    /* 更新统计 */
    ++m_stats.totalArraysBuilt;
    m_stats.totalCharactersProcessed += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalArraysBuilt;

    emit arrayBuilt(n, timer.elapsed());
    return sa;
}

QVector<int> SuffixArray3::buildFromString(const QString& text)
{
    if (text.isEmpty()) return {};

    int n = text.size();
    QVector<int> intText(n + 1);
    for (int i = 0; i < n; ++i) {
        intText[i] = static_cast<int>(text[i].unicode()) + 1;
    }
    intText[n] = 0;  /* 哨兵 */

    /* 找最大字符值确定字母表大小 */
    int maxChar = 0;
    for (int i = 0; i < n; ++i) {
        maxChar = qMax(maxChar, intText[i]);
    }

    QVector<int> sa = build(intText, n + 1, maxChar + 1);

    /* 移除哨兵位置的项 */
    QVector<int> result;
    result.reserve(n);
    for (int i = 0; i < sa.size(); ++i) {
        if (sa[i] < n) result.append(sa[i]);
    }
    return result;
}

QVector<int> SuffixArray3::buildFromBytes(const QByteArray& data)
{
    if (data.isEmpty()) return {};

    int n = data.size();
    QVector<int> intText(n + 1);
    for (int i = 0; i < n; ++i) {
        intText[i] = static_cast<quint8>(data[i]) + 1;
    }
    intText[n] = 0;

    QVector<int> sa = build(intText, n + 1, 257);

    QVector<int> result;
    result.reserve(n);
    for (int i = 0; i < sa.size(); ++i) {
        if (sa[i] < n) result.append(sa[i]);
    }
    return result;
}

QVector<int> SuffixArray3::buildLCP(const QVector<int>& text,
                                     const QVector<int>& sa)
{
    int n = sa.size();
    if (n <= 0 || text.size() < n) return {};

    QVector<int> lcp(n, 0);
    QVector<int> rank(n);

    /* 构建rank数组: rank[sa[i]] = i */
    for (int i = 0; i < n; ++i) {
        rank[sa[i]] = i;
    }

    /* Kasai算法: O(n)时间构造LCP */
    int h = 0;
    for (int i = 0; i < n; ++i) {
        if (rank[i] > 0) {
            int j = sa[rank[i] - 1];
            while (i + h < n && j + h < n && text[i + h] == text[j + h]) {
                ++h;
            }
            lcp[rank[i]] = h;
            if (h > 0) --h;
        }
    }

    return lcp;
}

SuffixArray3::SearchResult SuffixArray3::search(
    const QVector<int>& text, const QVector<int>& sa,
    const QVector<int>& pattern)
{
    SearchResult result;
    int n = sa.size();
    int m = pattern.size();

    if (n <= 0 || m <= 0) return result;

    /* 二分搜索找左边界 */
    int lo = 0, hi = n - 1;
    int left = n;
    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        int cmp = 0;
        for (int i = 0; i < m && sa[mid] + i < n; ++i) {
            if (text[sa[mid] + i] < pattern[i]) { cmp = -1; break; }
            if (text[sa[mid] + i] > pattern[i]) { cmp = 1; break; }
        }
        if (cmp == 0 && sa[mid] + m > n) cmp = -1;

        if (cmp >= 0) { left = mid; hi = mid - 1; }
        else { lo = mid + 1; }
    }

    /* 二分搜索找右边界 */
    lo = left; hi = n - 1;
    int right = left - 1;
    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        int cmp = 0;
        for (int i = 0; i < m && sa[mid] + i < n; ++i) {
            if (text[sa[mid] + i] < pattern[i]) { cmp = -1; break; }
            if (text[sa[mid] + i] > pattern[i]) { cmp = 1; break; }
        }
        if (cmp == 0 && sa[mid] + m > n) cmp = -1;

        if (cmp <= 0) { right = mid; lo = mid + 1; }
        else { hi = mid - 1; }
    }

    result.count = right - left + 1;
    result.positions.reserve(result.count);
    for (int i = left; i <= right; ++i) {
        result.positions.append(sa[i]);
    }
    std::sort(result.positions.begin(), result.positions.end());
    result.matchRatio = (n > 0) ? static_cast<double>(result.count) / n : 0.0;

    ++m_stats.totalSearches;
    m_stats.totalMatches += result.count;

    emit searchCompleted(m, result.count);
    return result;
}

SuffixArray3::SearchResult SuffixArray3::searchString(
    const QString& text, const QVector<int>& sa, const QString& pattern)
{
    QVector<int> intText(text.size());
    for (int i = 0; i < text.size(); ++i) {
        intText[i] = static_cast<int>(text[i].unicode());
    }
    QVector<int> intPattern(pattern.size());
    for (int i = 0; i < pattern.size(); ++i) {
        intPattern[i] = static_cast<int>(pattern[i].unicode());
    }
    return search(intText, sa, intPattern);
}

QVector<QPair<int, int>> SuffixArray3::longestRepeats(
    const QVector<int>& lcp, const QVector<int>& sa, int minLength)
{
    QVector<QPair<int, int>> result;

    /* LCP值最大的位置对应最长重复子串 */
    for (int i = 1; i < lcp.size(); ++i) {
        if (lcp[i] >= minLength) {
            result.append({sa[i], lcp[i]});
        }
    }

    /* 按长度降序排序 */
    std::sort(result.begin(), result.end(),
        [](const QPair<int, int>& a, const QPair<int, int>& b) {
            return a.second > b.second;
        });

    /* 去重: 保留每个长度的第一个 */
    QSet<int> seen;
    QVector<QPair<int, int>> unique;
    for (auto& p : result) {
        if (!seen.contains(p.second)) {
            seen.insert(p.second);
            unique.append(p);
        }
    }

    return unique;
}

qint64 SuffixArray3::distinctSubstrings(int n, const QVector<int>& lcp)
{
    /* 不同子串数 = n*(n+1)/2 - sum(LCP) */
    qint64 total = static_cast<qint64>(n) * (n + 1) / 2;
    for (int i = 0; i < lcp.size(); ++i) {
        total -= lcp[i];
    }
    return qMax(0LL, total);
}

SuffixArray3::Stats SuffixArray3::stats() const
{
    return m_stats;
}

void SuffixArray3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

void SuffixArray3::saisCore(const QVector<int>& s, QVector<int>& sa,
                             int n, int K)
{
    /* SA-IS递归实现 */
    if (n <= 1) {
        if (n == 1) sa[0] = 0;
        return;
    }

    /* 步骤1: 分类L/S型 */
    QVector<bool> types = classifyTypes(s, n);

    /* 步骤2: 找LMS后缀 */
    QVector<int> lms = findLMS(types, n);
    int nLms = lms.size();

    if (nLms <= 1) {
        /* 递归基: 直接排序 */
        induceSort(s, sa, n, K, lms);
        return;
    }

    /* 步骤3: 诱导排序得到SA */
    for (int i = 0; i < n; ++i) sa[i] = -1;
    induceSort(s, sa, n, K, lms);

    /* 步骤4: 从SA中提取LMS后缀的排序 */
    QVector<int> sortedLms;
    for (int i = 0; i < n; ++i) {
        if (sa[i] >= 0) {
            bool isLms = false;
            if (sa[i] > 0 && types[sa[i]]) {
                if (sa[i] - 1 >= 0 && !types[sa[i] - 1]) {
                    isLms = true;
                }
            }
            if (isLms) sortedLms.append(sa[i]);
        }
    }

    /* 步骤5: 为LMS子串命名 */
    QVector<int> lmsRank(n, -1);
    int rank = 0;
    lmsRank[sortedLms[0]] = rank;

    for (int i = 1; i < sortedLms.size(); ++i) {
        bool same = true;
        for (int j = 0; j < n; ++j) {
            int p1 = sortedLms[i - 1] + j;
            int p2 = sortedLms[i] + j;
            if (p1 >= n || p2 >= n || s[p1] != s[p2] ||
                types[p1] != types[p2]) {
                same = false;
                break;
            }
            if (j > 0) {
                bool lms1 = types[p1] && p1 > 0 && !types[p1 - 1];
                bool lms2 = types[p2] && p2 > 0 && !types[p2 - 1];
                if (lms1 || lms2) break;
            }
        }
        if (!same) ++rank;
        lmsRank[sortedLms[i]] = rank;
    }

    /* 步骤6: 递归求解缩减问题 */
    QVector<int> s1(nLms);
    QVector<int> lmsMap(nLms);
    for (int i = 0; i < nLms; ++i) {
        s1[i] = lmsRank[lms[i]];
        lmsMap[i] = lms[i];
    }

    QVector<int> sa1(nLms, -1);
    if (rank + 1 < nLms) {
        /* 需要递归 */
        saisCore(s1, sa1, nLms, rank + 1);
    } else {
        /* 直接从rank得到SA */
        for (int i = 0; i < nLms; ++i) {
            sa1[s1[i]] = i;
        }
    }

    /* 步骤7: 用递归结果映射回原LMS位置 */
    QVector<int> finalLms(nLms);
    for (int i = 0; i < nLms; ++i) {
        finalLms[i] = lmsMap[sa1[i]];
    }

    /* 步骤8: 最终诱导排序 */
    for (int i = 0; i < n; ++i) sa[i] = -1;
    induceSort(s, sa, n, K, finalLms);
}

void SuffixArray3::induceSort(const QVector<int>& s, QVector<int>& sa,
                               int n, int K, const QVector<int>& lmsOffsets)
{
    /* 步骤A: 计算桶大小 */
    QVector<int> bucket(K + 1, 0);
    for (int i = 0; i < n; ++i) {
        if (s[i] >= 0 && s[i] < K) ++bucket[s[i]];
    }

    /* 前缀和得到桶的结束位置 */
    QVector<int> bucketEnd(K + 1);
    int sum = 0;
    for (int i = 0; i <= K; ++i) {
        sum += bucket[i];
        bucketEnd[i] = sum;
    }

    /* 步骤B: 将LMS后缀放入桶尾 */
    QVector<int> bucketPos = bucketEnd;
    for (int i = lmsOffsets.size() - 1; i >= 0; --i) {
        int c = s[lmsOffsets[i]];
        sa[--bucketPos[c]] = lmsOffsets[i];
    }

    /* 步骤C: 诱导排序L型后缀(从左到右) */
    QVector<int> bucketStart(K + 1, 0);
    for (int i = 1; i <= K; ++i) {
        bucketStart[i] = bucketEnd[i - 1];
    }

    for (int i = 0; i < n; ++i) {
        if (sa[i] <= 0) continue;
        int j = sa[i] - 1;
        int c = s[j];
        /* L型: s[j] <= s[j+1] */
        if (j + 1 < n && s[j] <= s[j + 1]) {
            sa[bucketStart[c]++] = j;
        }
    }

    /* 步骤D: 诱导排序S型后缀(从右到左) */
    bucketPos = bucketEnd;
    for (int i = n - 1; i >= 0; --i) {
        if (sa[i] <= 0) continue;
        int j = sa[i] - 1;
        int c = s[j];
        /* S型: s[j] > s[j+1] */
        if (j + 1 < n && s[j] > s[j + 1]) {
            sa[--bucketPos[c]] = j;
        }
    }
}

QVector<bool> SuffixArray3::classifyTypes(const QVector<int>& s, int n)
{
    QVector<bool> types(n, false);
    if (n <= 1) return types;

    types[n - 1] = true;  /* 最后一个字符是S型 */

    for (int i = n - 2; i >= 0; --i) {
        if (s[i] < s[i + 1]) {
            types[i] = true;
        } else if (s[i] == s[i + 1]) {
            types[i] = types[i + 1];
        }
        /* s[i] > s[i+1]: 默认false(L型) */
    }

    return types;
}

QVector<int> SuffixArray3::findLMS(const QVector<bool>& types, int n)
{
    QVector<int> lms;
    for (int i = 1; i < n; ++i) {
        /* LMS: 当前是S型, 前一个是L型 */
        if (types[i] && !types[i - 1]) {
            lms.append(i);
        }
    }
    return lms;
}

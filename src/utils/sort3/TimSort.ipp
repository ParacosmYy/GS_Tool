/**
 * @file TimSort.ipp
 * @brief TimSort混合稳定排序 — 模板实现
 *
 * 此文件包含TimSort的模板方法实现, 被TimSort.h间接包含。
 * 不要在此文件中 #include "TimSort.h", 因为它由头文件包含。
 */

#include <algorithm>

// ═══════════════════════════════════════════════════════════
// 模板排序实现
// ═══════════════════════════════════════════════════════════

template<typename T>
void TimSort::sortWithComparator(QVector<T>& data,
                                 const std::function<bool(const T&, const T&)>& comp)
{
    const int n = data.size();
    if (n <= 1) {
        m_stats.totalSorts += 1;
        emit sortCompleted(n);
        return;
    }

    const int minRun = minRunLength(n);
    QVector<int> stack;
    stack.reserve(64);

    int i = 0;
    while (i < n) {
        int runEnd = i + 1;
        if (runEnd < n) {
            if (comp(data[runEnd], data[i])) {
                while (runEnd + 1 < n && comp(data[runEnd + 1], data[runEnd])) {
                    ++runEnd;
                }
                std::reverse(data.begin() + i, data.begin() + runEnd + 1);
            } else {
                while (runEnd + 1 < n && !comp(data[runEnd + 1], data[runEnd])) {
                    ++runEnd;
                }
            }
        }

        if (runEnd - i + 1 < minRun) {
            const int extend = qMin(n - 1, i + minRun - 1);
            insertionSortImpl(data, i, extend, comp);
            runEnd = extend;
        }

        stack.append(i);
        stack.append(runEnd);

        while (stack.size() >= 4) {
            const int rEnd = stack[stack.size() - 1];
            const int rStart = stack[stack.size() - 2];
            const int lEnd = stack[stack.size() - 3];
            const int lStart = stack[stack.size() - 4];

            const int len1 = lEnd - lStart + 1;
            const int len2 = rEnd - rStart + 1;

            if (len1 <= len2) {
                mergeImpl(data, lStart, lEnd, rEnd, comp);
                stack[stack.size() - 4] = lStart;
                stack[stack.size() - 3] = rEnd;
                stack.remove(stack.size() - 2, 2);
            } else {
                break;
            }
        }

        i = runEnd + 1;
    }

    while (stack.size() >= 4) {
        const int rEnd = stack[stack.size() - 1];
        const int rStart = stack[stack.size() - 2];
        const int lEnd = stack[stack.size() - 3];
        const int lStart = stack[stack.size() - 4];

        mergeImpl(data, lStart, lEnd, rEnd, comp);
        stack[stack.size() - 4] = lStart;
        stack[stack.size() - 3] = rEnd;
        stack.remove(stack.size() - 2, 2);
    }

    m_stats.totalSorts += 1;
    emit sortCompleted(n);
}

template<typename T>
void TimSort::insertionSortImpl(QVector<T>& data, int left, int right,
                                const std::function<bool(const T&, const T&)>& comp)
{
    for (int i = left + 1; i <= right; ++i) {
        const T key = data[i];
        int j = i - 1;
        while (j >= left && comp(key, data[j])) {
            data[j + 1] = data[j];
            --j;
            m_stats.totalComparisons += 1;
        }
        m_stats.totalComparisons += 1;
        data[j + 1] = key;
    }
}

template<typename T>
void TimSort::mergeImpl(QVector<T>& data, int l, int m, int r,
                        const std::function<bool(const T&, const T&)>& comp)
{
    const int leftSize = m - l + 1;
    const int rightSize = r - m;

    QVector<T> leftPart;
    leftPart.reserve(leftSize);
    QVector<T> rightPart;
    rightPart.reserve(rightSize);

    for (int i = 0; i < leftSize; ++i) {
        leftPart.append(data[l + i]);
    }
    for (int i = 0; i < rightSize; ++i) {
        rightPart.append(data[m + 1 + i]);
    }

    int i = 0, j = 0, k = l;
    while (i < leftSize && j < rightSize) {
        m_stats.totalComparisons += 1;
        if (!comp(rightPart[j], leftPart[i])) {
            data[k++] = leftPart[i++];
        } else {
            data[k++] = rightPart[j++];
        }
    }

    while (i < leftSize) {
        data[k++] = leftPart[i++];
    }
    while (j < rightSize) {
        data[k++] = rightPart[j++];
    }
}

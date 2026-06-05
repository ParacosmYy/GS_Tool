/**
 * @file TimSort.cpp
 * @brief TimSort混合稳定排序实现 — 归并+插入混合排序
 */

#include "TimSort.h"

#include <QElapsedTimer>
#include <algorithm>

// ═══════════════════════════════════════════════════════════
// 构造 / 析构
// ═══════════════════════════════════════════════════════════

TimSort::TimSort(QObject* parent)
    : QObject(parent)
{
}

TimSort::~TimSort() = default;

// ═══════════════════════════════════════════════════════════
// 排序
// ═══════════════════════════════════════════════════════════

void TimSort::sort(QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    const int n = data.size();
    if (n <= 1) {
        m_stats.totalSorts += 1;
        emit sortCompleted(n);
        return;
    }

    const int minRun = minRunLength(n);
    // 使用栈来记录待合并的run边界
    QVector<int> stack;
    stack.reserve(64);

    int i = 0;
    while (i < n) {
        // 找到一个自然run(升序或降序)
        int runEnd = i + 1;
        if (runEnd < n) {
            // 判断升降序
            if (data[runEnd] < data[i]) {
                // 降序: 继续找到降序结束
                while (runEnd + 1 < n && data[runEnd + 1] < data[runEnd]) {
                    ++runEnd;
                }
                // 反转为升序
                std::reverse(data.begin() + i, data.begin() + runEnd + 1);
            } else {
                // 升序: 继续找到升序结束
                while (runEnd + 1 < n && data[runEnd + 1] >= data[runEnd]) {
                    ++runEnd;
                }
            }
        }

        // 如果run太短, 用插入排序扩展到minRun
        if (runEnd - i + 1 < minRun) {
            const int extend = qMin(n - 1, i + minRun - 1);
            insertionSort(data, i, extend);
            runEnd = extend;
        }

        // 将run边界压入栈
        stack.append(i);
        stack.append(runEnd);

        // 合并以满足不变式
        while (stack.size() >= 4) {
            const int rEnd = stack[stack.size() - 1];
            const int rStart = stack[stack.size() - 2];
            const int lEnd = stack[stack.size() - 3];
            const int lStart = stack[stack.size() - 4];

            const int len1 = lEnd - lStart + 1;
            const int len2 = rEnd - rStart + 1;

            // 合并条件: 较小run <= 较大run
            if (len1 <= len2) {
                merge(data, lStart, lEnd, rEnd);
                // 合并后替换栈顶两个run为一个
                stack[stack.size() - 4] = lStart;
                stack[stack.size() - 3] = rEnd;
                stack.remove(stack.size() - 2, 2);
            } else {
                break;
            }
        }

        i = runEnd + 1;
    }

    // 合并栈上剩余的所有run
    while (stack.size() >= 4) {
        const int rEnd = stack[stack.size() - 1];
        const int rStart = stack[stack.size() - 2];
        const int lEnd = stack[stack.size() - 3];
        const int lStart = stack[stack.size() - 4];

        merge(data, lStart, lEnd, rEnd);
        stack[stack.size() - 4] = lStart;
        stack[stack.size() - 3] = rEnd;
        stack.remove(stack.size() - 2, 2);
    }

    m_stats.totalSorts += 1;
    const double elapsedMs = static_cast<double>(timer.elapsed());
    updateAvgTime(elapsedMs);

    emit sortCompleted(n);
}

// ═══════════════════════════════════════════════════════════
// 统计
// ═══════════════════════════════════════════════════════════

TimSort::Stats TimSort::stats() const
{
    return m_stats;
}

void TimSort::resetStatistics()
{
    m_stats = Stats{};
}

// ═══════════════════════════════════════════════════════════
// 内部辅助
// ═══════════════════════════════════════════════════════════

int TimSort::minRunLength(int n) const
{
    int r = 0;
    while (n >= MIN_MERGE) {
        r |= (n & 1);
        n >>= 1;
    }
    return n + r;
}

void TimSort::insertionSort(QVector<double>& data, int left, int right)
{
    for (int i = left + 1; i <= right; ++i) {
        const double key = data[i];
        int j = i - 1;
        while (j >= left && data[j] > key) {
            data[j + 1] = data[j];
            --j;
            m_stats.totalComparisons += 1;
        }
        m_stats.totalComparisons += 1; // 最后一次比较失败
        data[j + 1] = key;
    }
}

void TimSort::merge(QVector<double>& data, int l, int m, int r)
{
    // 复制左右两半到临时数组
    const int leftSize = m - l + 1;
    const int rightSize = r - m;

    QVector<double> leftPart;
    leftPart.reserve(leftSize);
    QVector<double> rightPart;
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
        if (leftPart[i] <= rightPart[j]) {
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

void TimSort::updateAvgTime(double elapsedMs) const
{
    if (m_stats.totalSorts <= 1) {
        m_stats.avgProcessingTimeMs = elapsedMs;
    } else {
        m_stats.avgProcessingTimeMs =
            m_stats.avgProcessingTimeMs *
                static_cast<double>(m_stats.totalSorts - 1) /
                static_cast<double>(m_stats.totalSorts) +
            elapsedMs / static_cast<double>(m_stats.totalSorts);
    }
}

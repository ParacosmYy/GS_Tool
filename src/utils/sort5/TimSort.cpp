/**
 * @file TimSort.cpp
 * @brief TimSort排序实现 — 自适应归并 + Galloping模式
 */

#include "utils/sort5/TimSort.h"

#include <QElapsedTimer>
#include <algorithm>

/**
 * @brief 构造函数
 * @param parent 父对象
 */
TimSort::TimSort(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
    , m_elemSum(0)
{
}

/**
 * @brief 升序排序
 * @param data 待排序数组
 */
void TimSort::sort(QVector<double>& data)
{
    sortWithComparator(data, [](double a, double b) { return a < b; });
}

/**
 * @brief 自定义比较器排序
 * @param data 待排序数组
 * @param cmp 比较函数
 *
 * TimSort算法流程:
 * 1. 扫描数组找到自然有序的run
 * 2. 对不足最小长度的run用二分插入排序扩展
 * 3. 将run压入栈，维护栈不变量(递减长度)
 * 4. 当不变量被破坏时归并相邻run
 */
void TimSort::sortWithComparator(QVector<double>& data,
                                   std::function<bool(double, double)> cmp)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n < 2) {
        double elapsed = static_cast<double>(timer.elapsed());
        ++m_stats.totalSorted;
        m_elemSum += static_cast<quint64>(n);
        m_stats.totalElements = m_elemSum;
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs =
            m_timeSum / static_cast<double>(m_stats.totalSorted);
        emit sortCompleted(n);
        return;
    }

    int minRun = calcMinRun(n);
    QVector<Run> runStack;
    runStack.reserve(64);

    int i = 0;
    int remaining = n;

    while (remaining > 0) {
        /* 找到下一个自然run */
        int runLen = extendRun(data, i, n, cmp);

        /* 如果run太短，用二分插入排序扩展到minRun */
        if (runLen < minRun) {
            int force = qMin(minRun, remaining);
            binaryInsertionSort(data, i, i + force, cmp);
            runLen = force;
        }

        /* 压入栈 */
        runStack.append({i, runLen});

        /* 合并以维护栈不变量: C > A+B 且 B > A */
        while (runStack.size() > 1) {
            int sz = runStack.size();
            Run& a = runStack[sz - 2];
            Run& b = runStack[sz - 1];

            bool needMerge = false;

            if (sz >= 3) {
                Run& c = runStack[sz - 3];
                /* 不变量: c.len > a.len + b.len 且 a.len > b.len */
                if (c.length <= a.length + b.length) {
                    needMerge = true;
                }
            }

            if (!needMerge && a.length <= b.length) {
                needMerge = true;
            }

            if (!needMerge) break;

            /* 决定归并策略 */
            if (sz >= 3 && runStack[sz - 3].length <= a.length + b.length) {
                if (runStack[sz - 3].length < b.length) {
                    /* 先归并C和A */
                    mergeAt(data, runStack[sz - 3], a);
                    runStack[sz - 3].length += a.length;
                    runStack.removeAt(sz - 2);
                } else {
                    /* 归并A和B */
                    mergeAt(data, a, b);
                    a.length += b.length;
                    runStack.removeLast();
                }
            } else {
                mergeAt(data, a, b);
                a.length += b.length;
                runStack.removeLast();
            }
        }

        i += runLen;
        remaining -= runLen;
    }

    /* 归并栈中剩余的所有run */
    while (runStack.size() > 1) {
        int sz = runStack.size();
        mergeAt(data, runStack[sz - 2], runStack[sz - 1]);
        runStack[sz - 2].length += runStack[sz - 1].length;
        runStack.removeLast();
    }

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalSorted;
    m_elemSum += static_cast<quint64>(n);
    m_stats.totalElements = m_elemSum;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs =
        m_timeSum / static_cast<double>(m_stats.totalSorted);

    emit sortCompleted(n);
}

/**
 * @brief 返回排序后的索引
 * @param data 输入数据
 * @return 排序后的索引数组
 */
QVector<int> TimSort::sortIndices(const QVector<double>& data)
{
    int n = data.size();
    QVector<int> indices(n);
    for (int i = 0; i < n; ++i) indices[i] = i;

    /* 基于data值的索引排序 */
    std::stable_sort(indices.begin(), indices.end(),
        [&data](int a, int b) { return data[a] < data[b]; });

    ++m_stats.totalSorted;
    m_elemSum += static_cast<quint64>(n);
    m_stats.totalElements = m_elemSum;

    emit sortCompleted(n);
    return indices;
}

/**
 * @brief 检查数组是否已排序
 * @param data 输入数组
 * @return true 如果已升序排列
 */
bool TimSort::isSorted(const QVector<double>& data) const
{
    for (int i = 1; i < data.size(); ++i) {
        if (data[i] < data[i - 1]) return false;
    }
    return true;
}

/**
 * @brief 计算最小run长度
 * @param n 数组大小
 * @return 最小run长度
 *
 * 如果n < kMinRun则返回n，否则找到32~64之间
 * 最接近且n能被整除或略小于的值。
 */
int TimSort::calcMinRun(int n)
{
    int r = 0;
    while (n >= kMinRun) {
        r |= (n & 1);
        n >>= 1;
    }
    return n + r;
}

/**
 * @brief 二分插入排序
 * @param arr 数组
 * @param left 左边界(含)
 * @param right 右边界(不含)
 * @param cmp 比较器
 *
 * 对 [left, right) 范围执行二分插入排序，
 * 查找插入位置时用二分搜索减少比较次数。
 */
void TimSort::binaryInsertionSort(QVector<double>& arr, int left,
                                    int right, const Comparator& cmp)
{
    for (int i = left + 1; i < right; ++i) {
        double key = arr[i];

        /* 二分查找插入位置 */
        int lo = left, hi = i;
        while (lo < hi) {
            int mid = lo + (hi - lo) / 2;
            if (cmp(key, arr[mid])) {
                hi = mid;
            } else {
                lo = mid + 1;
            }
        }

        /* 移动元素 */
        for (int j = i; j > lo; --j) {
            arr[j] = arr[j - 1];
        }
        arr[lo] = key;
    }
}

/**
 * @brief 向右扩展run
 * @param arr 数组
 * @param start 起始位置
 * @param end 数组末尾
 * @param cmp 比较器
 * @return run的结束位置(不含)
 *
 * 从start开始向右扫描，找到最长的严格递增或非递减序列。
 */
int TimSort::extendRun(QVector<double>& arr, int start, int end,
                         const Comparator& cmp)
{
    if (start + 1 >= end) return end - start;

    int runEnd = start + 1;

    /* 判断递增还是递减 */
    if (cmp(arr[runEnd], arr[start])) {
        /* 严格递减run — 反转为递增 */
        while (runEnd < end && cmp(arr[runEnd], arr[runEnd - 1])) {
            ++runEnd;
        }
        /* 反转 */
        int lo = start, hi = runEnd - 1;
        while (lo < hi) {
            std::swap(arr[lo], arr[hi]);
            ++lo;
            --hi;
        }
    } else {
        /* 递增或相等 */
        while (runEnd < end && !cmp(arr[runEnd], arr[runEnd - 1])) {
            ++runEnd;
        }
    }

    return runEnd - start;
}

/**
 * @brief 归并两个相邻run(带Galloping模式)
 * @param arr 数组
 * @param runA 第一个run(左侧)
 * @param runB 第二个run(右侧)
 * @param cmp 比较器
 *
 * 使用临时缓冲区存储较小的run，然后用Galloping搜索
 * 确定归并范围，减少比较次数。
 */
void TimSort::mergeAt(QVector<double>& arr, const Run& runA,
                        const Run& runB, const Comparator& cmp)
{
    int start = runA.start;
    int mid = runA.start + runA.length;
    int end = runB.start + runB.length;

    /* Galloping搜索: 找到B的第一个元素在A中的位置 */
    int gallopA = gallopSearch(arr, arr[mid], start, runA.length, cmp, false);
    int gallopB = gallopSearch(arr, arr[mid - 1], mid, runB.length, cmp, true);

    /* 复制需要归并的A段到临时缓冲区 */
    int mergeStart = gallopA;
    int mergeALen = mid - gallopA;
    int mergeBStart = mid;
    int mergeBLen = gallopB - mid;

    if (mergeALen <= 0 || mergeBLen <= 0) return;

    QVector<double> temp(arr.begin() + mergeStart,
                         arr.begin() + mergeStart + mergeALen);

    /* 归并temp(B的左边)和B的右边部分 */
    int i = 0;            /* temp索引 */
    int j = mergeBStart;  /* B段索引 */
    int k = mergeStart;   /* 目标索引 */
    int tempEnd = temp.size();
    int bEnd = mergeBStart + mergeBLen;

    while (i < tempEnd && j < bEnd) {
        if (!cmp(arr[j], temp[i])) {
            arr[k++] = temp[i++];
        } else {
            arr[k++] = arr[j++];
        }
    }

    /* 拷贝剩余 */
    while (i < tempEnd) {
        arr[k++] = temp[i++];
    }
    while (j < bEnd) {
        arr[k++] = arr[j++];
    }
}

/**
 * @brief Galloping模式搜索
 * @param arr 数组
 * @param key 目标值
 * @param base 范围起始
 * @param len 范围长度
 * @param cmp 比较器
 * @param findMax true找上界(最后<=key的位置+1)，false找下界(第一个>=key的位置)
 * @return 插入位置
 *
 * 先以指数步长(1,3,7,15,...)扩大搜索范围，
 * 确定区间后再二分查找，适合一侧远大于另一侧的归并场景。
 */
int TimSort::gallopSearch(const QVector<double>& arr, double key,
                            int base, int len, const Comparator& cmp,
                            bool findMax) const
{
    if (len == 0) return base;

    if (!findMax) {
        /* 查找第一个 >= key 的位置(下界) */
        int lo = 0;
        int hi = len;
        int lastOffset = 0;
        int offset = 1;

        /* Galloping阶段: 指数扩大 */
        while (offset < hi) {
            if (!cmp(arr[base + offset], key)) {
                break;
            }
            lastOffset = offset;
            offset = (offset << 1) + 1;
        }

        if (offset > hi) offset = hi;

        /* 二分查找阶段 */
        lo = lastOffset;
        hi = offset;

        while (lo < hi) {
            int mid = lo + (hi - lo) / 2;
            if (cmp(arr[base + mid], key)) {
                lo = mid + 1;
            } else {
                hi = mid;
            }
        }
        return base + lo;

    } else {
        /* 查找最后一个 <= key 的位置+1(上界) */
        int lo = 0;
        int hi = len;
        int lastOffset = 0;
        int offset = 1;

        while (offset < hi) {
            if (cmp(key, arr[base + offset])) {
                break;
            }
            lastOffset = offset;
            offset = (offset << 1) + 1;
        }

        if (offset > hi) offset = hi;

        lo = lastOffset;
        hi = offset;

        while (lo < hi) {
            int mid = lo + (hi - lo) / 2;
            if (!cmp(key, arr[base + mid])) {
                lo = mid + 1;
            } else {
                hi = mid;
            }
        }
        return base + lo;
    }
}

/** @brief 重置统计 */
void TimSort::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_elemSum = 0;
}

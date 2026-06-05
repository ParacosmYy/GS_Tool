/**
 * @file DataReducer.cpp
 * @brief 数据降采样引擎实现 -- 高频数据降采样并保留关键特征
 *
 * 实现四种降采样算法:
 * - Decimation: 等距抽样，简单高效
 * - Average: 分桶均值，保留趋势
 * - MinMax: 分桶极值，保留范围
 * - LTTB: 最大三角形三桶算法，视觉最优
 *
 * 降采样完成后自动计算 RMSE 并更新统计。
 */

#include "utils/reductor/DataReducer.h"

#include <algorithm>
#include <cmath>

// ---- 构造 / 析构 ----

/** @brief 构造数据降采样引擎 @param parent 父对象 */
DataReducer::DataReducer(QObject *parent)
    : QObject(parent)
{
}

/** @brief 析构函数 */
DataReducer::~DataReducer() = default;

// ---- 配置 ----

/** @brief 设置降采样方法 @param method 降采样方法枚举 */
void DataReducer::setMethod(ReductionMethod method)
{
    m_method = method;
}

/** @brief 获取当前降采样方法 @return 方法枚举 */
DataReducer::ReductionMethod DataReducer::method() const
{
    return m_method;
}

/** @brief 设置目标输出点数，最小2 @param count 目标点数 */
void DataReducer::setTargetPoints(int count)
{
    m_targetPoints = qMax(2, count);
}

/** @brief 获取当前目标输出点数 @return 目标点数 */
int DataReducer::targetPoints() const
{
    return m_targetPoints;
}

// ---- 降采样 ----

/** @brief 对输入数据执行降采样，完成后发射 reductionComplete @param data 输入数据点列表 @return 降采样后的数据点列表 */
QList<DataReducer::DataPoint> DataReducer::reduce(const QList<DataPoint> &data)
{
    const int n = data.size();
    if (n <= m_targetPoints) {
        // 不需要降采样
        emit reductionComplete(n, n);
        return data;
    }

    QList<DataPoint> result;
    switch (m_method) {
    case ReductionMethod::Decimation:
        result = reduceDecimation(data, m_targetPoints);
        break;
    case ReductionMethod::Average:
        result = reduceAverage(data, m_targetPoints);
        break;
    case ReductionMethod::MinMax:
        result = reduceMinMax(data, m_targetPoints);
        break;
    case ReductionMethod::LTTB:
        result = reduceLTTB(data, m_targetPoints);
        break;
    }

    // 更新统计
    m_stats.totalPointsInput += n;
    m_stats.totalPointsOutput += result.size();
    if (m_stats.totalPointsInput > 0) {
        m_stats.reductionRatio = static_cast<double>(m_stats.totalPointsOutput)
                                 / static_cast<double>(m_stats.totalPointsInput);
    }

    // 计算误差
    double rmse = estimateError(data, result);
    if (rmse > m_stats.peakError) {
        m_stats.peakError = rmse;
    }
    m_errorSum += rmse;
    ++m_errorCount;
    m_stats.avgError = m_errorSum / static_cast<double>(m_errorCount);

    emit reductionComplete(n, result.size());
    return result;
}

// ---- 误差评估 ----

/** @brief 计算 RMSE 评估降采样质量 @param original 原始数据 @param reduced 降采样后数据 @return RMSE 值 */
double DataReducer::estimateError(const QList<DataPoint> &original,
                                  const QList<DataPoint> &reduced) const
{
    if (original.size() < 2 || reduced.size() < 2) {
        return 0.0;
    }

    double sumSq = 0.0;
    int count = 0;

    for (const auto &pt : original) {
        bool ok = false;
        double interpVal = linearInterpolate(reduced, pt.first, ok);
        if (!ok) {
            continue;
        }
        double diff = pt.second - interpVal;
        sumSq += diff * diff;
        ++count;
    }

    if (count == 0) {
        return 0.0;
    }
    return std::sqrt(sumSq / static_cast<double>(count));
}

// ---- 统计 ----

/** @brief 获取运行时统计快照 @return Stats 结构体 */
DataReducer::Stats DataReducer::stats() const
{
    return m_stats;
}

/** @brief 重置所有统计计数器 */
void DataReducer::resetStatistics()
{
    m_stats = Stats();
    m_errorSum = 0.0;
    m_errorCount = 0;
}

// ---- 私有: 降采样算法实现 ----

/** @brief 等距抽样: 每隔 N 个点保留一个 @param data 输入数据 @param target 目标点数 @return 降采样结果 */
QList<DataReducer::DataPoint> DataReducer::reduceDecimation(const QList<DataPoint> &data, int target)
{
    QList<DataPoint> result;
    const int n = data.size();
    if (n == 0 || target <= 0) {
        return result;
    }

    // 始终包含第一个和最后一个点
    result.reserve(target + 1);
    double step = static_cast<double>(n - 1) / static_cast<double>(target - 1);

    for (int i = 0; i < target; ++i) {
        int idx = static_cast<int>(std::round(i * step));
        idx = qMin(idx, n - 1);
        result.append(data[idx]);
    }
    return result;
}

/** @brief 均值降采样: 分桶后输出每个桶的平均值 @param data 输入数据 @param target 目标点数 @return 降采样结果 */
QList<DataReducer::DataPoint> DataReducer::reduceAverage(const QList<DataPoint> &data, int target)
{
    QList<DataPoint> result;
    const int n = data.size();
    if (n == 0 || target <= 0) {
        return result;
    }

    result.reserve(target);
    double bucketSize = static_cast<double>(n) / static_cast<double>(target);

    for (int i = 0; i < target; ++i) {
        int start = static_cast<int>(i * bucketSize);
        int end = static_cast<int>((i + 1) * bucketSize);
        end = qMin(end, n);

        if (start >= n) {
            break;
        }

        double sumVal = 0.0;
        qint64 sumTs = 0;
        int count = 0;
        for (int j = start; j < end; ++j) {
            sumVal += data[j].second;
            sumTs += data[j].first;
            ++count;
        }

        if (count > 0) {
            result.append(qMakePair(sumTs / count, sumVal / count));
        }
    }
    return result;
}

/** @brief 极值降采样: 每桶保留最小值和最大值 @param data 输入数据 @param target 目标桶数 @return 降采样结果(约 2*target 个点) */
QList<DataReducer::DataPoint> DataReducer::reduceMinMax(const QList<DataPoint> &data, int target)
{
    QList<DataPoint> result;
    const int n = data.size();
    if (n == 0 || target <= 0) {
        return result;
    }

    result.reserve(target * 2);
    double bucketSize = static_cast<double>(n) / static_cast<double>(target);

    for (int i = 0; i < target; ++i) {
        int start = static_cast<int>(i * bucketSize);
        int end = static_cast<int>((i + 1) * bucketSize);
        end = qMin(end, n);

        if (start >= n) {
            break;
        }

        double minVal = data[start].second;
        double maxVal = data[start].second;
        qint64 minTs = data[start].first;
        qint64 maxTs = data[start].first;

        for (int j = start + 1; j < end; ++j) {
            if (data[j].second < minVal) {
                minVal = data[j].second;
                minTs = data[j].first;
            }
            if (data[j].second > maxVal) {
                maxVal = data[j].second;
                maxTs = data[j].first;
            }
        }

        // 先输出较小时间戳的点，再输出较大时间戳的点
        if (minTs <= maxTs) {
            result.append(qMakePair(minTs, minVal));
            result.append(qMakePair(maxTs, maxVal));
        } else {
            result.append(qMakePair(maxTs, maxVal));
            result.append(qMakePair(minTs, minVal));
        }
    }
    return result;
}

/** @brief LTTB 最大三角形三桶算法: 视觉最优降采样 @param data 输入数据 @param target 目标点数 @return 降采样结果 */
QList<DataReducer::DataPoint> DataReducer::reduceLTTB(const QList<DataPoint> &data, int target)
{
    const int n = data.size();
    if (n <= target || target < 3) {
        return data;
    }

    QList<DataPoint> result;
    result.reserve(target);

    // 始终保留第一个点
    result.append(data.first());

    // 将数据分成 (target-2) 个桶，首尾点单独处理
    double bucketSize = static_cast<double>(n - 2) / static_cast<double>(target - 2);

    int prevSelectedIdx = 0;

    for (int i = 0; i < target - 2; ++i) {
        // 当前桶的范围
        int bucketStart = static_cast<int>(std::floor(1 + i * bucketSize));
        int bucketEnd = static_cast<int>(std::floor(1 + (i + 1) * bucketSize));
        bucketEnd = qMin(bucketEnd, n - 1);

        // 下一桶的平均值作为第三个点
        int nextBucketStart = static_cast<int>(std::floor(1 + (i + 1) * bucketSize));
        int nextBucketEnd = static_cast<int>(std::floor(1 + (i + 2) * bucketSize));
        nextBucketEnd = qMin(nextBucketEnd, n);

        double avgTs = 0.0;
        double avgVal = 0.0;
        int nextCount = 0;
        for (int j = nextBucketStart; j < nextBucketEnd && j < n; ++j) {
            avgTs += static_cast<double>(data[j].first);
            avgVal += data[j].second;
            ++nextCount;
        }
        if (nextCount > 0) {
            avgTs /= nextCount;
            avgVal /= nextCount;
        }

        // 在当前桶中找使三角形面积最大的点
        DataPoint prevPt = data[prevSelectedIdx];
        DataPoint nextPt = qMakePair(static_cast<qint64>(std::round(avgTs)), avgVal);

        double maxArea = -1.0;
        int maxIdx = bucketStart;
        for (int j = bucketStart; j < bucketEnd; ++j) {
            double area = triangleArea(prevPt, data[j], nextPt);
            if (area > maxArea) {
                maxArea = area;
                maxIdx = j;
            }
        }

        result.append(data[maxIdx]);
        prevSelectedIdx = maxIdx;
    }

    // 始终保留最后一个点
    result.append(data.last());
    return result;
}

// ---- 私有: 辅助方法 ----

/** @brief 计算三个数据点构成的三角形面积(用于LTTB算法) @param p1 第一个点 @param p2 第二个点(候选) @param p3 第三个点 @return 三角形面积绝对值 */
double DataReducer::triangleArea(const DataPoint &p1, const DataPoint &p2, const DataPoint &p3)
{
    // 使用向量叉积公式: |AB x AC| / 2
    // 为避免时间戳数值过大导致溢出，将时间戳归一化到 [0, 1] 区间
    // 但这里使用差值计算，数值稳定性已足够
    double x1 = static_cast<double>(p1.first);
    double y1 = p1.second;
    double x2 = static_cast<double>(p2.first);
    double y2 = p2.second;
    double x3 = static_cast<double>(p3.first);
    double y3 = p3.second;

    return std::abs((x1 - x3) * (y2 - y1) - (x1 - x2) * (y3 - y1)) / 2.0;
}

/** @brief 在 reduced 上对 targetTs 进行线性插值 @param reduced 降采样数据(时间戳升序) @param targetTs 目标时间戳 @param ok 是否能插值 @return 插值结果 */
double DataReducer::linearInterpolate(const QList<DataPoint> &reduced,
                                      qint64 targetTs, bool &ok)
{
    ok = false;
    if (reduced.size() < 2) {
        return 0.0;
    }

    // targetTs 在范围之前
    if (targetTs <= reduced.first().first) {
        ok = true;
        return reduced.first().second;
    }
    // targetTs 在范围之后
    if (targetTs >= reduced.last().first) {
        ok = true;
        return reduced.last().second;
    }

    // 二分查找 targetTs 所在区间
    int lo = 0;
    int hi = reduced.size() - 1;
    while (lo < hi - 1) {
        int mid = lo + (hi - lo) / 2;
        if (reduced[mid].first <= targetTs) {
            lo = mid;
        } else {
            hi = mid;
        }
    }

    qint64 t0 = reduced[lo].first;
    qint64 t1 = reduced[hi].first;
    double v0 = reduced[lo].second;
    double v1 = reduced[hi].second;

    if (t1 == t0) {
        ok = true;
        return v0;
    }

    ok = true;
    double ratio = static_cast<double>(targetTs - t0) / static_cast<double>(t1 - t0);
    return v0 + ratio * (v1 - v0);
}

/**
 * @file WindowedAggregator.cpp
 * @brief 滑动窗口统计聚合器实现 -- 基于时间窗口的实时统计计算
 *
 * 维护一个 QQueue<(timestamp, value)> 作为滑动窗口，
 * 每次 pushValue 时自动淘汰过期数据并重新计算完整统计。
 * 使用 QElapsedTimer 记录每次计算耗时，用于统计平均处理时间。
 */

#include "utils/aggregator2/WindowedAggregator.h"

#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <algorithm>
#include <cmath>

// ---- 构造 / 析构 ----

/** @brief 构造滑动窗口聚合器，启动计时器 @param parent 父对象 */
WindowedAggregator::WindowedAggregator(QObject *parent)
    : QObject(parent)
{
    m_elapsed.start();
}

/** @brief 析构函数 */
WindowedAggregator::~WindowedAggregator() = default;

// ---- 配置 ----

/** @brief 设置滑动窗口时长，最小100ms @param ms 窗口大小(毫秒) */
void WindowedAggregator::setWindowSizeMs(qint64 ms)
{
    m_windowSizeMs = qMax(100LL, ms);
}

/** @brief 获取当前窗口时长 @return 窗口大小(毫秒) */
qint64 WindowedAggregator::windowSizeMs() const
{
    return m_windowSizeMs;
}

// ---- 数据输入 ----

/** @brief 向窗口推入一个新值，淘汰过期数据后计算统计 @param value 新数据值 */
void WindowedAggregator::pushValue(double value)
{
    qint64 nowMs = QDateTime::currentMSecsSinceEpoch();

    // 淘汰过期数据
    evictExpired(nowMs);

    // 将新值推入窗口
    m_window.enqueue(qMakePair(nowMs, value));
    ++m_stats.totalValuesProcessed;

    // 计时: 统计计算耗时
    qint64 startNs = m_elapsed.nsecsElapsed();

    // 计算窗口统计
    QList<double> values;
    values.reserve(m_window.size());
    for (const auto &item : m_window) {
        values.append(item.second);
    }
    m_currentStats = computeStats(std::move(values));

    qint64 elapsedNs = m_elapsed.nsecsElapsed() - startNs;
    double elapsedUs = static_cast<double>(elapsedNs) / 1000.0;
    m_processingTimeSum += elapsedUs;

    // 更新统计
    ++m_stats.totalWindowsComputed;
    m_stats.peakWindowSize = qMax(m_stats.peakWindowSize, m_window.size());
    if (m_stats.totalWindowsComputed > 0) {
        m_stats.avgProcessingTimeUs = m_processingTimeSum
                                      / static_cast<double>(m_stats.totalWindowsComputed);
    }

    // 保存历史快照
    m_history.append(m_currentStats);
    if (m_history.size() > m_maxHistory) {
        m_history.removeFirst();
    }

    // 发射信号
    emit windowUpdated(m_currentStats);

    // 窗口为空时发射过期信号(通常不会在 push 后发生，但保留逻辑)
    if (m_window.isEmpty()) {
        emit windowExpired();
    }
}

// ---- 查询 ----

/** @brief 获取当前窗口统计快照 @return WindowStats 结构体 */
WindowedAggregator::WindowStats WindowedAggregator::currentStats() const
{
    return m_currentStats;
}

/** @brief 获取当前窗口内的样本数 @return 样本数 */
int WindowedAggregator::currentCount() const
{
    return m_window.size();
}

// ---- 重置与导出 ----

/** @brief 清空窗口缓冲区、历史快照和统计计数器 */
void WindowedAggregator::reset()
{
    m_window.clear();
    m_history.clear();
    m_currentStats = WindowStats();
    m_stats = Stats();
    m_processingTimeSum = 0.0;
}

/** @brief 将历史 WindowStats 导出为 CSV @param path 目标文件路径 @return 成功返回 true */
bool WindowedAggregator::exportHistory(const QString &path) const
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
    out << "index,mean,median,stddev,min,max,sum,count\n";

    for (int i = 0; i < m_history.size(); ++i) {
        const auto &ws = m_history[i];
        out << i << ","
            << QString::number(ws.mean, 'f', 6) << ","
            << QString::number(ws.median, 'f', 6) << ","
            << QString::number(ws.stddev, 'f', 6) << ","
            << QString::number(ws.min, 'f', 6) << ","
            << QString::number(ws.max, 'f', 6) << ","
            << QString::number(ws.sum, 'f', 6) << ","
            << ws.count << "\n";
    }

    file.close();
    return true;
}

// ---- 统计 ----

/** @brief 获取运行时统计快照 @return Stats 结构体 */
WindowedAggregator::Stats WindowedAggregator::stats() const
{
    return m_stats;
}

/** @brief 重置运行时统计计数器(不影响窗口数据) */
void WindowedAggregator::resetStatistics()
{
    m_stats = Stats();
    m_processingTimeSum = 0.0;
}

// ---- 私有方法 ----

/** @brief 淘汰窗口外的过期条目 @param nowMs 当前时间戳(ms) */
void WindowedAggregator::evictExpired(qint64 nowMs)
{
    qint64 cutoff = nowMs - m_windowSizeMs;
    while (!m_window.isEmpty() && m_window.head().first < cutoff) {
        m_window.dequeue();
    }

    // 如果窗口为空则通知
    if (m_window.isEmpty()) {
        emit windowExpired();
    }
}

/** @brief 对给定值列表计算完整统计(均值/中位数/标准差/极值/和) @param values 值列表(会被移动和排序) @return WindowStats 结构体 */
WindowedAggregator::WindowStats WindowedAggregator::computeStats(QList<double> values)
{
    WindowStats ws;
    if (values.isEmpty()) {
        return ws;
    }

    const int n = values.size();
    ws.count = n;

    // 排序用于中位数和极值
    std::sort(values.begin(), values.end());

    ws.min = values.first();
    ws.max = values.last();

    // 求和与均值
    double sum = 0.0;
    for (double v : values) {
        sum += v;
    }
    ws.sum = sum;
    ws.mean = sum / n;

    // 中位数
    if (n % 2 == 1) {
        ws.median = values[n / 2];
    } else {
        ws.median = (values[n / 2 - 1] + values[n / 2]) / 2.0;
    }

    // 标准差(总体标准差)
    if (n >= 2) {
        double variance = 0.0;
        for (double v : values) {
            double diff = v - ws.mean;
            variance += diff * diff;
        }
        ws.stddev = std::sqrt(variance / n);
    }

    return ws;
}

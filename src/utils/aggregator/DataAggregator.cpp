/**
 * @file DataAggregator.cpp
 * @brief 数据聚合器实现 — 多源滑动窗口聚合 + 时间窗口聚合
 *
 * 支持多数据源的独立窗口聚合运算(求和/均值/极值/计数/首尾值)，
 * 以及基于固定时间间隔的统计聚合(min/max/avg/count/sum)。
 * 时间窗口按自然时间对齐，支持1s/5s/30s/1min/5min预定义间隔。
 * 实时滚动聚合通过QTimer定期发射所有数据源的最新聚合结果。
 */

#include "utils/aggregator/DataAggregator.h"
#include <QtMath>
#include <QDateTime>
#include <algorithm>

// ---- 构造/析构 ----

/** @brief 构造数据聚合器，初始化滚动聚合定时器 @param parent 父对象 */
DataAggregator::DataAggregator(QObject *parent)
    : QObject(parent)
{
    connect(&m_rollingTimer, &QTimer::timeout, this, &DataAggregator::onRollingTimeout);
}

/** @brief 析构函数，停止定时器 */
DataAggregator::~DataAggregator()
{
    m_rollingTimer.stop();
}

// ---- 滑动窗口聚合(原有接口) ----

/** @brief 添加数据源并指定聚合函数和窗口大小 @param name 数据源名称 @param func 聚合函数类型 @param window 滑动窗口大小 */
void DataAggregator::addSource(const QString &name, AggregateFunc func, int window) {
    ++m_totalSourceAdds;
    m_sources[name] = {func, window, {}, 0.0};
    emit sourceAdded(name);
}

/** @brief 移除指定名称的数据源(滑动窗口+时间窗口一并移除) @param name 数据源名称 */
void DataAggregator::removeSource(const QString &name) {
    ++m_totalSourceRemoves;
    m_sources.remove(name);
    m_timeWindows.remove(name);
    emit sourceRemoved(name);
}

/** @brief 向指定数据源输入一个新值，同时触发滑动窗口和时间窗口聚合计算 @param source 数据源名称 @param value 新数据值 */
void DataAggregator::feedValue(const QString &source, double value) {
    ++m_totalValuesFed;
    // 滑动窗口聚合
    auto it = m_sources.find(source);
    if (it == m_sources.end()) return;
    it->values.append(value);
    if (it->values.size() > it->windowSize) it->values.removeFirst();
    computeAggregate(source);
    emit valueAggregated(source, it->result);

    // 时间窗口聚合(如果已启用)
    auto twIt = m_timeWindows.find(source);
    if (twIt != m_timeWindows.end()) {
        qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
        processTimeWindow(source, value, nowMs);
    }
}

/** @brief 获取指定数据源的聚合结果 @param source 数据源名称 @return 聚合结果值，不存在返回0.0 */
double DataAggregator::aggregateResult(const QString &source) const {
    auto it = m_sources.constFind(source);
    return it != m_sources.constEnd() ? it->result : 0.0;
}

/** @brief 获取所有数据源的聚合结果 @return 数据源名称到聚合结果的映射 */
QMap<QString, double> DataAggregator::allResults() const {
    QMap<QString, double> results;
    for (auto it = m_sources.constBegin(); it != m_sources.constEnd(); ++it)
        results[it.key()] = it->result;
    return results;
}

/** @brief 获取所有数据源名称列表 @return 数据源名称列表 */
QStringList DataAggregator::sources() const { return m_sources.keys(); }

/** @brief 设置指定数据源的滑动窗口大小 @param src 数据源名称 @param sz 新窗口大小 */
void DataAggregator::setWindowSize(const QString &src, int sz) {
    auto it = m_sources.find(src);
    if (it != m_sources.end()) it->windowSize = sz;
}

/** @brief 设置指定数据源的聚合函数 @param src 数据源名称 @param fn 聚合函数类型 */
void DataAggregator::setAggregateFunc(const QString &src, AggregateFunc fn) {
    auto it = m_sources.find(src);
    if (it != m_sources.end()) it->func = fn;
}

/** @brief 重置指定数据源的缓冲区和结果(滑动窗口+时间窗口一并重置) @param src 数据源名称 */
void DataAggregator::resetSource(const QString &src) {
    auto it = m_sources.find(src);
    if (it != m_sources.end()) { it->values.clear(); it->result = 0.0; }

    auto twIt = m_timeWindows.find(src);
    if (twIt != m_timeWindows.end()) {
        twIt->currentWindowStart = 0;
        twIt->currentValues.clear();
        twIt->lastCompleted = WindowStats();
        twIt->history.clear();
    }
}

/** @brief 重置所有数据源的缓冲区和结果 */
void DataAggregator::resetAll() {
    for (auto it = m_sources.begin(); it != m_sources.end(); ++it) {
        it->values.clear();
        it->result = 0.0;
    }
    for (auto twIt = m_timeWindows.begin(); twIt != m_timeWindows.end(); ++twIt) {
        twIt->currentWindowStart = 0;
        twIt->currentValues.clear();
        twIt->lastCompleted = WindowStats();
        twIt->history.clear();
    }
}

// ---- 时间窗口聚合 ----

/** @brief 为数据源启用时间窗口聚合，按自然时间对齐窗口边界 @param name 数据源名称(必须已通过addSource添加) @param intervalSec 时间窗口间隔(秒) */
void DataAggregator::enableTimeWindow(const QString &name, int intervalSec) {
    if (!m_sources.contains(name) || intervalSec <= 0) return;
    TimeWindowData twd;
    twd.intervalSec = intervalSec;
    twd.currentWindowStart = 0;
    m_timeWindows[name] = twd;
}

/** @brief 禁用指定数据源的时间窗口聚合 @param name 数据源名称 */
void DataAggregator::disableTimeWindow(const QString &name) {
    m_timeWindows.remove(name);
}

/** @brief 获取指定数据源最近一个已完成时间窗口的统计结果 @param name 数据源名称 @return 统计结果；若不存在或无已完成窗口，返回count=0的结构体 */
DataAggregator::WindowStats DataAggregator::lastWindowStats(const QString &name) const {
    auto it = m_timeWindows.constFind(name);
    if (it != m_timeWindows.constEnd()) return it->lastCompleted;
    return WindowStats();
}

/** @brief 获取指定数据源所有已完成时间窗口的统计结果列表(时间升序) @param name 数据源名称 @return 统计结果列表 */
QList<DataAggregator::WindowStats> DataAggregator::windowStatsHistory(const QString &name) const {
    auto it = m_timeWindows.constFind(name);
    if (it != m_timeWindows.constEnd()) return it->history;
    return {};
}

/** @brief 获取指定数据源当前正在进行的(尚未关闭的)时间窗口统计(实时更新) @param name 数据源名称 @return 当前窗口的实时统计 */
DataAggregator::WindowStats DataAggregator::currentWindowStats(const QString &name) const {
    auto it = m_timeWindows.constFind(name);
    if (it == m_timeWindows.constEnd() || it->currentValues.isEmpty())
        return WindowStats();

    WindowStats stats = computeStats(it->currentValues);
    stats.windowStartMs = it->currentWindowStart;
    stats.windowEndMs = it->currentWindowStart + static_cast<qint64>(it->intervalSec) * 1000;
    return stats;
}

/** @brief 设置时间窗口历史记录最大保留数量 @param max 最大保留数量 */
void DataAggregator::setMaxHistoryWindows(int max) {
    m_maxHistoryWindows = qMax(1, max);
}

/** @brief 获取时间窗口历史记录最大保留数量 @return 最大保留数量 */
int DataAggregator::maxHistoryWindows() const {
    return m_maxHistoryWindows;
}

// ---- 实时滚动聚合 ----

/** @brief 启用实时滚动聚合定时器，定期发射rollingAggregation信号 @param rollIntervalMs 滚动聚合间隔(毫秒) */
void DataAggregator::enableRollingAggregation(int rollIntervalMs) {
    m_rollingEnabled = true;
    m_rollingTimer.start(qMax(100, rollIntervalMs));
}

/** @brief 禁用实时滚动聚合定时器 */
void DataAggregator::disableRollingAggregation() {
    m_rollingEnabled = false;
    m_rollingTimer.stop();
}

/** @brief 查询实时滚动聚合是否已启用 @return true表示已启用 */
bool DataAggregator::isRollingEnabled() const {
    return m_rollingEnabled;
}

// ---- 私有方法 ----

/** @brief 对指定数据源执行滑动窗口聚合计算 @param source 数据源名称 */
void DataAggregator::computeAggregate(const QString &source) {
    auto it = m_sources.find(source);
    if (it == m_sources.end() || it->values.isEmpty()) return;
    const auto &vals = it->values;
    switch (it->func) {
    case Sum:     { double s=0; for (auto v:vals) s+=v; it->result=s; break; }
    case Average: { double s=0; for (auto v:vals) s+=v; it->result=s/vals.size(); break; }
    case Min:     { double m=vals[0]; for (auto v:vals) if (v<m) m=v; it->result=m; break; }
    case Max:     { double m=vals[0]; for (auto v:vals) if (v>m) m=v; it->result=m; break; }
    case Count:   it->result=vals.size(); break;
    case First:   it->result=vals.first(); break;
    case Last:    it->result=vals.last(); break;
    }
}

/** @brief 将一个新值纳入时间窗口统计，自动检测窗口边界并关闭过期窗口 @param source 数据源名称 @param value 新数据值 @param timestampMs 当前时间戳(Epoch毫秒) */
void DataAggregator::processTimeWindow(const QString &source, double value, qint64 timestampMs) {
    auto it = m_timeWindows.find(source);
    if (it == m_timeWindows.end()) return;

    qint64 windowStart = alignToWindow(timestampMs, it->intervalSec);

    // 首次收到数据或窗口已切换
    if (it->currentWindowStart == 0 || windowStart != it->currentWindowStart) {
        // 关闭上一个窗口(如果有数据)
        if (it->currentWindowStart != 0 && !it->currentValues.isEmpty()) {
            WindowStats completed = computeStats(it->currentValues);
            completed.windowStartMs = it->currentWindowStart;
            completed.windowEndMs = it->currentWindowStart
                                    + static_cast<qint64>(it->intervalSec) * 1000;
            it->lastCompleted = completed;
            it->history.append(completed);

            // 限制历史记录数量
            while (it->history.size() > m_maxHistoryWindows) {
                it->history.removeFirst();
            }
            emit windowClosed(source, completed);
            ++m_totalWindowsCompleted;
        }

        // 开启新窗口
        it->currentWindowStart = windowStart;
        it->currentValues.clear();
    }

    it->currentValues.append(value);
}

/** @brief 计算指定列表值的完整统计(min/max/avg/count/sum) @param values 值列表 @return 统计结果 */
DataAggregator::WindowStats DataAggregator::computeStats(const QList<double> &values) {
    WindowStats stats;
    if (values.isEmpty()) return stats;

    stats.count = values.size();
    double minVal = values.first();
    double maxVal = values.first();
    double sumVal = 0.0;
    for (double v : values) {
        if (v < minVal) minVal = v;
        if (v > maxVal) maxVal = v;
        sumVal += v;
    }
    stats.min = minVal;
    stats.max = maxVal;
    stats.sum = sumVal;
    stats.avg = sumVal / stats.count;
    return stats;
}

/** @brief 计算指定时间戳所属的时间窗口起始时间(自然对齐) @param timestampMs 时间戳(Epoch毫秒) @param intervalSec 窗口间隔(秒) @return 窗口起始时间(Epoch毫秒) */
qint64 DataAggregator::alignToWindow(qint64 timestampMs, int intervalSec) {
    qint64 intervalMs = static_cast<qint64>(intervalSec) * 1000;
    return (timestampMs / intervalMs) * intervalMs;
}

/** @brief 定时触发滚动聚合，发射所有数据源的最新聚合结果 */
void DataAggregator::onRollingTimeout() {
    if (m_sources.isEmpty()) return;
    ++m_totalRollingEmits;
    emit rollingAggregation(allResults());
}

// ============================================================================
// 统计接口
// ============================================================================

/** @brief 重置聚合器统计计数器 */
void DataAggregator::resetAggregatorStatistics() {
    m_totalValuesFed = 0;
    m_totalWindowsCompleted = 0;
    m_totalRollingEmits = 0;
    m_totalSourceAdds = 0;
    m_totalSourceRemoves = 0;
}

/**
 * @file DataAggregator.cpp
 * @brief 数据聚合器实现 — 多源滑动窗口聚合计算
 *
 * 支持多数据源的独立窗口聚合运算（求和/均值/极值/计数/首尾值），
 * 每个数据源可单独设置窗口大小和聚合函数。
 */

#include "utils/aggregator/DataAggregator.h"
#include <QtMath>
#include <algorithm>

/** @brief 构造数据聚合器 @param parent 父对象 */
DataAggregator::DataAggregator(QObject *parent) : QObject(parent) {}

/** @brief 析构函数 */
DataAggregator::~DataAggregator() = default;

/** @brief 添加数据源并指定聚合函数和窗口大小 @param name 数据源名称 @param func 聚合函数类型 @param window 滑动窗口大小 */
void DataAggregator::addSource(const QString &name, AggregateFunc func, int window) {
    m_sources[name] = {func, window, {}, 0.0};
    emit sourceAdded(name);
}

/** @brief 移除指定名称的数据源 @param name 数据源名称 */
void DataAggregator::removeSource(const QString &name) {
    m_sources.remove(name);
    emit sourceRemoved(name);
}

/** @brief 向指定数据源输入一个新值，触发聚合计算并发射结果 @param source 数据源名称 @param value 新数据值 */
void DataAggregator::feedValue(const QString &source, double value) {
    auto it = m_sources.find(source);
    if (it == m_sources.end()) return;
    it->values.append(value);
    if (it->values.size() > it->windowSize) it->values.removeFirst();
    computeAggregate(source);
    emit valueAggregated(source, it->result);
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

/** @brief 重置指定数据源的缓冲区和结果 @param src 数据源名称 */
void DataAggregator::resetSource(const QString &src) {
    auto it = m_sources.find(src);
    if (it != m_sources.end()) { it->values.clear(); it->result = 0.0; }
}

/** @brief 重置所有数据源的缓冲区和结果 */
void DataAggregator::resetAll() {
    for (auto it = m_sources.begin(); it != m_sources.end(); ++it) { it->values.clear(); it->result = 0.0; }
}

/** @brief 对指定数据源执行聚合计算（根据func类型选择Sum/Average/Min/Max/Count/First/Last） @param source 数据源名称 */
void DataAggregator::computeAggregate(const QString &source) {
    auto it = m_sources.find(source);
    if (it == m_sources.end() || it->values.isEmpty()) return;
    const auto &vals = it->values;
    switch (it->func) {
    case Sum: { double s=0; for (auto v:vals) s+=v; it->result=s; break; }
    case Average: { double s=0; for (auto v:vals) s+=v; it->result=s/vals.size(); break; }
    case Min: { double m=vals[0]; for (auto v:vals) if (v<m) m=v; it->result=m; break; }
    case Max: { double m=vals[0]; for (auto v:vals) if (v>m) m=v; it->result=m; break; }
    case Count: it->result=vals.size(); break;
    case First: it->result=vals.first(); break;
    case Last: it->result=vals.last(); break;
    }
}

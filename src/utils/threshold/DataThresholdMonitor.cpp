/**
 * @file DataThresholdMonitor.cpp
 * @brief 数据阈值监控组件实现 — 越界检测与告警
 */

#include "utils/threshold/DataThresholdMonitor.h"

#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
DataThresholdMonitor::DataThresholdMonitor(QObject* parent)
    : QObject(parent)
    , m_alertEnabled(true)
{
}

/** @brief 添加阈值规则 @param name 规则名 @param min 最小值 @param max 最大值 */
void DataThresholdMonitor::addRule(const QString& name, double min, double max)
{
    ThresholdRule rule;
    rule.name     = name;
    rule.minValue = min;
    rule.maxValue = max;
    rule.enabled  = true;
    rule.isPercentage   = false;
    rule.referenceValue = 0.0;
    m_rules.insert(name, rule);
    m_stats.activeRules = static_cast<int>(
        std::count_if(m_rules.cbegin(), m_rules.cend(),
                      [](const ThresholdRule& r) { return r.enabled; }));
}

/** @brief 移除阈值规则 @param name 规则名 */
void DataThresholdMonitor::removeRule(const QString& name)
{
    m_rules.remove(name);
    m_stats.violationsByRule.remove(name);
    m_stats.activeRules = static_cast<int>(
        std::count_if(m_rules.cbegin(), m_rules.cend(),
                      [](const ThresholdRule& r) { return r.enabled; }));
}

/** @brief 检查单个值是否越界 @param ruleName 规则名 @param value 待检值 @return 阈值状态 */
DataThresholdMonitor::ThresholdState DataThresholdMonitor::checkValue(
    const QString& ruleName, double value)
{
    ++m_stats.totalChecks;

    /* 更新峰值/谷值 */
    if (m_stats.totalChecks == 1) {
        m_stats.peakValue   = value;
        m_stats.lowestValue = value;
    } else {
        if (value > m_stats.peakValue)   m_stats.peakValue   = value;
        if (value < m_stats.lowestValue) m_stats.lowestValue = value;
    }

    auto it = m_rules.find(ruleName);
    if (it == m_rules.end()) return ThresholdState::Normal;

    ThresholdRule& rule = it.value();
    if (!rule.enabled) return ThresholdState::Normal;

    double effMin = effectiveMin(rule);
    double effMax = effectiveMax(rule);

    if (value < effMin) {
        ++m_stats.totalViolations;
        ++m_stats.violationsByRule[ruleName];
        if (m_alertEnabled) {
            emit thresholdViolated(ruleName, value,
                                   tr("below"));
        }
        return ThresholdState::BelowMin;
    }
    if (value > effMax) {
        ++m_stats.totalViolations;
        ++m_stats.violationsByRule[ruleName];
        if (m_alertEnabled) {
            emit thresholdViolated(ruleName, value,
                                   tr("above"));
        }
        return ThresholdState::AboveMax;
    }

    if (m_alertEnabled) {
        emit valueInRange(ruleName);
    }
    return ThresholdState::Normal;
}

/** @brief 用所有规则检查一组值 @param values 规则名→值的映射 @return 规则名→状态映射 */
QMap<QString, DataThresholdMonitor::ThresholdState> DataThresholdMonitor::checkAll(
    const QMap<QString, double>& values)
{
    QMap<QString, ThresholdState> results;
    for (auto it = values.cbegin(); it != values.cend(); ++it) {
        results.insert(it.key(), checkValue(it.key(), it.value()));
    }
    return results;
}

/** @brief 设置规则为百分比模式 @param name 规则名 @param refValue 参考基准值 */
void DataThresholdMonitor::setRulePercentage(const QString& name, double refValue)
{
    auto it = m_rules.find(name);
    if (it != m_rules.end()) {
        it.value().isPercentage   = true;
        it.value().referenceValue = refValue;
    }
}

/** @brief 启用/禁用规则 @param name 规则名 @param enabled 是否启用 */
void DataThresholdMonitor::setRuleEnabled(const QString& name, bool enabled)
{
    auto it = m_rules.find(name);
    if (it != m_rules.end()) {
        it.value().enabled = enabled;
        m_stats.activeRules = static_cast<int>(
            std::count_if(m_rules.cbegin(), m_rules.cend(),
                          [](const ThresholdRule& r) { return r.enabled; }));
    }
}

/** @brief 全局告警开关 @param enabled 是否启用告警信号 */
void DataThresholdMonitor::setAlertEnabled(bool enabled)
{
    m_alertEnabled = enabled;
}

/** @brief 获取所有规则 @return 规则列表 */
QList<DataThresholdMonitor::ThresholdRule> DataThresholdMonitor::rules() const
{
    return m_rules.values();
}

/** @brief 重置所有统计计数器 */
void DataThresholdMonitor::resetStatistics()
{
    m_stats = Stats{};
    m_stats.activeRules = static_cast<int>(
        std::count_if(m_rules.cbegin(), m_rules.cend(),
                      [](const ThresholdRule& r) { return r.enabled; }));
}

/* ── 私有方法 ── */

/** @brief 计算规则的有效最小值(支持百分比模式) @param rule 规则 @return 有效最小值 */
double DataThresholdMonitor::effectiveMin(const ThresholdRule& rule) const
{
    if (rule.isPercentage && rule.referenceValue != 0.0) {
        return rule.referenceValue * (1.0 - rule.minValue / 100.0);
    }
    return rule.minValue;
}

/** @brief 计算规则的有效最大值(支持百分比模式) @param rule 规则 @return 有效最大值 */
double DataThresholdMonitor::effectiveMax(const ThresholdRule& rule) const
{
    if (rule.isPercentage && rule.referenceValue != 0.0) {
        return rule.referenceValue * (1.0 + rule.maxValue / 100.0);
    }
    return rule.maxValue;
}

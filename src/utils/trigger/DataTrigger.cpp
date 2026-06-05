/**
 * @file DataTrigger.cpp
 * @brief 数据触发器实现
 */

#include "utils/trigger/DataTrigger.h"
#include <QtMath>

DataTrigger::DataTrigger(QObject* parent)
    : QObject(parent), m_prevValue(0.0) {}

void DataTrigger::addRule(const TriggerRule& rule)
{
    m_rules[rule.name] = rule;
}

void DataTrigger::removeRule(const QString& name)
{
    m_rules.remove(name);
}

void DataTrigger::evaluate(double value)
{
    ++m_stats.totalEvaluations;
    if (qAbs(value) > m_stats.peakValue) m_stats.peakValue = qAbs(value);

    for (auto it = m_rules.begin(); it != m_rules.end(); ++it) {
        auto& rule = it.value();
        if (!rule.enabled) continue;
        if (rule.oneShot && rule.fired) continue;

        if (checkRule(rule, value)) {
            rule.fired = true;
            ++m_stats.totalFirings;
            ++m_stats.firingsByRule[rule.name];
            emit triggered(rule.name, value);
        }
    }
    m_prevValue = value;
}

void DataTrigger::evaluatePattern(const QByteArray& data)
{
    for (auto it = m_rules.begin(); it != m_rules.end(); ++it) {
        auto& rule = it.value();
        if (!rule.enabled || rule.condition != TriggerCondition::PatternMatch) continue;
        if (rule.oneShot && rule.fired) continue;

        QByteArray pattern;
        pattern.append(static_cast<char>(static_cast<int>(rule.param1)));
        if (data.contains(pattern)) {
            rule.fired = true;
            ++m_stats.totalFirings;
            ++m_stats.firingsByRule[rule.name];
            emit triggered(rule.name, 0.0);
        }
    }
}

void DataTrigger::resetAllFired()
{
    for (auto it = m_rules.begin(); it != m_rules.end(); ++it) {
        it.value().fired = false;
    }
}

bool DataTrigger::checkRule(const TriggerRule& rule, double value)
{
    switch (rule.condition) {
    case TriggerCondition::AboveThreshold:
        return value > rule.param1;
    case TriggerCondition::BelowThreshold:
        return value < rule.param1;
    case TriggerCondition::InRange:
        return value >= rule.param1 && value <= rule.param2;
    case TriggerCondition::OutOfRange:
        return value < rule.param1 || value > rule.param2;
    case TriggerCondition::RateOfChange: {
        double rate = qAbs(value - m_prevValue);
        return rate > rule.param1;
    }
    case TriggerCondition::PatternMatch:
        return false; /* 由evaluatePattern处理 */
    }
    return false;
}

void DataTrigger::resetStatistics()
{
    m_stats = Stats{};
}

/**
 * @file SerialSimulator.cpp
 * @brief 串口设备模拟器 — 核心交互/规则匹配/统计
 *
 * 响应生成/错误注入/设备配置见 SerialSimulatorProfiles.cpp。
 */

#include "serial/simulator/SerialSimulator.h"

#include <QRandomGenerator>
#include <QThread>
#include <QJsonArray>
#include <algorithm>

// ═══════════════════════════════════════════════════════════
// 构造 / 生命周期
// ═══════════════════════════════════════════════════════════

SerialSimulator::SerialSimulator(QObject* parent)
    : QObject(parent)
    , m_stats{}
{
    setObjectName(QStringLiteral("SerialSimulator"));
    m_timer.start();
}

// ═══════════════════════════════════════════════════════════
// 响应规则管理
// ═══════════════════════════════════════════════════════════

void SerialSimulator::addRule(const QByteArray& pattern, const QByteArray& response)
{
    ResponseRule rule;
    rule.pattern = pattern;
    rule.response = response;
    rule.isWildcard = pattern.contains('*');
    rule.matchLength = pattern.length();
    m_rules.append(rule);
}

void SerialSimulator::clearRules() { m_rules.clear(); }

// ═══════════════════════════════════════════════════════════
// 延迟 / 错误配置
// ═══════════════════════════════════════════════════════════

void SerialSimulator::setResponseDelay(int minMs, int maxMs)
{
    m_minDelayMs = qMax(0, minMs);
    m_maxDelayMs = qMax(m_minDelayMs, maxMs);
}

void SerialSimulator::setErrorRate(double bitErrorRate, double dropRate)
{
    m_bitErrorRate = qBound(0.0, bitErrorRate, 1.0);
    m_dropRate = qBound(0.0, dropRate, 1.0);
}

// ═══════════════════════════════════════════════════════════
// 设备配置 / 脚本
// ═══════════════════════════════════════════════════════════

void SerialSimulator::loadProfile(SimulatorProfile profile)
{
    clearRules();
    m_counter = 0;
    m_currentProfile = profile;
    switch (profile) {
    case SimulatorProfile::Gps:       setupGpsProfile();       break;
    case SimulatorProfile::ModbusSlave: setupModbusSlaveProfile(); break;
    case SimulatorProfile::SensorArray: setupSensorArrayProfile(); break;
    case SimulatorProfile::Echo:      break;
    case SimulatorProfile::Custom:
    default: break;
    }
}

SimulatorProfile SerialSimulator::currentProfile() const { return m_currentProfile; }

bool SerialSimulator::loadScript(const QJsonObject& script)
{
    if (script.contains("delay")) {
        const QJsonObject delay = script["delay"].toObject();
        setResponseDelay(delay.value("min").toInt(5), delay.value("max").toInt(20));
    }
    if (!script.contains("rules")) return false;
    const QJsonArray rules = script["rules"].toArray();
    if (rules.isEmpty()) return false;
    clearRules();
    m_currentProfile = SimulatorProfile::Custom;
    for (const QJsonValue& val : rules) {
        const QJsonObject ruleObj = val.toObject();
        const QByteArray pattern = ruleObj.value("pattern").toString().toUtf8();
        const QByteArray response = ruleObj.value("response").toString().toUtf8();
        if (!pattern.isEmpty()) addRule(pattern, response);
    }
    return true;
}

// ═══════════════════════════════════════════════════════════
// 核心交互
// ═══════════════════════════════════════════════════════════

QByteArray SerialSimulator::feed(const QByteArray& input)
{
    m_stats.totalRequestsProcessed++;
    const int profileIdx = static_cast<int>(m_currentProfile);
    if (profileIdx >= 0 && profileIdx < 5) m_stats.requestsByProfile[profileIdx]++;

    QByteArray response;
    if (m_currentProfile == SimulatorProfile::Echo) {
        response = input;
    } else {
        const ResponseRule* rule = findMatchingRule(input);
        response = rule ? generateResponse(rule->response)
                        : QByteArrayLiteral("\x15");
    }

    const int delayRange = m_maxDelayMs - m_minDelayMs;
    const int delay = m_minDelayMs + (delayRange > 0
                       ? static_cast<int>(QRandomGenerator::global()->generateDouble() * delayRange)
                       : 0);
    if (delay > 0) QThread::msleep(static_cast<unsigned long>(delay));

    m_delayCount++;
    m_totalDelayAccum += delay;
    m_stats.avgResponseDelayMs = static_cast<double>(m_totalDelayAccum)
                                 / static_cast<double>(m_delayCount);

    injectErrors(response);
    m_stats.totalResponsesGenerated++;
    emit responseGenerated(response);
    return response;
}

// ═══════════════════════════════════════════════════════════
// 统计
// ═══════════════════════════════════════════════════════════

const SimulatorStats& SerialSimulator::stats() const { return m_stats; }

void SerialSimulator::resetStatistics()
{
    m_stats = SimulatorStats{};
    m_totalDelayAccum = 0;
    m_delayCount = 0;
    m_counter = 0;
}

// ═══════════════════════════════════════════════════════════
// 内部: 规则匹配
// ═══════════════════════════════════════════════════════════

const ResponseRule* SerialSimulator::findMatchingRule(const QByteArray& input) const
{
    /* 精确匹配优先 */
    for (const ResponseRule& rule : m_rules) {
        if (!rule.isWildcard && input == rule.pattern) return &rule;
    }
    /* 通配符匹配 */
    for (const ResponseRule& rule : m_rules) {
        if (!rule.isWildcard) continue;
        const QByteArray& pat = rule.pattern;
        const int starIdx = pat.indexOf('*');
        if (starIdx == 0) {
            if (input.endsWith(pat.mid(1))) return &rule;
        } else if (starIdx == pat.length() - 1) {
            if (input.startsWith(pat.left(starIdx))) return &rule;
        } else if (starIdx > 0) {
            const QByteArray prefix = pat.left(starIdx);
            const QByteArray suffix = pat.mid(starIdx + 1);
            if (input.startsWith(prefix) && input.endsWith(suffix)
                && input.length() >= prefix.length() + suffix.length()) return &rule;
        }
    }
    return nullptr;
}

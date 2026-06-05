/**
 * @file DeviceSimulator.cpp
 * @brief 设备模拟器核心引擎实现 -- 命令匹配、响应生成、延迟调度
 */
#include "utils/simulator/DeviceSimulator.h"

#include <QTimer>
#include <QRandomGenerator>

// ── 构造/析构 ──

DeviceSimulator::DeviceSimulator(QObject* parent)
    : QObject(parent)
    , m_rng(QRandomGenerator::global()->generate())
{
    m_timer.start();
}

// ── 规则管理 ──

void DeviceSimulator::addResponse(const SimResponse& rule)
{
    m_rules.append(rule);
    m_scriptCursors.append(0);
}

void DeviceSimulator::removeResponse(int index)
{
    if (index < 0 || index >= m_rules.size()) return;
    m_rules.removeAt(index);
    m_scriptCursors.removeAt(index);
}

void DeviceSimulator::setResponse(int index, const SimResponse& rule)
{
    if (index < 0 || index >= m_rules.size()) return;
    m_rules[index] = rule;
}

const QVector<SimResponse>& DeviceSimulator::responses() const
{
    return m_rules;
}

void DeviceSimulator::clearResponses()
{
    m_rules.clear();
    m_scriptCursors.clear();
}

// ── 全局配置 ──

void DeviceSimulator::setAutoEcho(bool enable) { m_autoEcho = enable; }
bool DeviceSimulator::autoEcho() const { return m_autoEcho; }

void DeviceSimulator::setNoiseRate(double rate) { m_noiseRate = qBound(0.0, rate, 1.0); }
double DeviceSimulator::noiseRate() const { return m_noiseRate; }

void DeviceSimulator::setDefaultDelay(int minMs, int maxMs)
{
    m_defaultMinDelayMs = qMax(0, minMs);
    m_defaultMaxDelayMs = qMax(m_defaultMinDelayMs, maxMs);
}

// ── 核心交互 ──

void DeviceSimulator::feedData(const QByteArray& data)
{
    emit commandReceived(data);
    m_stats.commandsReceived++;
    m_stats.totalBytesReceived += static_cast<quint64>(data.size());

    // 自动回显模式
    if (m_autoEcho) {
        m_stats.echoCount++;
        QByteArray echo = data;
        if (m_noiseRate > 0.0) echo = injectNoise(echo);
        m_stats.responsesSent++;
        m_stats.totalBytesSent += static_cast<quint64>(echo.size());
        emit responseSent(echo);
    }

    // 规则匹配
    int ruleIdx = findMatchingRule(data);
    if (ruleIdx < 0) {
        m_stats.commandsUnmatched++;
        return;
    }

    const SimResponse& rule = m_rules[ruleIdx];
    if (!rule.enabled) {
        m_stats.commandsUnmatched++;
        return;
    }

    QByteArray response = generateResponse(rule, ruleIdx);
    if (m_noiseRate > 0.0) response = injectNoise(response);

    int delay = calcDelay(rule);

    // 使用 QTimer::singleShot 模拟延迟后发送
    QTimer::singleShot(delay, this, [this, response]() {
        m_stats.responsesSent++;
        m_stats.totalBytesSent += static_cast<quint64>(response.size());
        qint64 elapsed = m_timer.elapsed();
        m_delayAccum += elapsed;
        m_delayCount++;
        if (m_delayCount > 0)
            m_stats.avgResponseDelayMs = static_cast<double>(m_delayAccum) / m_delayCount;
        emit responseSent(response);
    });
}

// ── 统计 ──

const SimulatorRunStats& DeviceSimulator::stats() const { return m_stats; }

void DeviceSimulator::resetStats()
{
    m_stats = SimulatorRunStats{};
    m_delayAccum = 0;
    m_delayCount = 0;
}

// ── 私有方法 ──

int DeviceSimulator::findMatchingRule(const QByteArray& input) const
{
    // 优先级: Exact > Prefix > Regex
    // 第一轮: 精确匹配
    for (int i = 0; i < m_rules.size(); ++i) {
        const SimResponse& r = m_rules[i];
        if (!r.enabled || r.strategy != MatchStrategy::Exact) continue;
        if (input == r.commandPattern) return i;
    }
    // 第二轮: 前缀匹配
    for (int i = 0; i < m_rules.size(); ++i) {
        const SimResponse& r = m_rules[i];
        if (!r.enabled || r.strategy != MatchStrategy::Prefix) continue;
        if (input.startsWith(r.commandPattern)) return i;
    }
    // 第三轮: 正则匹配
    for (int i = 0; i < m_rules.size(); ++i) {
        const SimResponse& r = m_rules[i];
        if (!r.enabled || r.strategy != MatchStrategy::Regex) continue;
        QRegularExpression re(QString::fromUtf8(r.commandPattern));
        if (re.isValid()) {
            QRegularExpressionMatch m = re.match(QString::fromUtf8(input));
            if (m.hasMatch()) return i;
        }
    }
    return -1;
}

QByteArray DeviceSimulator::generateResponse(const SimResponse& rule, int ruleIndex)
{
    switch (rule.mode) {
    case ResponseMode::Fixed:
        return rule.fixedData;

    case ResponseMode::Incremental: {
        QByteArray resp = rule.fixedData;
        if (!resp.isEmpty()) {
            char last = resp[resp.size() - 1];
            resp[resp.size() - 1] = static_cast<char>(static_cast<int>(last) + 1);
        }
        // 注意: 不修改 m_rules 中的 fixedData，避免累积副作用
        // 每次都基于原始 fixedData 递增 m_scriptCursors 作为计数器
        m_scriptCursors[ruleIndex]++;
        resp[resp.size() - 1] = static_cast<char>(
            (static_cast<int>(rule.fixedData[rule.fixedData.size() - 1])
             + m_scriptCursors[ruleIndex]) & 0xFF);
        return resp;
    }

    case ResponseMode::Random:
        return generateRandomBytes(rule.randomMinLen, rule.randomMaxLen);

    case ResponseMode::Scripted: {
        if (rule.scriptedResponses.isEmpty()) return QByteArray();
        int& cursor = m_scriptCursors[ruleIndex];
        QByteArray resp = rule.scriptedResponses[cursor];
        cursor = (cursor + 1) % rule.scriptedResponses.size();
        return resp;
    }
    }
    return QByteArray();
}

QByteArray DeviceSimulator::generateRandomBytes(int minLen, int maxLen) const
{
    int len = (minLen == maxLen) ? minLen
              : minLen + static_cast<int>(m_rng.bounded(maxLen - minLen + 1));
    QByteArray buf(len, Qt::Uninitialized);
    for (int i = 0; i < len; ++i)
        buf[i] = static_cast<char>(m_rng.bounded(256));
    return buf;
}

QByteArray DeviceSimulator::injectNoise(const QByteArray& data)
{
    if (m_rng.generateDouble() >= m_noiseRate) return data;
    m_stats.noiseCount++;
    QByteArray noisy = data;
    int noiseLen = 1 + static_cast<int>(m_rng.bounded(4));
    for (int i = 0; i < noiseLen; ++i)
        noisy.append(static_cast<char>(m_rng.bounded(256)));
    return noisy;
}

int DeviceSimulator::calcDelay(const SimResponse& rule) const
{
    int lo = (rule.minDelayMs >= 0) ? rule.minDelayMs : m_defaultMinDelayMs;
    int hi = (rule.maxDelayMs >= 0) ? rule.maxDelayMs : m_defaultMaxDelayMs;
    if (lo >= hi) return lo;
    return lo + static_cast<int>(m_rng.bounded(hi - lo + 1));
}

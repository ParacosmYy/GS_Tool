/**
 * @file PortBridge.cpp
 * @brief 多端口桥接引擎实现 — 规则管理、数据转发与过滤器评估
 */

#include "connection/bridge/PortBridge.h"

#include <QRegularExpression>
#include <algorithm>

// ============================================================
// 构造 / 析构
// ============================================================

/** @brief 构造桥接引擎 @param parent 父对象 */
PortBridge::PortBridge(QObject* parent)
    : QObject(parent)
{
}

// ============================================================
// 规则管理
// ============================================================

/**
 * @brief 添加一条桥接规则
 * @param rule 规则配置（name 字段作为唯一键，不可为空且不可重复）
 * @return true=添加成功，false=名称为空或已存在
 */
bool PortBridge::addBridge(const BridgeRule& rule)
{
    if (rule.name.isEmpty()) {
        return false;
    }
    if (findRuleIndex(rule.name) >= 0) {
        return false;
    }
    m_rules.append(rule);
    m_ruleStats.insert(rule.name, BridgeStats{});
    return true;
}

/**
 * @brief 按规则名称移除桥接规则
 * @param ruleName 规则名称
 * @return true=移除成功，false=规则不存在
 */
bool PortBridge::removeBridge(const QString& ruleName)
{
    const int idx = findRuleIndex(ruleName);
    if (idx < 0) {
        return false;
    }
    m_rules.removeAt(idx);
    m_ruleStats.remove(ruleName);
    return true;
}

/** @brief 清空所有桥接规则并重置统计计数器 */
void PortBridge::clearBridges()
{
    m_rules.clear();
    m_ruleStats.clear();
}

/** @brief 获取所有桥接规则的副本 @return 规则列表 */
QVector<BridgeRule> PortBridge::bridges() const
{
    return m_rules;
}

/**
 * @brief 启用指定名称的桥接规则
 * @param ruleName 规则名称
 * @return true=成功，false=规则不存在
 */
bool PortBridge::enableBridge(const QString& ruleName)
{
    const int idx = findRuleIndex(ruleName);
    if (idx < 0) {
        return false;
    }
    m_rules[idx].enabled = true;
    return true;
}

/**
 * @brief 禁用指定名称的桥接规则
 * @param ruleName 规则名称
 * @return true=成功，false=规则不存在
 */
bool PortBridge::disableBridge(const QString& ruleName)
{
    const int idx = findRuleIndex(ruleName);
    if (idx < 0) {
        return false;
    }
    m_rules[idx].enabled = false;
    return true;
}

// ============================================================
// 数据转发
// ============================================================

/**
 * @brief 向桥接引擎注入数据
 * @param sourceId 数据来源连接的唯一标识
 * @param data 接收到的原始字节数据
 *
 * 遍历所有已启用的规则。对于每条规则，根据 BridgeDirection
 * 判断 sourceId 是否匹配规则的源端或目标端，然后调用
 * evaluateAndForward() 执行过滤与转发。
 */
void PortBridge::feedData(const QString& sourceId, const QByteArray& data)
{
    for (int i = 0; i < m_rules.size(); ++i) {
        const BridgeRule& rule = m_rules.at(i);
        if (!rule.enabled) {
            continue;
        }

        switch (rule.direction) {
        case BridgeDirection::Forward:
            /* 仅 source→target */
            if (rule.sourceId == sourceId) {
                evaluateAndForward(i, rule.sourceId, rule.targetId, data);
            }
            break;

        case BridgeDirection::Backward:
            /* 仅 target→source */
            if (rule.targetId == sourceId) {
                evaluateAndForward(i, rule.targetId, rule.sourceId, data);
            }
            break;

        case BridgeDirection::Bidirectional:
            /* 双向：匹配哪端就向对端转发 */
            if (rule.sourceId == sourceId) {
                evaluateAndForward(i, rule.sourceId, rule.targetId, data);
            } else if (rule.targetId == sourceId) {
                evaluateAndForward(i, rule.targetId, rule.sourceId, data);
            }
            break;
        }
    }
}

// ============================================================
// 过滤评估
// ============================================================

/**
 * @brief 评估数据是否通过过滤器链（AND 逻辑）
 * @param data 待检查的数据
 * @param filters 过滤器列表
 * @return true=通过所有过滤器，false=被拦截
 *
 * 对每个过滤器：
 *   - None: 始终通过
 *   - Prefix: 数据以 pattern 的 UTF-8 编码开头
 *   - Regex: 数据中存在匹配正则的子串
 *   - Length: pattern 格式为 "min-max"，数据长度在闭区间内
 *
 * inclusive=false 时对单条过滤结果取反（黑名单模式）。
 */
bool PortBridge::passesFilter(const QByteArray& data,
                               const QVector<BridgeFilter>& filters) const
{
    for (const BridgeFilter& filter : filters) {
        bool match = false;

        switch (filter.type) {
        case BridgeFilterType::None:
            match = true;
            break;

        case BridgeFilterType::Prefix: {
            const QByteArray prefix = filter.pattern.toUtf8();
            match = data.startsWith(prefix);
            break;
        }

        case BridgeFilterType::Regex: {
            const QRegularExpression re(filter.pattern);
            if (!re.isValid()) {
                /* 正则无效视为不匹配 */
                match = false;
            } else {
                match = re.match(QString::fromUtf8(data)).hasMatch();
            }
            break;
        }

        case BridgeFilterType::Length: {
            /* 解析 "min-max" 格式 */
            const QStringList parts = filter.pattern.split(QLatin1Char('-'));
            if (parts.size() == 2) {
                bool okMin = false;
                bool okMax = false;
                const int minLen = parts[0].toInt(&okMin);
                const int maxLen = parts[1].toInt(&okMax);
                if (okMin && okMax) {
                    match = (data.size() >= minLen && data.size() <= maxLen);
                }
            }
            break;
        }
        }

        /* 黑名单：inclusive=false 时取反 */
        const bool passed = filter.inclusive ? match : !match;
        if (!passed) {
            return false;
        }
    }
    return true;
}

// ============================================================
// 私有方法
// ============================================================

/**
 * @brief 查找规则名称在列表中的索引
 * @param ruleName 规则名称
 * @return 索引，不存在返回 -1
 */
int PortBridge::findRuleIndex(const QString& ruleName) const
{
    for (int i = 0; i < m_rules.size(); ++i) {
        if (m_rules.at(i).name == ruleName) {
            return i;
        }
    }
    return -1;
}

/**
 * @brief 对单条规则执行方向确定后的过滤与转发
 * @param ruleIdx 规则索引
 * @param fromId 实际数据来源 ID
 * @param toId 转发目标 ID
 * @param data 待转发数据
 *
 * 先评估过滤器链。通过则更新统计并发射 dataForwarded 信号；
 * 被拦截则更新 bytesFiltered 并发射 dataFiltered 信号；
 * 过程中发生异常则更新 errors 并发射 bridgeError 信号。
 */
void PortBridge::evaluateAndForward(int ruleIdx, const QString& fromId,
                                     const QString& toId, const QByteArray& data)
{
    const BridgeRule& rule = m_rules.at(ruleIdx);
    BridgeStats& stats = m_ruleStats[rule.name];

    try {
        if (passesFilter(data, rule.filters)) {
            stats.bytesForwarded += static_cast<quint64>(data.size());
            ++stats.packetsForwarded;
            emit dataForwarded(fromId, toId, data.size());
        } else {
            stats.bytesFiltered += static_cast<quint64>(data.size());
            emit dataFiltered(rule.name, data.size());
        }
    } catch (...) {
        stats.bytesDropped += static_cast<quint64>(data.size());
        ++stats.errors;
        emit bridgeError(rule.name, tr("转发过程中发生异常"));
    }
}

// 统计 getter / reset 已移至 PortBridgeStats.cpp

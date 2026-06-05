/**
 * @file ProtocolSimulator.cpp
 * @brief 协议响应模拟器实现 - 构造/规则管理/数据处理/延迟响应/脚本持久化
 *
 * 核心流程:
 *   1. addRule() 添加规则, processData() 匹配规则并返回响应
 *   2. 精确匹配: QByteArray逐字节比较; 正则匹配: QRegularExpression
 *   3. 延迟响应: 响应数据加入待发送队列, 定时器周期检查并发送
 *   4. 默认响应: 无规则匹配时的兜底应答
 *   5. JSON脚本: loadScript/saveScript 序列化规则集
 *
 * 统计相关方法见 ProtocolSimulatorStats.cpp。
 */

#include "protocol/simulator/ProtocolSimulator.h"

#include <QRegularExpression>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDateTime>
#include <QMutexLocker>

// ============================================================================
// 构造 / 析构
// ============================================================================

/** @brief 构造协议响应模拟器 @param parent 父对象 */
ProtocolSimulator::ProtocolSimulator(QObject* parent)
    : QObject(parent)
    , m_delayTimer(new QTimer(this))
{
    m_delayTimer->setSingleShot(false);
    m_delayTimer->setInterval(kDelayCheckIntervalMs);
    connect(m_delayTimer, &QTimer::timeout,
            this, &ProtocolSimulator::processPendingResponses);
}

/** @brief 析构(停止定时器,清空待发送队列) */
ProtocolSimulator::~ProtocolSimulator()
{
    stop();
    m_pendingResponses.clear();
}

// ============================================================================
// 规则管理
// ============================================================================

/**
 * @brief 添加响应规则
 * @param rule 规则定义(仅使用name/pattern/responseData/delayMs/useRegex/enabled)
 * @return 分配的规则ID, 失败返回-1
 *
 * 自动分配唯一ID,忽略传入的id字段。规则按添加顺序存储,
 * 匹配时从前到后遍历,首个匹配的规则生效。
 */
int ProtocolSimulator::addRule(const ResponseRule& rule)
{
    ResponseRule newRule = rule;
    newRule.id = m_nextRuleId++;
    newRule.matchCount = 0;
    m_rules.append(newRule);
    updateActiveRuleCount();
    return newRule.id;
}

/**
 * @brief 移除规则
 * @param id 规则ID
 * @return true=移除成功 false=未找到指定ID
 */
bool ProtocolSimulator::removeRule(int id)
{
    for (int i = 0; i < m_rules.size(); ++i) {
        if (m_rules[i].id == id) {
            m_rules.removeAt(i);
            updateActiveRuleCount();
            return true;
        }
    }
    return false;
}

/**
 * @brief 更新规则
 * @param id 规则ID
 * @param rule 新的规则内容(保留原ID和matchCount)
 * @return true=更新成功 false=未找到指定ID
 */
bool ProtocolSimulator::updateRule(int id, const ResponseRule& rule)
{
    for (int i = 0; i < m_rules.size(); ++i) {
        if (m_rules[i].id == id) {
            int savedMatchCount = m_rules[i].matchCount;
            m_rules[i] = rule;
            m_rules[i].id = id;
            m_rules[i].matchCount = savedMatchCount;
            updateActiveRuleCount();
            return true;
        }
    }
    return false;
}

/**
 * @brief 启用/禁用规则
 * @param id 规则ID
 * @param enabled true=启用 false=禁用
 */
void ProtocolSimulator::enableRule(int id, bool enabled)
{
    for (int i = 0; i < m_rules.size(); ++i) {
        if (m_rules[i].id == id) {
            m_rules[i].enabled = enabled;
            updateActiveRuleCount();
            return;
        }
    }
}

/** @brief 获取所有规则列表 @return 规则列表副本 */
QList<ProtocolSimulator::ResponseRule> ProtocolSimulator::rules() const
{
    return m_rules;
}

// ============================================================================
// 数据处理
// ============================================================================

/**
 * @brief 处理输入数据
 * @param incoming 收到的请求数据
 * @return 立即可发送的响应(延迟响应返回空,通过信号异步发送)
 *
 * 流程:
 *   1. 更新接收统计
 *   2. 查找匹配规则(精确或正则)
 *   3. 匹配成功: 更新匹配统计,按延迟配置发送响应
 *   4. 未匹配: 尝试发送默认响应
 */
QByteArray ProtocolSimulator::processData(const QByteArray& incoming)
{
    if (!m_running || incoming.isEmpty()) {
        return QByteArray();
    }

    /* 更新接收统计 */
    m_stats.totalRequestsReceived++;
    m_stats.totalBytesReceived += static_cast<quint64>(incoming.size());
    emit requestReceived(incoming);

    /* 查找匹配规则 */
    int ruleIndex = findMatchingRule(incoming);
    if (ruleIndex < 0) {
        /* 未匹配: 发射信号,尝试默认响应 */
        m_stats.totalMisses++;
        emit noMatchFound(incoming);

        if (!m_defaultResponse.isEmpty()) {
            m_stats.totalResponsesSent++;
            m_stats.totalBytesSent += static_cast<quint64>(m_defaultResponse.size());
            emit responseSent(m_defaultResponse, -1);
            return m_defaultResponse;
        }
        return QByteArray();
    }

    /* 匹配成功 */
    ResponseRule& matchedRule = m_rules[ruleIndex];
    matchedRule.matchCount++;
    m_stats.totalMatches++;
    emit ruleMatched(matchedRule.id);

    /* 零延迟: 直接返回响应 */
    if (matchedRule.delayMs <= 0) {
        m_stats.totalResponsesSent++;
        m_stats.totalBytesSent += static_cast<quint64>(matchedRule.responseData.size());

        /* 更新平均响应时间统计(零延迟计为0ms) */
        m_responseTimeCount++;
        m_stats.avgResponseTimeMs =
            static_cast<double>(m_sumResponseTimeMs) /
            static_cast<double>(m_responseTimeCount);

        emit responseSent(matchedRule.responseData, matchedRule.id);
        return matchedRule.responseData;
    }

    /* 有延迟: 加入待发送队列 */
    PendingResponse pending;
    pending.data = matchedRule.responseData;
    pending.sendAtMs = QDateTime::currentMSecsSinceEpoch() + matchedRule.delayMs;
    pending.ruleId = matchedRule.id;
    m_pendingResponses.append(pending);
    m_stats.totalDelayedResponses++;

    /* 更新峰值待处理数 */
    if (m_pendingResponses.size() > m_stats.peakPendingRequests) {
        m_stats.peakPendingRequests = m_pendingResponses.size();
    }

    /* 确保延迟定时器运行 */
    if (!m_delayTimer->isActive()) {
        m_delayTimer->start();
    }

    return QByteArray(); /* 延迟响应异步发送 */
}

// ============================================================================
// 规则匹配
// ============================================================================

/**
 * @brief 查找匹配的规则
 * @param data 输入数据
 * @return 匹配规则在列表中的索引, -1=未找到
 *
 * 遍历所有已启用的规则,按添加顺序匹配:
 *   - 精确匹配(useRegex=false): QByteArray逐字节比较
 *   - 正则匹配(useRegex=true): 将requestPattern视为正则表达式,
 *     将输入数据转为QString后匹配
 */
int ProtocolSimulator::findMatchingRule(const QByteArray& data) const
{
    for (int i = 0; i < m_rules.size(); ++i) {
        const ResponseRule& rule = m_rules[i];
        if (!rule.enabled) {
            continue;
        }

        if (rule.useRegex) {
            /* 正则匹配: 将模式和数据转为QString */
            QRegularExpression regex(QString::fromUtf8(rule.requestPattern));
            if (!regex.isValid()) {
                continue; /* 正则语法无效,跳过 */
            }
            QRegularExpressionMatch match = regex.match(QString::fromUtf8(data));
            if (match.hasMatch()) {
                return i;
            }
        } else {
            /* 精确匹配: 逐字节比较 */
            if (data == rule.requestPattern) {
                return i;
            }
        }
    }
    return -1;
}

// ============================================================================
// 延迟响应处理
// ============================================================================

/**
 * @brief 检查并发送到期的延迟响应
 *
 * 由 m_delayTimer 周期触发。遍历待发送队列,
 * 将所有已到期的响应发射出去并从队列中移除。
 * 队列为空时自动停止定时器。
 */
void ProtocolSimulator::processPendingResponses()
{
    if (m_pendingResponses.isEmpty()) {
        m_delayTimer->stop();
        return;
    }

    qint64 now = QDateTime::currentMSecsSinceEpoch();
    QVector<PendingResponse> remaining;
    remaining.reserve(m_pendingResponses.size());

    for (const PendingResponse& pending : m_pendingResponses) {
        if (pending.sendAtMs <= now) {
            /* 到期: 发送响应 */
            m_stats.totalResponsesSent++;
            m_stats.totalBytesSent += static_cast<quint64>(pending.data.size());

            /* 计算实际响应延迟(用于平均响应时间) */
            qint64 responseDelta = now - pending.sendAtMs;
            if (responseDelta < 0) {
                /* 系统时钟回拨导致负值，跳过该采样点 */
                Q_UNUSED(responseDelta)
            } else {
                m_sumResponseTimeMs += static_cast<quint64>(responseDelta);
                m_responseTimeCount++;
            }

            if (m_responseTimeCount > 0) {
                m_stats.avgResponseTimeMs =
                    static_cast<double>(m_sumResponseTimeMs) /
                    static_cast<double>(m_responseTimeCount);
            }

            emit responseSent(pending.data, pending.ruleId);
        } else {
            /* 未到期: 保留 */
            remaining.append(pending);
        }
    }

    m_pendingResponses = remaining;

    /* 队列清空后停止定时器 */
    if (m_pendingResponses.isEmpty()) {
        m_delayTimer->stop();
    }
}

// ============================================================================
// 模拟控制
// ============================================================================

/** @brief 启动模拟器(开始接收和处理数据) */
void ProtocolSimulator::start()
{
    if (m_running) {
        return;
    }
    m_running = true;
    emit simulatorStateChanged(true);
}

/** @brief 停止模拟器(暂停处理,保留规则和统计) */
void ProtocolSimulator::stop()
{
    if (!m_running) {
        return;
    }
    m_running = false;
    m_delayTimer->stop();
    m_pendingResponses.clear();
    emit simulatorStateChanged(false);
}

/** @brief 模拟器是否正在运行 @return true=运行中 */
bool ProtocolSimulator::isRunning() const
{
    return m_running;
}

// ============================================================================
// 配置
// ============================================================================

/**
 * @brief 设置默认响应
 * @param data 默认响应数据(空=不发送默认响应)
 *
 * 当输入数据未匹配任何规则时,若默认响应非空则发送。
 */
void ProtocolSimulator::setDefaultResponse(const QByteArray& data)
{
    m_defaultResponse = data;
}

// ============================================================================
// 脚本持久化
// ============================================================================

/**
 * @brief 从JSON文件加载规则脚本
 * @param filePath JSON文件路径
 * @return true=加载成功 false=文件读取或解析失败
 *
 * JSON格式:
 * {
 *   "rules": [
 *     {
 *       "name": "规则名称",
 *       "requestPattern": "hex编码的模式",
 *       "responseData": "hex编码的响应",
 *       "delayMs": 100,
 *       "useRegex": false,
 *       "enabled": true
 *     }
 *   ],
 *   "defaultResponse": "hex编码的默认响应"
 * }
 */
bool ProtocolSimulator::loadScript(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    file.close();

    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        return false;
    }

    QJsonObject root = doc.object();

    /* 加载规则列表 */
    QJsonArray rulesArray = root.value("rules").toArray();
    for (const QJsonValue& ruleVal : rulesArray) {
        if (!ruleVal.isObject()) {
            continue;
        }
        QJsonObject ruleObj = ruleVal.toObject();

        ResponseRule rule;
        rule.name = ruleObj.value("name").toString();
        rule.requestPattern = QByteArray::fromHex(
            ruleObj.value("requestPattern").toString().toUtf8());
        rule.responseData = QByteArray::fromHex(
            ruleObj.value("responseData").toString().toUtf8());
        rule.delayMs = ruleObj.value("delayMs").toInt(0);
        rule.useRegex = ruleObj.value("useRegex").toBool(false);
        rule.enabled = ruleObj.value("enabled").toBool(true);
        rule.matchCount = 0;

        rule.id = m_nextRuleId++;
        m_rules.append(rule);
    }

    /* 加载默认响应 */
    QString defaultHex = root.value("defaultResponse").toString();
    if (!defaultHex.isEmpty()) {
        m_defaultResponse = QByteArray::fromHex(defaultHex.toUtf8());
    }

    updateActiveRuleCount();
    return true;
}

/**
 * @brief 保存规则脚本到JSON文件
 * @param filePath 目标文件路径
 * @return true=保存成功 false=文件写入失败
 */
bool ProtocolSimulator::saveScript(const QString& filePath) const
{
    QJsonObject root;
    QJsonArray rulesArray;

    for (const ResponseRule& rule : m_rules) {
        QJsonObject ruleObj;
        ruleObj["name"] = rule.name;
        ruleObj["requestPattern"] = QString::fromUtf8(rule.requestPattern.toHex());
        ruleObj["responseData"] = QString::fromUtf8(rule.responseData.toHex());
        ruleObj["delayMs"] = rule.delayMs;
        ruleObj["useRegex"] = rule.useRegex;
        ruleObj["enabled"] = rule.enabled;
        rulesArray.append(ruleObj);
    }

    root["rules"] = rulesArray;
    if (!m_defaultResponse.isEmpty()) {
        root["defaultResponse"] = QString::fromUtf8(m_defaultResponse.toHex());
    }

    QJsonDocument doc(root);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

// ============================================================================
// 统计接口
// ============================================================================

/** @brief 获取运行统计 @return SimulationStats的const引用 */
const ProtocolSimulator::SimulationStats& ProtocolSimulator::stats() const
{
    return m_stats;
}

/** @brief 更新活跃规则计数 */
void ProtocolSimulator::updateActiveRuleCount()
{
    int count = 0;
    for (const ResponseRule& rule : m_rules) {
        if (rule.enabled) {
            count++;
        }
    }
    m_stats.activeRules = count;
}

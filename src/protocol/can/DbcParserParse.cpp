/**
 * @file DbcParserParse.cpp
 * @brief DBC解析器 - 私有行解析方法(消息/注释/属性/节点)
 *
 * 从 DbcParser.cpp 拆分而来，包含 BO_/CM_/BA_/BU_
 * 各类型行的正则解析方法。
 *
 * SG_ 信号行解析、VAL_ 值表解析、extractBits 位域提取、
 * decodeFrame 帧解码、formatSignalValue 值格式化
 * 见 DbcParserSignal.cpp
 */

#include "protocol/can/DbcParser.h"

#include <QRegularExpression>

// ──────────────────────── 私有: 行解析器 ────────────────────────

/**
 * @brief 解析BO_消息行
 * @param line 行内容 "BO_ <id> <name>: <dlc> <transmitter>"
 * @return true=解析成功
 */
bool DbcParser::parseMessageLine(const QString& line)
{
    /* BO_ 1234 MsgName: 8 NodeName */
    static const QRegularExpression re(
        "^BO_\\s+(\\d+)\\s+(\\w+)\\s*:\\s*(\\d+)\\s+(\\w+)");
    const auto match = re.match(line);
    if (!match.hasMatch()) {
        return false;
    }

    DbcMessage msg;
    msg.id = match.captured(1).toUInt();
    msg.name = match.captured(2);
    msg.dlc = match.captured(3).toInt();
    msg.transmitter = match.captured(4);

    m_messages[msg.id] = msg;
    m_nameToId[msg.name] = msg.id;
    ++m_totalMessagesParsed;
    return true;
}

/**
 * @brief 解析CM_注释行
 * @param line 行内容
 * @return true=解析成功
 *
 * DBC注释格式:
 *   CM_ BO_ <msgId> "<注释>";         — 消息注释
 *   CM_ SG_ <msgId> <sigName> "<注释>"; — 信号注释
 *   CM_ "<注释>";                      — 全局注释(跳过)
 */
bool DbcParser::parseCommentLine(const QString& line)
{
    /* CM_ BO_ 1234 "注释内容"; — 消息级注释 */
    static const QRegularExpression msgCommentRe(
        "^CM_\\s+BO_\\s+(\\d+)\\s+\"([^\"]*)\"\\s*;");
    auto match = msgCommentRe.match(line);
    if (match.hasMatch()) {
        const uint32_t msgId = match.captured(1).toUInt();
        if (m_messages.contains(msgId)) {
            m_messages[msgId].comment = match.captured(2);
        }
        return true;
    }

    /* CM_ SG_ 1234 SignalName "注释内容"; — 信号级注释 */
    static const QRegularExpression sigCommentRe(
        "^CM_\\s+SG_\\s+(\\d+)\\s+(\\w+)\\s+\"([^\"]*)\"\\s*;");
    match = sigCommentRe.match(line);
    if (match.hasMatch()) {
        const uint32_t msgId = match.captured(1).toUInt();
        const QString sigName = match.captured(2);
        const QString comment = match.captured(3);

        if (m_messages.contains(msgId)) {
            DbcMessage& msg = m_messages[msgId];
            for (DbcSignal& sig : msg.signalList) {
                if (sig.name == sigName) {
                    sig.comment = comment;
                    break;
                }
            }
        }
        return true;
    }

    /* CM_ BU_ NodeName "注释"; — 节点注释或其他格式，合法但暂不处理 */
    return false;
}

/**
 * @brief 解析BA_属性行
 * @param line 行内容
 * @return true=解析成功
 *
 * DBC属性定义格式:
 *   BA_ "<attrName>" BO_ <msgId> <value>;     — 消息属性(整数或浮点)
 *   BA_ "<attrName>" SG_ <msgId> <sigName> <value>; — 信号属性
 *   BA_ "<attrName>" "<stringValue>";          — 全局默认属性
 */
bool DbcParser::parseAttributeLine(const QString& line)
{
    /* BA_ "GenMsgCycleTime" BO_ 1234 100; — 消息级属性 */
    static const QRegularExpression msgAttrRe(
        "^BA_\\s+\"([^\"]+)\"\\s+BO_\\s+(\\d+)\\s+([^;]+)\\s*;");
    auto match = msgAttrRe.match(line);
    if (match.hasMatch()) {
        const QString attrName = match.captured(1);
        const uint32_t msgId = match.captured(2).toUInt();
        const QString attrValue = match.captured(3).trimmed();

        if (m_messages.contains(msgId)) {
            m_messages[msgId].attributes[attrName] = attrValue;
        }
        return true;
    }

    /* BA_ "GenSigStartValue" SG_ 1234 SigName 0; — 信号级属性(暂跳过) */
    static const QRegularExpression sigAttrRe(
        "^BA_\\s+\"([^\"]+)\"\\s+SG_\\s+\\d+\\s+\\w+\\s+[^;]+;");
    if (sigAttrRe.match(line).hasMatch()) {
        return true;  ///< 合法格式，暂不存储信号级属性
    }

    /* BA_ "attrName" "stringValue"; — 全局默认属性(暂跳过) */
    static const QRegularExpression globalAttrRe(
        "^BA_\\s+\"([^\"]+)\"\\s+\"([^\"]+)\"\\s*;");
    if (globalAttrRe.match(line).hasMatch()) {
        return true;  ///< 合法格式，暂不处理全局属性
    }

    return false;
}

/**
 * @brief 解析BU_节点行
 * @param line 行内容 "BU_: Node1 Node2 Node3"
 * @return true=解析成功
 */
bool DbcParser::parseNodeLine(const QString& line)
{
    /* BU_: Node1 Node2 Node3 */
    static const QRegularExpression re("^BU_:\\s*(.+)$");
    const auto match = re.match(line);
    if (!match.hasMatch()) {
        return false;
    }

    const QString nodesPart = match.captured(1).trimmed();
    m_nodes = nodesPart.split(' ', Qt::SkipEmptyParts);
    return true;
}


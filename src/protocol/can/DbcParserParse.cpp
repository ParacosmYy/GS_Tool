/**
 * @file DbcParserParse.cpp
 * @brief DBC解析器 - 私有行解析方法与位域提取
 *
 * 从 DbcParser.cpp 拆分而来，包含 BO_/SG_/VAL_/CM_/BA_/BU_
 * 各类型行的正则解析方法以及 extractBits 位域提取算法。
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
 * @brief 解析SG_信号行
 * @param line 行内容
 * @param currentMsgId 当前所属消息ID
 * @return true=解析成功
 *
 * 格式: SG_ <name> [M|m<receiver>]: <startBit>|<bitLength>@<byteOrder><sign>
 *       (<factor>,<offset>) [<min>|<max>] "<unit>" <receiver>
 */
bool DbcParser::parseSignalLine(const QString& line, uint32_t currentMsgId)
{
    if (!m_messages.contains(currentMsgId)) {
        return false;
    }

    static const QRegularExpression re(
        "^SG_\\s+(\\w+)\\s+(?:M|m\\d+\\s+)?:\\s*"
        "(\\d+)\\|(\\d+)@([01])([+-])\\s*"
        "\\(([^,]+),([^)]+)\\)\\s*"
        "\\[([^|]+)\\|([^\\]]+)\\]\\s*"
        "\"([^\"]*)\"\\s*"
        "(\\w+)");

    const auto match = re.match(line);
    if (!match.hasMatch()) {
        return false;
    }

    DbcSignal sig;
    sig.name = match.captured(1);
    sig.startBit = match.captured(2).toInt();
    sig.bitLength = match.captured(3).toInt();
    sig.byteOrder = match.captured(4).toInt();  /* 1=Intel(LE), 0=Motorola(BE) */
    sig.factor = match.captured(6).toDouble();
    sig.offset = match.captured(7).toDouble();
    sig.minimum = match.captured(8).toDouble();
    sig.maximum = match.captured(9).toDouble();
    sig.unit = match.captured(10);
    sig.receiver = match.captured(11);

    m_messages[currentMsgId].signalList.append(sig);
    return true;
}

/**
 * @brief 解析VAL_值表行
 * @param line 行内容 "VAL_ <id> <signalName> <val> "<desc>" ... ;"
 * @return true=解析成功
 */
bool DbcParser::parseValueTableLine(const QString& line)
{
    /* VAL_ 1234 SignalName 0 "Off" 1 "On" ; */
    static const QRegularExpression re("^VAL_\\s+(\\d+)\\s+(\\w+)\\s+(.+);");
    const auto match = re.match(line);
    if (!match.hasMatch()) {
        return false;
    }

    const uint32_t msgId = match.captured(1).toUInt();
    const QString sigName = match.captured(2);
    const QString pairs = match.captured(3).trimmed();

    if (!m_messages.contains(msgId)) {
        return false;
    }

    /* 查找对应信号 */
    DbcMessage& msg = m_messages[msgId];
    DbcSignal* target = nullptr;
    for (DbcSignal& sig : msg.signalList) {
        if (sig.name == sigName) {
            target = &sig;
            break;
        }
    }
    if (!target) {
        return false;
    }

    /* 解析 "值 描述" 对 */
    static const QRegularExpression pairRe("(\\d+)\\s+\"([^\"]*)\"");
    auto it = pairRe.globalMatch(pairs);
    while (it.hasNext()) {
        const auto m = it.next();
        const int val = m.captured(1).toInt();
        const QString desc = m.captured(2);
        target->valueTable[val] = desc;
    }

    return true;
}

/**
 * @brief 解析CM_注释行
 * @param line 行内容
 * @return true=解析成功
 */
bool DbcParser::parseCommentLine(const QString& line)
{
    /* CM_ BO_ 1234 "注释内容"; */
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

    /* CM_ SG_ 1234 SignalName "注释内容"; */
    static const QRegularExpression sigCommentRe(
        "^CM_\\s+SG_\\s+(\\d+)\\s+(\\w+)\\s+\"([^\"]*)\"\\s*;");
    match = sigCommentRe.match(line);
    Q_UNUSED(match)

    return false;
}

/**
 * @brief 解析BA_属性行
 * @param line 行内容 "BA_ "<attrName>" BO_ <id> <value>;"
 * @return true=解析成功
 */
bool DbcParser::parseAttributeLine(const QString& line)
{
    /* BA_ "GenMsgCycleTime" BO_ 1234 100; */
    static const QRegularExpression re(
        "^BA_\\s+\"([^\"]+)\"\\s+BO_\\s+(\\d+)\\s+(\\w+)\\s*;");
    const auto match = re.match(line);
    if (!match.hasMatch()) {
        return false;
    }

    const QString attrName = match.captured(1);
    const uint32_t msgId = match.captured(2).toUInt();
    const QString attrValue = match.captured(3);

    if (m_messages.contains(msgId)) {
        m_messages[msgId].attributes[attrName] = attrValue;
    }

    return true;
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

// ──────────────────────── 私有: 位域提取 ────────────────────────

/**
 * @brief 从字节数据中提取指定位域的原始值
 * @param data 帧数据(最多8字节)
 * @param startBit 起始位(DBC编号)
 * @param bitLength 位长度
 * @param byteOrder 字节序(1=Intel小端, 0=Motorola大端)
 * @return 提取的原始值
 */
uint64_t DbcParser::extractBits(const QByteArray& data, int startBit,
                                 int bitLength, int byteOrder) const
{
    const int dataLen = data.size();
    if (dataLen == 0 || bitLength <= 0 || bitLength > 64) {
        return 0;
    }

    uint64_t result = 0;

    if (byteOrder == 1) {
        /* Intel / 小端(LSB): 位序从低位到高位顺序排列 */
        for (int i = 0; i < bitLength; ++i) {
            const int bitPos = startBit + i;
            const int byteIdx = bitPos / 8;
            const int bitIdx = bitPos % 8;
            if (byteIdx < dataLen) {
                const uint8_t byteVal = static_cast<uint8_t>(data[byteIdx]);
                if (byteVal & (1 << bitIdx)) {
                    result |= (1ULL << i);
                }
            }
        }
    } else {
        /* Motorola / 大端(MSB): DBC中使用翻转位编号 */
        for (int i = 0; i < bitLength; ++i) {
            int bitPos = startBit - i;
            if (bitPos < 0) {
                break;
            }
            const int byteIdx = bitPos / 8;
            const int bitIdx = 7 - (bitPos % 8);
            if (byteIdx < dataLen) {
                const uint8_t byteVal = static_cast<uint8_t>(data[byteIdx]);
                if (byteVal & (1 << bitIdx)) {
                    result |= (1ULL << (bitLength - 1 - i));
                }
            }
        }
    }

    return result;
}

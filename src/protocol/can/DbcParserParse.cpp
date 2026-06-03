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
 * DBC信号定义格式:
 *   SG_ <name> : <startBit>|<length>@<endian><sign> (<factor>,<offset>) [<min>|<max>] "<unit>" <receiver>
 *   SG_ <name> M : ...             (多路复用标记信号)
 *   SG_ <name> m<MUX> : ...        (多路复用从信号，MUX为十六进制值)
 */
bool DbcParser::parseSignalLine(const QString& line, uint32_t currentMsgId)
{
    if (!m_messages.contains(currentMsgId)) {
        return false;
    }

    /**
     * 正则分组说明:
     *   1: 信号名
     *   2: 多路复用标记 (M | m<hex> | 空)
     *   3: 起始位
     *   4: 位长度
     *   5: 字节序 (1=Intel/LE, 0=Motorola/BE)
     *   6: 符号 (+=无符号, -=有符号)
     *   7: 缩放因子
     *   8: 偏移量
     *   9: 最小值
     *  10: 最大值
     *  11: 单位
     *  12: 接收节点(可能含逗号分隔的多个节点，如 Node1,Node2)
     */
    static const QRegularExpression re(
        "^SG_\\s+(\\w+)\\s*(M|m[0-9A-Fa-f]+)?\\s*:\\s*"
        "(\\d+)\\|(\\d+)@([01])([+-])\\s*"
        "\\(([^,]+),([^)]+)\\)\\s*"
        "\\[([^|]+)\\|([^\\]]+)\\]\\s*"
        "\"([^\"]*)\"\\s*"
        "([\\w,]+)");

    const auto match = re.match(line);
    if (!match.hasMatch()) {
        return false;
    }

    DbcSignal sig;
    sig.name = match.captured(1);
    /* match.captured(2) 为多路复用标记，当前版本暂不存储，后续可扩展 */
    sig.startBit = match.captured(3).toInt();
    sig.bitLength = match.captured(4).toInt();
    sig.byteOrder = match.captured(5).toInt();  ///< 1=Intel(LE), 0=Motorola(BE)
    /* match.captured(6) 为符号标记: +无符号, -有符号 */
    sig.factor = match.captured(7).toDouble();
    sig.offset = match.captured(8).toDouble();
    sig.minimum = match.captured(9).toDouble();
    sig.maximum = match.captured(10).toDouble();
    sig.unit = match.captured(11);
    sig.receiver = match.captured(12);

    m_messages[currentMsgId].signalList.append(sig);
    return true;
}

/**
 * @brief 解析VAL_值表行
 * @param line 行内容
 * @return true=解析成功
 *
 * DBC值表格式:
 *   VAL_ <msgId> <signalName> <value1> "<desc1>" <value2> "<desc2>" ... ;
 *   VAL_ <globalTableName> <value1> "<desc1>" ... ;  (全局值表，暂不处理)
 */
bool DbcParser::parseValueTableLine(const QString& line)
{
    /* VAL_ 1234 SignalName 0 "Off" 1 "On" ; — 信号关联值表 */
    static const QRegularExpression sigValRe("^VAL_\\s+(\\d+)\\s+(\\w+)\\s+(.+);");
    const auto match = sigValRe.match(line);
    if (!match.hasMatch()) {
        /**
         * 全局值表格式: VAL_ <tableName> <val> "<desc>" ... ;
         * 当前版本不处理全局值表(无消息ID关联)，直接跳过不报错
         */
        static const QRegularExpression globalValRe("^VAL_\\s+(\\w+)\\s+.+;");
        if (globalValRe.match(line).hasMatch()) {
            return true;  ///< 全局值表，合法但暂不处理
        }
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

    /* 解析 "值 描述" 对: 匹配所有 <integer> "string" 模式 */
    static const QRegularExpression pairRe("(-?\\d+)\\s+\"([^\"]*)\"");
    auto it = pairRe.globalMatch(pairs);
    bool foundAny = false;
    while (it.hasNext()) {
        const auto m = it.next();
        const int val = m.captured(1).toInt();
        const QString desc = m.captured(2);
        target->valueTable[val] = desc;
        foundAny = true;
    }

    return foundAny;
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

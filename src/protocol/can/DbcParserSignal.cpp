/**
 * @file DbcParserSignal.cpp
 * @brief DBC解析器 - 信号解码、值表查找与位域提取
 *
 * 从 DbcParserParse.cpp / DbcParser.cpp 拆分而来，包含:
 * - SG_ 信号行解析
 * - VAL_ 值表行解析
 * - extractBits 位域提取算法(Intel/Motorola双字节序)
 * - decodeFrame 帧解码(原始值→物理值)
 * - formatSignalValue 物理值格式化(含值表翻译)
 */

#include "protocol/can/DbcParser.h"

#include <QRegularExpression>

// ──────────────────────── 私有: 信号行解析 ────────────────────────

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

// ──────────────────────── 公开: 信号解码 ────────────────────────

/**
 * @brief 解码CAN帧数据为信号值映射
 * @param msgId 消息ID
 * @param data 帧数据(最多8字节)
 * @return 信号名→物理值映射
 *
 * 遍历消息的所有信号，使用 extractBits 提取原始值，
 * 再经 factor/offset 缩放转换为物理值。
 */
QMap<QString, double> DbcParser::decodeFrame(uint32_t msgId,
                                              const QByteArray& data) const
{
    QMap<QString, double> result;

    auto it = m_messages.constFind(msgId);
    if (it == m_messages.constEnd()) {
        return result;
    }

    const DbcMessage& msg = it.value();
    for (const DbcSignal& sig : msg.signalList) {
        const uint64_t raw = extractBits(data, sig.startBit,
                                          sig.bitLength, sig.byteOrder);
        const double physical = static_cast<double>(raw) * sig.factor + sig.offset;
        result[sig.name] = physical;
    }
    m_totalSignalsDecoded += static_cast<quint64>(msg.signalList.size());

    return result;
}

/**
 * @brief 获取信号物理值的文本描述(含值表翻译)
 * @param msgId 消息ID
 * @param signalName 信号名
 * @param rawValue 原始值
 * @return 格式化字符串
 *
 * 优先使用值表(VAL_)翻译整数值；无匹配时按 factor/offset
 * 格式化为 "值 单位" 字符串。
 */
QString DbcParser::formatSignalValue(uint32_t msgId,
                                      const QString& signalName,
                                      double rawValue) const
{
    auto msgIt = m_messages.constFind(msgId);
    if (msgIt == m_messages.constEnd()) { return QString(); }

    const DbcMessage& msg = msgIt.value();
    const DbcSignal* targetSig = nullptr;
    for (const DbcSignal& sig : msg.signalList) {
        if (sig.name == signalName) { targetSig = &sig; break; }
    }
    if (!targetSig) { return QString(); }

    /* 值表翻译(仅整数值) */
    const int intVal = static_cast<int>(rawValue);
    if (targetSig->valueTable.contains(intVal)) {
        return targetSig->valueTable.value(intVal);
    }

    /* 通用格式: "值 单位" */
    QString text;
    if (targetSig->factor == 1.0 && targetSig->offset == 0.0) {
        text = QString::number(intVal);
    } else {
        text = QString::number(rawValue, 'f', 2);
    }
    if (!targetSig->unit.isEmpty()) {
        text += QLatin1Char(' ') + targetSig->unit;
    }
    return text;
}

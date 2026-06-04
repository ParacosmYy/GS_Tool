/**
 * @file DbcParserDecode.cpp
 * @brief DBC解析器 - 帧解码、位域提取与信号值格式化
 *
 * 从 DbcParserSignal.cpp 拆分而来，包含:
 *   - extractBits: 位域提取算法(Intel/Motorola双字节序)
 *   - decodeFrame: 帧解码(原始值→物理值)
 *   - formatSignalValue: 物理值格式化(含值表翻译)
 */

#include "protocol/can/DbcParser.h"
#include <QLatin1Char>

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

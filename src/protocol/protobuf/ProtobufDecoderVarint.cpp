/**
 * @file ProtobufDecoderVarint.cpp
 * @brief Protobuf varint编解码与wire type字段解码实现
 *
 * 从ProtobufDecoderEncoding.cpp拆分而来，包含:
 *   - writeVarint: 将64位无符号整数编码为varint格式写入缓冲区
 *   - parseVarint: 从二进制数据中解析varint值（溢出保护，最多10字节）
 *   - decodeField: 根据wire type解码单个字段（支持嵌套递归+重复字段跟踪）
 *
 * varint是Protobuf的核心变长编码格式，每个字节的最高位(MSB)标识是否还有后续字节。
 * wire type决定字段值的编码方式: 0=varint, 1=64-bit, 2=length-delimited, 5=32-bit。
 */

#include "protocol/protobuf/ProtobufDecoder.h"

// ============================================================================
// Varint 最大字节数常量
// ============================================================================

/** @brief Protobuf varint最多占用10字节（64位最多9字节+1字节溢出保护） */
static constexpr int kMaxVarintBytes = 10;

// ============================================================================
// Varint编解码
// ============================================================================

/** @brief 将varint编码写入缓冲区
 *
 * 每个字节低7位承载数据，最高位(MSB)为延续标志:
 *   - MSB=1 表示后面还有更多字节
 *   - MSB=0 表示这是最后一个字节
 * 小端序: 最低有效组先写入
 *
 * @param buf   目标缓冲区（QBuffer，需已打开WriteOnly）
 * @param value 待写入的64位无符号整数
 */
void ProtobufDecoder::writeVarint(QBuffer& buf, quint64 value) const {
    while (value > 0x7F) {
        buf.putChar(static_cast<char>((value & 0x7F) | 0x80));
        value >>= 7;
    }
    buf.putChar(static_cast<char>(value & 0x7F));
}

/** @brief 从数据中解析一个varint值（带溢出保护，最多10字节）
 *
 * 解析流程:
 *   1. 逐字节读取，低7位拼接到结果对应位置
 *   2. 最高位为0时表示varint结束
 *   3. 超过10字节视为数据损坏，返回{0, 0}
 *   4. 第10字节只允许最低位有效（Protobuf规范）
 *
 * @param data   源二进制数据
 * @param offset 起始偏移量
 * @return QPair<解析后的值, 消耗的字节数>，出错时返回{0, 0}
 */
QPair<quint64, int> ProtobufDecoder::parseVarint(const QByteArray& data,
                                                  int offset) const {
    quint64 value = 0;
    int shift = 0;
    int pos = offset;

    while (pos < data.size()) {
        // 溢出保护: varint最多10字节
        if (pos - offset >= kMaxVarintBytes) {
            // 超过最大长度，数据损坏
            return {0, 0};
        }

        quint8 byte = static_cast<quint8>(data[pos]);
        value |= static_cast<quint64>(byte & 0x7F) << shift;
        pos++;

        // 最高位为0表示varint结束
        if ((byte & 0x80) == 0) {
            return {value, pos - offset};
        }

        shift += 7;

        // 位移超过63位（64位无符号整数最大范围）的额外保护
        if (shift >= 64) {
            // 第10个字节只允许最低位有效（Protobuf规范）
            if (pos - offset == 10) {
                quint8 lastByte = static_cast<quint8>(data[pos - 1]);
                if ((lastByte & 0x7E) != 0) {
                    // 超过64位范围，数据损坏
                    return {0, 0};
                }
                value |= static_cast<quint64>(lastByte & 0x01) << 63;
                return {value, pos - offset};
            }
            return {0, 0};
        }
    }

    // 到达数据末尾但varint未结束
    return {0, 0};
}

// ---- 字段解码(wire type分发+嵌套递归)见 ProtobufDecoderField.cpp ----
// decodeField

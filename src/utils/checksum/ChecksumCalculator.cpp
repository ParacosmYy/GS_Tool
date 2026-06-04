/**
 * @file ChecksumCalculator.cpp
 * @brief 校验和计算器实现
 * @author Serial Tool Team
 * @date 2026-06-02
 */

#include "utils/checksum/ChecksumCalculator.h"

/** @brief 构造函数 @param parent 父对象 */
ChecksumCalculator::ChecksumCalculator(QObject *parent)
    : QObject(parent)
{
}

/** @brief 计算校验和(根据算法分派到CRC8/CRC16/CRC32/Xor/Sum等具体实现)，空数据返回0 @param data 待计算的字节数据 @param alg 校验和算法枚举 @return 校验和结果 */
quint64 ChecksumCalculator::calculate(const QByteArray &data, Algorithm alg) const
{
    if (data.isEmpty()) {
        return 0;
    }

    ++m_totalCalculations;
    m_totalBytesProcessed += static_cast<quint64>(data.size());
    ++m_algorithmCounts[static_cast<int>(alg)]; ///< 统计: 按算法计数递增

    switch (alg) {
    case CRC8: {
        quint8 crc = 0x00;
        for (char byte : data) {
            crc ^= static_cast<quint8>(byte);
            for (int i = 0; i < 8; ++i) {
                if (crc & 0x80) {
                    crc = (crc << 1) ^ 0x07;
                } else {
                    crc <<= 1;
                }
            }
        }
        return crc;
    }

    case CRC16Ccitt: {
        quint16 crc = 0xFFFF;
        for (char byte : data) {
            crc ^= (static_cast<quint16>(byte) << 8);
            for (int i = 0; i < 8; ++i) {
                if (crc & 0x8000) {
                    crc = (crc << 1) ^ 0x1021;
                } else {
                    crc <<= 1;
                }
            }
        }
        return crc;
    }

    case CRC16Modbus: {
        quint16 crc = 0xFFFF;
        for (char byte : data) {
            crc ^= static_cast<quint8>(byte);
            for (int i = 0; i < 8; ++i) {
                if (crc & 0x0001) {
                    crc = (crc >> 1) ^ 0xA001;
                } else {
                    crc >>= 1;
                }
            }
        }
        return crc;
    }

    case CRC16Kermit: {
        quint16 crc = 0x0000;
        for (char byte : data) {
            crc ^= static_cast<quint8>(byte);
            for (int i = 0; i < 8; ++i) {
                if (crc & 0x0001) {
                    crc = (crc >> 1) ^ 0x8408;
                } else {
                    crc >>= 1;
                }
            }
        }
        return crc;
    }

    case CRC32: {
        quint32 crc = 0xFFFFFFFF;
        for (char byte : data) {
            crc ^= static_cast<quint8>(byte);
            for (int i = 0; i < 8; ++i) {
                if (crc & 0x00000001) {
                    crc = (crc >> 1) ^ 0xEDB88320;
                } else {
                    crc >>= 1;
                }
            }
        }
        return crc ^ 0xFFFFFFFF;
    }

    case CRC32C: {
        quint32 crc = 0xFFFFFFFF;
        for (char byte : data) {
            crc ^= static_cast<quint8>(byte);
            for (int i = 0; i < 8; ++i) {
                if (crc & 0x00000001) {
                    crc = (crc >> 1) ^ 0x82F63B78;
                } else {
                    crc >>= 1;
                }
            }
        }
        return crc ^ 0xFFFFFFFF;
    }

    case Xor8: {
        quint8 result = 0x00;
        for (char byte : data) {
            result ^= static_cast<quint8>(byte);
        }
        return result;
    }

    case Sum8: {
        quint32 sum = 0;
        for (char byte : data) {
            sum += static_cast<quint8>(byte);
        }
        return sum & 0xFF;
    }

    case Sum16: {
        quint32 sum = 0;
        for (int i = 0; i + 1 < data.size(); i += 2) {
            sum += (static_cast<quint8>(data[i]) << 8) | static_cast<quint8>(data[i + 1]);
        }
        if (data.size() % 2 != 0) {
            sum += static_cast<quint8>(data[data.size() - 1]) << 8;
        }
        return sum & 0xFFFF;
    }

    case Sum32: {
        quint64 sum = 0;
        for (int i = 0; i + 3 < data.size(); i += 4) {
            sum += (quint32(static_cast<quint8>(data[i])) << 24)
                   | (quint32(static_cast<quint8>(data[i + 1])) << 16)
                   | (quint32(static_cast<quint8>(data[i + 2])) << 8)
                   | quint32(static_cast<quint8>(data[i + 3]));
        }
        /* 处理尾部不足4字节的剩余数据 */
        int remaining = data.size() % 4;
        int tailStart = data.size() - remaining;
        for (int i = tailStart; i < data.size(); ++i) {
            sum += quint32(static_cast<quint8>(data[i]))
                   << (24 - 8 * (i - tailStart));
        }
        return sum & 0xFFFFFFFF;
    }

    case CustomCrc:
        return 0; // 使用 calculateCustom
    }

    return 0;
}

/** @brief 自定义CRC多项式计算，支持8/16/32位宽度 @param data 待计算的字节数据 @param polynomial CRC多项式 @param width 位宽(8/16/32) @return CRC校验结果 */
quint64 ChecksumCalculator::calculateCustom(const QByteArray &data, quint64 polynomial, int width) const
{
    if (data.isEmpty() || polynomial == 0) {
        return 0;
    }

    ++m_totalCustomCalculations; ///< 统计: 自定义CRC计算递增
    ++m_totalCalculations;
    m_totalBytesProcessed += static_cast<quint64>(data.size());
    ++m_algorithmCounts[static_cast<int>(CustomCrc)];

    quint64 mask = (width == 8) ? 0xFF : (width == 16) ? 0xFFFF : 0xFFFFFFFF;
    quint64 crc = mask;

    for (char byte : data) {
        crc ^= (static_cast<quint64>(static_cast<quint8>(byte)) << (width - 8));
        for (int i = 0; i < 8; ++i) {
            if (crc & (1ULL << (width - 1))) {
                crc = ((crc << 1) ^ polynomial) & mask;
            } else {
                crc = (crc << 1) & mask;
            }
        }
    }

    return crc;
}

// algorithmName/algorithmBitWidth/algorithmDescription/calculateAll/stats见 ChecksumCalculatorStats.cpp

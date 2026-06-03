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

/** @brief 获取算法的标准名称字符串 @param alg 校验和算法枚举 @return 算法名称(如"CRC-8"/"CRC-16/Modbus"/"SUM-32"等) */
QString ChecksumCalculator::algorithmName(Algorithm alg)
{
    switch (alg) {
    case CRC8:       return QStringLiteral("CRC-8");
    case CRC16Ccitt: return QStringLiteral("CRC-16/CCITT");
    case CRC16Modbus:return QStringLiteral("CRC-16/Modbus");
    case CRC16Kermit:return QStringLiteral("CRC-16/Kermit");
    case CRC32:      return QStringLiteral("CRC-32");
    case CRC32C:     return QStringLiteral("CRC-32C");
    case Xor8:       return QStringLiteral("XOR-8");
    case Sum8:       return QStringLiteral("SUM-8");
    case Sum16:      return QStringLiteral("SUM-16");
    case Sum32:      return QStringLiteral("SUM-32");
    case CustomCrc:  return QStringLiteral("Custom CRC");
    }
    return QStringLiteral("Unknown");
}

/** @brief 获取算法的位宽 @param alg 校验和算法枚举 @return 位宽(8/16/32)，CustomCrc返回0(由用户指定) */
int ChecksumCalculator::algorithmBitWidth(Algorithm alg)
{
    switch (alg) {
    case CRC8:       return 8;
    case CRC16Ccitt:
    case CRC16Modbus:
    case CRC16Kermit:
    case Sum16:      return 16;
    case CRC32:
    case CRC32C:
    case Sum32:      return 32;
    case Xor8:
    case Sum8:       return 8;
    case CustomCrc:  return 0;  // 由用户指定
    }
    return 0;
}

/** @brief 获取算法的人类可读中文描述 @param alg 校验和算法枚举 @return 包含多项式和用途的中文描述 */
QString ChecksumCalculator::algorithmDescription(Algorithm alg)
{
    switch (alg) {
    case CRC8:       return tr("CRC-8 标准校验，多项式 0x07");
    case CRC16Ccitt: return tr("CRC-16/CCITT，多项式 0x1021，常用于通信协议");
    case CRC16Modbus:return tr("CRC-16/Modbus，多项式 0xA001，工业标准");
    case CRC16Kermit:return tr("CRC-16/Kermit，多项式 0x8408，又名CRC-CCITT");
    case CRC32:      return tr("CRC-32，多项式 0xEDB88320，以太网/ZIP标准");
    case CRC32C:     return tr("CRC-32C Castagnoli，多项式 0x82F63B78，iSCSI标准");
    case Xor8:       return tr("8位异或校验，简单快速");
    case Sum8:       return tr("8位累加和，取低8位");
    case Sum16:      return tr("16位累加和，大端序双字节累加");
    case Sum32:      return tr("32位累加和，大端序四字节累加");
    case CustomCrc:  return tr("自定义CRC多项式和位宽");
    }
    return QString();
}

/** @brief 使用所有内置算法计算同一份数据的校验和 @param data 待计算的字节数据 @return 算法名称到校验结果的映射表 */
QMap<QString, quint64> ChecksumCalculator::calculateAll(const QByteArray& data) const
{
    QMap<QString, quint64> results;
    const QList<Algorithm> algorithms = {
        CRC8, CRC16Ccitt, CRC16Modbus, CRC16Kermit,
        CRC32, CRC32C, Xor8, Sum8, Sum16, Sum32
    };
    for (Algorithm alg : algorithms) {
        results[algorithmName(alg)] = calculate(data, alg);
    }
    return results;
}

/** @brief 获取累计计算次数 @return 计算总次数 */
quint64 ChecksumCalculator::totalCalculations() const
{
    return m_totalCalculations;
}

/** @brief 获取累计处理字节数 @return 处理字节总数 */
quint64 ChecksumCalculator::totalBytesProcessed() const
{
    return m_totalBytesProcessed;
}

/** @brief 重置所有校验和统计计数器(计算次数和处理字节数归零) */
void ChecksumCalculator::resetChecksumStatistics()
{
    m_totalCalculations = 0;
    m_totalBytesProcessed = 0;
}

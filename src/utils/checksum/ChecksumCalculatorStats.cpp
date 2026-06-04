/**
 * @file ChecksumCalculatorStats.cpp
 * @brief 校验和计算器统计查询和元数据方法实现
 *
 * 从 ChecksumCalculator.cpp 拆分而来，包含algorithmName/algorithmBitWidth/
 * algorithmDescription/calculateAll和统计getter/resetStats方法。
 */

#include "utils/checksum/ChecksumCalculator.h"

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

/** @brief 获取算法的位宽 @param alg 校验和算法枚举 @return 位宽(8/16/32)，CustomCrc返回0 */
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
    case CustomCrc:  return 0;
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
    ++m_totalCalculateAllCalls; ///< 统计: 批量计算调用递增
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

/** @brief 获取指定算法的累计计算次数 @param alg 算法枚举 @return 该算法的计算总次数，未使用过返回0 */
quint64 ChecksumCalculator::totalComputationsByAlgorithm(Algorithm alg) const
{
    return m_algorithmCounts.value(static_cast<int>(alg), 0);
}

/** @brief 重置所有校验和统计计数器(计算次数/字节数/自定义次数/批量次数/按算法计数归零) */
void ChecksumCalculator::resetChecksumStatistics()
{
    m_totalCalculations = 0;
    m_totalBytesProcessed = 0;
    m_totalCustomCalculations = 0;
    m_totalCalculateAllCalls = 0;
    m_algorithmCounts.clear();
}

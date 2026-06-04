/**
 * @file DataChecksumVerifierStats.cpp
 * @brief 数据校验验证引擎 -- 统计查询和元数据方法实现
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 从 DataChecksumVerifier.cpp 拆分而来，包含:
 * algorithmName/algorithmWidth/allAlgorithms/stats/resetStatistics。
 */

#include "utils/checksum/DataChecksumVerifier.h"

#include <QStringList>

/** @brief 获取算法的标准名称字符串 @param algo 校验算法枚举 @return 算法名称 */
QString DataChecksumVerifier::algorithmName(Algorithm algo)
{
    switch (algo) {
    case Algorithm::CRC8:             return QStringLiteral("CRC-8");
    case Algorithm::CRC16_CCITT:      return QStringLiteral("CRC-16/CCITT");
    case Algorithm::CRC16_MODBUS:     return QStringLiteral("CRC-16/Modbus");
    case Algorithm::CRC16_XMODEM:    return QStringLiteral("CRC-16/XMODEM");
    case Algorithm::CRC32:            return QStringLiteral("CRC-32");
    case Algorithm::CRC32C:           return QStringLiteral("CRC-32C");
    case Algorithm::XOR8:             return QStringLiteral("XOR-8");
    case Algorithm::Sum8:             return QStringLiteral("SUM-8");
    case Algorithm::Sum16_LE:         return QStringLiteral("SUM-16/LE");
    case Algorithm::Sum16_BE:         return QStringLiteral("SUM-16/BE");
    case Algorithm::Sum32_LE:         return QStringLiteral("SUM-32/LE");
    case Algorithm::OnesComplement16: return QStringLiteral("OnesComp-16");
    case Algorithm::TwosComplement16: return QStringLiteral("TwosComp-16");
    case Algorithm::Fletcher8:        return QStringLiteral("Fletcher-8");
    case Algorithm::Fletcher16:       return QStringLiteral("Fletcher-16");
    case Algorithm::Fletcher32:       return QStringLiteral("Fletcher-32");
    case Algorithm::Adler32:          return QStringLiteral("Adler-32");
    }
    return QStringLiteral("Unknown");
}

/**
 * @brief 获取算法输出位宽
 * @param algo 校验算法枚举
 * @return 8/16/32 位
 */
int DataChecksumVerifier::algorithmWidth(Algorithm algo)
{
    switch (algo) {
    case Algorithm::CRC8:             return 8;
    case Algorithm::CRC16_CCITT:
    case Algorithm::CRC16_MODBUS:
    case Algorithm::CRC16_XMODEM:
    case Algorithm::Sum16_LE:
    case Algorithm::Sum16_BE:
    case Algorithm::OnesComplement16:
    case Algorithm::TwosComplement16:
    case Algorithm::Fletcher16:       return 16;
    case Algorithm::CRC32:
    case Algorithm::CRC32C:
    case Algorithm::Sum32_LE:
    case Algorithm::Fletcher32:
    case Algorithm::Adler32:          return 32;
    case Algorithm::XOR8:
    case Algorithm::Sum8:
    case Algorithm::Fletcher8:        return 8;
    }
    return 8;
}

/**
 * @brief 获取全部17种算法枚举列表
 * @return 按定义顺序的算法列表
 */
QList<DataChecksumVerifier::Algorithm> DataChecksumVerifier::allAlgorithms()
{
    return {
        Algorithm::CRC8,
        Algorithm::CRC16_CCITT,
        Algorithm::CRC16_MODBUS,
        Algorithm::CRC16_XMODEM,
        Algorithm::CRC32,
        Algorithm::CRC32C,
        Algorithm::XOR8,
        Algorithm::Sum8,
        Algorithm::Sum16_LE,
        Algorithm::Sum16_BE,
        Algorithm::Sum32_LE,
        Algorithm::OnesComplement16,
        Algorithm::TwosComplement16,
        Algorithm::Fletcher8,
        Algorithm::Fletcher16,
        Algorithm::Fletcher32,
        Algorithm::Adler32
    };
}

/**
 * @brief 获取累计统计信息
 * @return 统计结构的const引用
 */
const DataChecksumVerifier::Stats &DataChecksumVerifier::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计计数器(验证次数/通过/失败/字节数/平均耗时/按算法计数归零)
 */
void DataChecksumVerifier::resetStatistics()
{
    m_stats.totalVerifications = 0;
    m_stats.totalPasses = 0;
    m_stats.totalFailures = 0;
    m_stats.totalBytesProcessed = 0;
    m_stats.avgComputationTimeUs = 0.0;
    for (int i = 0; i < 17; ++i)
        m_stats.computationsByAlgorithm[i] = 0;
}

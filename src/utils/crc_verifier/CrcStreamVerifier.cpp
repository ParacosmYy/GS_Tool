/**
 * @file CrcStreamVerifier.cpp
 * @brief CRC流验证器实现 — 实时数据流CRC校验
 */

#include "utils/crc_verifier/CrcStreamVerifier.h"

#include <QVector>

/** @brief 构造函数 @param parent 父对象 */
CrcStreamVerifier::CrcStreamVerifier(QObject* parent)
    : QObject(parent)
    , m_algorithm(CrcAlgorithm::Crc16Modbus)
    , m_blockSize(64)
    , m_nextExpected(0)
    , m_blockIndex(0)
    , m_consecutiveFailures(0)
{
}

/** @brief 设置CRC算法 @param algo 算法 */
void CrcStreamVerifier::setAlgorithm(CrcAlgorithm algo)
{
    m_algorithm = algo;
}

/** @brief 设置验证块大小 @param size 块大小(字节) */
void CrcStreamVerifier::setBlockSize(int size)
{
    m_blockSize = qMax(1, size);
    m_stats.blockSize = m_blockSize;
}

/** @brief 喂入数据流，缓冲到块大小后自动验证 @param data 数据 */
void CrcStreamVerifier::feedData(const QByteArray& data)
{
    m_buffer.append(data);

    while (m_buffer.size() >= m_blockSize) {
        QByteArray block = m_buffer.left(m_blockSize);
        m_buffer.remove(0, m_blockSize);

        VerifyResult result = verifyBlock(block, m_nextExpected);
        m_history.append(result);

        /* 保留最近100条历史 */
        while (m_history.size() > 100) {
            m_history.removeFirst();
        }

        m_nextExpected = 0; /* 重置预期值，等待外部设置下一个 */
    }
}

/** @brief 设置下一个预期CRC值 @param expected 预期值 */
void CrcStreamVerifier::setExpectedCrc(quint32 expected)
{
    m_nextExpected = expected;
}

/** @brief 手动验证一个块 @param data 数据 @param expected 预期CRC @return 验证结果 */
CrcStreamVerifier::VerifyResult CrcStreamVerifier::verifyBlock(
    const QByteArray& data, quint32 expected)
{
    VerifyResult result;
    result.blockIndex = m_blockIndex;
    result.byteOffset = static_cast<qint64>(m_blockIndex) * m_blockSize;
    result.data = data;
    result.expectedCrc = expected;
    result.computedCrc = computeCrc(data);

    /* 如果预期值为0则跳过比较(仅计算模式) */
    if (expected == 0) {
        result.passed = true;
    } else {
        result.passed = (result.computedCrc == expected);
    }

    ++m_stats.totalBlocksVerified;
    m_stats.totalBytesVerified += static_cast<quint64>(data.size());

    if (result.passed) {
        ++m_stats.totalPasses;
        m_consecutiveFailures = 0;
        emit blockVerified(result.blockIndex, result.computedCrc);
    } else {
        ++m_stats.totalFailures;
        ++m_consecutiveFailures;
        if (m_consecutiveFailures > m_stats.peakFailures) {
            m_stats.peakFailures = m_consecutiveFailures;
        }
        emit blockFailed(result);
        if (m_consecutiveFailures >= 3) {
            emit consecutiveFailures(m_consecutiveFailures);
        }
    }

    ++m_blockIndex;
    return result;
}

/** @brief 计算CRC @param data 数据 @return CRC值 */
quint32 CrcStreamVerifier::computeCrc(const QByteArray& data) const
{
    switch (m_algorithm) {
    case CrcAlgorithm::Crc8:        return computeCrc8(data);
    case CrcAlgorithm::Crc16:       return computeCrc16(data);
    case CrcAlgorithm::Crc16Modbus: return computeCrc16Modbus(data);
    case CrcAlgorithm::Crc16Ccitt:  return computeCrc16(data); /* 近似 */
    case CrcAlgorithm::Crc32:       return computeCrc32(data);
    case CrcAlgorithm::Crc32C:      return computeCrc32(data); /* 近似 */
    case CrcAlgorithm::Xor8:        return computeXor8(data);
    case CrcAlgorithm::Checksum8:   return computeChecksum8(data);
    }
    return 0;
}

/** @brief CRC-8计算 @param data 数据 @return CRC值 */
quint32 CrcStreamVerifier::computeCrc8(const QByteArray& data) const
{
    quint8 crc = 0x00;
    for (char b : data) {
        crc ^= static_cast<quint8>(b);
        for (int i = 0; i < 8; ++i) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x07;
            } else {
                crc <<= 1;
            }
        }
    }
    return static_cast<quint32>(crc);
}

/** @brief CRC-16计算 @param data 数据 @return CRC值 */
quint32 CrcStreamVerifier::computeCrc16(const QByteArray& data) const
{
    quint16 crc = 0xFFFF;
    for (char b : data) {
        crc ^= static_cast<quint16>(static_cast<quint8>(b));
        for (int i = 0; i < 8; ++i) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return static_cast<quint32>(crc);
}

/** @brief CRC-16 Modbus计算 @param data 数据 @return CRC值 */
quint32 CrcStreamVerifier::computeCrc16Modbus(const QByteArray& data) const
{
    return computeCrc16(data); /* Modbus使用相同多项式0xA001 */
}

/** @brief CRC-32计算 @param data 数据 @return CRC值 */
quint32 CrcStreamVerifier::computeCrc32(const QByteArray& data) const
{
    quint32 crc = 0xFFFFFFFF;
    for (char b : data) {
        crc ^= static_cast<quint32>(static_cast<quint8>(b));
        for (int i = 0; i < 8; ++i) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xEDB88320;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc ^ 0xFFFFFFFF;
}

/** @brief XOR-8校验 @param data 数据 @return 校验值 */
quint32 CrcStreamVerifier::computeXor8(const QByteArray& data) const
{
    quint8 result = 0;
    for (char b : data) {
        result ^= static_cast<quint8>(b);
    }
    return static_cast<quint32>(result);
}

/** @brief 算术和模256 @param data 数据 @return 校验值 */
quint32 CrcStreamVerifier::computeChecksum8(const QByteArray& data) const
{
    quint32 sum = 0;
    for (char b : data) {
        sum += static_cast<quint8>(b);
    }
    return sum & 0xFF;
}

/** @brief 获取验证历史 @return 最近100条验证结果 */
QList<CrcStreamVerifier::VerifyResult> CrcStreamVerifier::history() const
{
    return m_history;
}

/** @brief 清除缓冲和状态 */
void CrcStreamVerifier::clear()
{
    m_buffer.clear();
    m_blockIndex = 0;
    m_consecutiveFailures = 0;
    m_history.clear();
}

/** @brief 重置所有统计计数器 */
void CrcStreamVerifier::resetStatistics()
{
    m_stats = Stats{};
    m_stats.blockSize = m_blockSize;
}

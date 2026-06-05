/**
 * @file CrcAccelerator.cpp
 * @brief CRC查表加速器实现
 *
 * 预计算256项查表, 支持CRC-8/16/32, 内置标准预设。
 * 查表法将逐位计算替换为逐字节查表, 大幅提升CRC计算速度。
 */

#include "utils/code24/CrcAccelerator.h"

#include <QElapsedTimer>

#include <cstring>

/**
 * @brief 构造函数
 * 默认配置为CRC-32标准多项式
 * @param parent 父对象
 */
CrcAccelerator::CrcAccelerator(QObject* parent)
    : QObject(parent)
{
    setStandard("CRC-32");
}

/**
 * @brief 配置CRC参数
 *
 * 根据指定的宽度、多项式、初始值和最终异或值配置CRC引擎,
 * 然后构建256项查表。查表将每个可能的字节输入映射为对应的CRC余数。
 *
 * @param width CRC位宽(CRC8/CRC16/CRC32)
 * @param polynomial 生成多项式(不含最高位)
 * @param initialValue 初始CRC值
 * @param finalXOR 最终异或值
 */
void CrcAccelerator::configure(CrcWidth width, quint32 polynomial,
                               quint32 initialValue, quint32 finalXOR)
{
    m_width = width;
    m_polynomial = polynomial;
    m_initialValue = initialValue;
    m_finalXOR = finalXOR;
    buildTable();
    reset();
}

/**
 * @brief 设置预定义CRC标准
 *
 * 支持以下标准预设:
 * - CRC-32: 0x04C11DB7, 初始0xFFFFFFFF, 异或0xFFFFFFFF
 * - CRC-32/MPEG-2: 0x04C11DB7, 初始0xFFFFFFFF, 异或0x00000000
 * - CRC-16-CCITT: 0x1021, 初始0xFFFF, 异或0x0000
 * - CRC-16-MODBUS: 0x8005(反射), 初始0xFFFF, 异或0x0000
 * - CRC-16-XMODEM: 0x1021, 初始0x0000, 异或0x0000
 * - CRC-8-MAXIM: 0x31(反射), 初始0x00, 异出0x00
 * - CRC-8-CCITT: 0x07, 初始0x00, 异或0x00
 *
 * @param name 标准名称
 */
void CrcAccelerator::setStandard(const QString& name)
{
    if (name == "CRC-32") {
        configure(CRC32, 0x04C11DB7, 0xFFFFFFFF, 0xFFFFFFFF);
    } else if (name == "CRC-32/MPEG-2") {
        configure(CRC32, 0x04C11DB7, 0xFFFFFFFF, 0x00000000);
    } else if (name == "CRC-16-CCITT") {
        configure(CRC16, 0x1021, 0xFFFF, 0x0000);
    } else if (name == "CRC-16-MODBUS") {
        configure(CRC16, 0x8005, 0xFFFF, 0x0000);
    } else if (name == "CRC-16-XMODEM") {
        configure(CRC16, 0x1021, 0x0000, 0x0000);
    } else if (name == "CRC-8-MAXIM") {
        configure(CRC8, 0x31, 0x00, 0x00);
    } else if (name == "CRC-8-CCITT") {
        configure(CRC8, 0x07, 0x00, 0x00);
    }
}

/**
 * @brief 计算完整数据的CRC值
 *
 * 重置内部状态后, 逐字节查表计算CRC。每个字节通过查表
 * 获得对应的CRC余数, 与当前CRC值进行异或组合。
 *
 * @param data 输入字节数据
 * @return 计算得到的CRC值
 */
quint32 CrcAccelerator::compute(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    reset();
    for (int i = 0; i < data.size(); ++i) {
        update(static_cast<quint8>(data[i]));
    }
    quint32 result = currentValue();

    /* 更新统计 */
    m_stats.totalComputations++;
    m_stats.totalBytesProcessed += data.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;

    emit computationCompleted(result, data.size());
    return result;
}

/**
 * @brief 增量更新CRC — 处理单个字节
 *
 * 查表法: 将当前CRC与输入字节组合后查表,
 * 得到新的CRC值。对于CRC-32, 高8位作为查表索引;
 * 对于CRC-16, 低8位作为索引。
 *
 * @param byte 输入字节
 */
void CrcAccelerator::update(quint8 byte)
{
    switch (m_width) {
    case CRC32: {
        quint8 idx = static_cast<quint8>((m_crc >> 24) ^ byte);
        m_crc = (m_crc << 8) ^ m_table[idx];
        break;
    }
    case CRC16: {
        quint8 idx = static_cast<quint8>((m_crc >> 8) ^ byte);
        m_crc = (m_crc << 8) ^ m_table[idx];
        m_crc &= 0xFFFF;
        break;
    }
    case CRC8: {
        quint8 idx = static_cast<quint8>(m_crc ^ byte);
        m_crc = m_table[idx];
        m_crc &= 0xFF;
        break;
    }
    }
}

/**
 * @brief 重置CRC到初始值
 * 在开始新一轮CRC计算前调用
 */
void CrcAccelerator::reset()
{
    m_crc = m_initialValue;
}

/**
 * @brief 获取CRC比特宽度
 * @return 位宽(8/16/32)
 */
int CrcAccelerator::bitWidth() const
{
    switch (m_width) {
    case CRC8:  return 8;
    case CRC16: return 16;
    case CRC32: return 32;
    }
    return 0;
}

/**
 * @brief 验证数据与期望CRC是否一致
 *
 * 计算输入数据的CRC并与期望值比较, 用于数据完整性校验。
 *
 * @param data 输入数据
 * @param expectedCrc 期望的CRC值
 * @return true=校验通过
 */
bool CrcAccelerator::verify(const QByteArray& data, quint32 expectedCrc)
{
    quint32 computed = compute(data);
    return computed == expectedCrc;
}

/**
 * @brief 重置统计计数器
 * 将计算次数、处理字节数和平均时间归零
 */
void CrcAccelerator::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 构建256项CRC查表
 *
 * 对每个可能的字节值(0~255), 模拟CRC除法过程,
 * 预计算该字节对应的CRC余数存入查表。
 * CRC-32使用MSB-first算法, CRC-16-CCITT使用MSB-first,
 * CRC-8-MAXIM使用反射算法(最低位优先)。
 */
void CrcAccelerator::buildTable()
{
    memset(m_table, 0, sizeof(m_table));

    for (int i = 0; i < 256; ++i) {
        quint32 crc = 0;
        switch (m_width) {
        case CRC32:
            crc = static_cast<quint32>(i) << 24;
            for (int j = 0; j < 8; ++j) {
                if (crc & 0x80000000) {
                    crc = (crc << 1) ^ m_polynomial;
                } else {
                    crc <<= 1;
                }
            }
            break;
        case CRC16:
            crc = static_cast<quint32>(i) << 8;
            for (int j = 0; j < 8; ++j) {
                if (crc & 0x8000) {
                    crc = (crc << 1) ^ m_polynomial;
                } else {
                    crc <<= 1;
                }
            }
            crc &= 0xFFFF;
            break;
        case CRC8:
            crc = static_cast<quint32>(i);
            for (int j = 0; j < 8; ++j) {
                if (crc & 0x80) {
                    crc = (crc << 1) ^ m_polynomial;
                } else {
                    crc <<= 1;
                }
            }
            crc &= 0xFF;
            break;
        }
        m_table[i] = crc;
    }
}

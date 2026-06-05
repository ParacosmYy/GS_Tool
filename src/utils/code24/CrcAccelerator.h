/**
 * @file CrcAccelerator.h
 * @brief CRC硬件加速查表法 — 多字节并行计算
 */
#pragma once

#include <QObject>
#include <QByteArray>
#include <QVector>

/**
 * @brief CRC查表加速器
 * 支持8/16/32位CRC, 预计算256项查表, 多字节并行处理
 */
class CrcAccelerator : public QObject
{
    Q_OBJECT

public:
    /** @brief CRC宽度 */
    enum CrcWidth {
        CRC8, CRC16, CRC32
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalComputations = 0;
        int totalBytesProcessed = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit CrcAccelerator(QObject* parent = nullptr);

    /** @brief 配置CRC参数 @param width 宽度 @param polynomial 多项式 @param initialValue 初始值 @param finalXOR 最终异或值 */
    void configure(CrcWidth width, quint32 polynomial,
                  quint32 initialValue = 0xFFFFFFFF, quint32 finalXOR = 0xFFFFFFFF);

    /** @brief 使用预定义CRC标准 @param name 标准名(CRC-32/CRC-16-CCITT/CRC-8-MAXIM等) */
    void setStandard(const QString& name);

    /** @brief 计算CRC @param data 输入数据 @return CRC值 */
    quint32 compute(const QByteArray& data);

    /** @brief 计算CRC(增量) @param byte 单字节输入 */
    void update(quint8 byte);

    /** @brief 获取当前CRC值 */
    quint32 currentValue() const { return m_crc; }

    /** @brief 重置CRC到初始值 */
    void reset();

    /** @brief 获取CRC比特宽度 */
    int bitWidth() const;

    /** @brief 验证数据+CRC @param data 数据 @param expectedCrc 期望CRC值 */
    bool verify(const QByteArray& data, quint32 expectedCrc);

    /** @brief 获取当前配置的多项式 */
    quint32 polynomial() const { return m_polynomial; }

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void computationCompleted(quint32 crc, int bytes);

private:
    void buildTable();

    CrcWidth m_width = CRC32;
    quint32 m_polynomial = 0x04C11DB7;
    quint32 m_initialValue = 0xFFFFFFFF;
    quint32 m_finalXOR = 0xFFFFFFFF;
    quint32 m_crc = 0;
    quint32 m_table[256];

    Stats m_stats;
    double m_timeSum = 0.0;
};

/**
 * @file ChecksumCalculator.h
 * @brief Calculate CRC8/CRC16/CRC32/checksums for protocol validation
 */
#pragma once
#include <QObject>
#include <QByteArray>
#include <QVector>
#include <QString>
#include <QDateTime>
#include <QMap>

/**
 * @brief Calculate CRC8/CRC16/CRC32/checksums for protocol validation
 */
class ChecksumCalculator : public QObject {
    Q_OBJECT
public:
    /** @brief 支持的校验算法 */
    enum Algorithm {
        CRC8 = 0,
        CRC16Ccitt,
        CRC16Modbus,
        CRC16Kermit,
        CRC32,
        CRC32C,
        Xor8,
        Sum8,
        Sum16,
        Sum32,
        CustomCrc
    };
    Q_ENUM(Algorithm)

    /** @brief Statistics counters */
    struct Stats {
        quint64 operationsPerformed = 0;
        quint64 bytesProcessed = 0;
        quint64 errorsDetected = 0;
        quint64 lastOperationMs = 0;
    };

    explicit ChecksumCalculator(QObject *parent = nullptr);
    ~ChecksumCalculator() override;

    /** @brief Process input data and return result */
    QByteArray process(const QByteArray &input);

    /** @brief 使用指定算法计算校验值 */
    quint64 calculate(const QByteArray& data, Algorithm alg) const;

    /** @brief 使用所有内置算法计算同一份数据 */
    QMap<QString, quint64> calculateAll(const QByteArray& data) const;

    /** @brief 获取算法名称 */
    static QString algorithmName(Algorithm alg);

    /** @brief 获取算法位宽 */
    static int algorithmBitWidth(Algorithm alg);

    /** @brief 获取算法说明 */
    QString algorithmDescription(Algorithm alg);

    /** @brief Reset all statistics */
    void resetStatistics();

    /** @brief 重置校验计算统计 */
    void resetChecksumStatistics();

    /** @brief Get current statistics */
    Stats statistics() const { return m_stats; }

    quint64 totalCalculations() const;
    quint64 totalBytesProcessed() const;
    quint64 totalComputationsByAlgorithm(Algorithm alg) const;

signals:
    /** @brief Emitted when processing completes */
    void processingComplete(const QByteArray &result);
    /** @brief Emitted on error */
    void errorOccurred(const QString &message);

private:
    static quint64 crc8(const QByteArray& data);
    static quint64 crc16Ccitt(const QByteArray& data);
    static quint64 crc16Modbus(const QByteArray& data);
    static quint64 crc16Kermit(const QByteArray& data);
    static quint64 crc32(const QByteArray& data, quint32 polynomial);
    static quint64 xor8(const QByteArray& data);
    static quint64 sumBytes(const QByteArray& data, int widthBits);

    Stats m_stats;
    mutable quint64 m_totalCalculations = 0;
    mutable quint64 m_totalBytesProcessed = 0;
    mutable quint64 m_totalCustomCalculations = 0;
    mutable quint64 m_totalCalculateAllCalls = 0;
    mutable QMap<int, quint64> m_algorithmCounts;
};


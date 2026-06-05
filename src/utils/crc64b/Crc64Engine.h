/**
 * @file Crc64Engine.h
 * @brief CRC-64引擎 — 64位循环冗余校验
 *
 * 功能: 支持CRC-64/ECMA和CRC-64/WE多项式，查表加速，
 *       流式接口支持大数据分块校验。
 */
#ifndef CRC64ENGINE_H
#define CRC64ENGINE_H

#include <QObject>
#include <QByteArray>
#include <QMap>

/**
 * @class Crc64Engine
 * @brief 64位CRC计算引擎
 */
class Crc64Engine : public QObject {
    Q_OBJECT
public:
    /** CRC多项式类型 */
    enum class Polynomial {
        ECMA_182,   ///< CRC-64/ECMA-182
        WE,         ///< CRC-64/WE
        ISO         ///< CRC-64/ISO
    };

    /** 计算统计 */
    struct Stats {
        quint64 totalChecksums = 0;
        quint64 totalBytesProcessed = 0;
        double  averageProcessingTimeMs = 0.0;
    };

    explicit Crc64Engine(QObject* parent = nullptr);

    void setPolynomial(Polynomial poly);

    /** 计算完整数据的CRC */
    quint64 compute(const QByteArray& data);

    /** 流式: 开始/更新/结束 */
    void begin();
    void update(const QByteArray& data);
    quint64 end();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void checksumComputed(quint64 crc, int dataSize);

private:
    void buildTable();

    Polynomial m_polynomial;
    quint64 m_table[256];
    quint64 m_runningCrc;
    bool m_streaming;
    Stats m_stats;
    double m_timeSum;
};

#endif // CRC64ENGINE_H

/**
 * @file RunLengthEncoder2.h
 * @brief 增强型游程编码器 — 多模式RLE压缩/解压
 *
 * 功能: 字节游程编码、位游程编码、PackBits解码，
 *       支持阈值控制、统计压缩率/耗时。
 */
#ifndef RUNLENGTHENCODER2_H
#define RUNLENGTHENCODER2_H

#include <QObject>
#include <QByteArray>
#include <QVector>

class RunLengthEncoder2 : public QObject {
    Q_OBJECT
public:
    /** RLE模式 */
    enum class Mode {
        ByteRun,   ///< 字节游程: [count|value]
        BitRun,    ///< 位游程: 按位压缩
        PackBits   ///< PackBits格式解码
    };
    Q_ENUM(Mode)

    /** 统计 */
    struct Stats {
        quint64 totalEncodes = 0;
        quint64 totalDecodes = 0;
        quint64 totalBytesIn = 0;
        quint64 totalBytesOut = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit RunLengthEncoder2(QObject* parent = nullptr);

    /** @brief 字节游程编码 @param data 输入数据 @return 编码结果 */
    QByteArray encodeByteRun(const QByteArray& data);

    /** @brief 字节游程解码 @param data 编码数据 @return 原始数据 */
    QByteArray decodeByteRun(const QByteArray& data);

    /** @brief 位游程编码 @param data 输入数据 @return 编码结果 */
    QByteArray encodeBitRun(const QByteArray& data);

    /** @brief 位游程解码 @param data 编码数据 @return 原始数据 */
    QByteArray decodeBitRun(const QByteArray& data);

    /** @brief PackBits解码 @param data PackBits格式数据 @return 解码数据 */
    QByteArray decodePackBits(const QByteArray& data);

    /** @brief 编码 @param data 数据 @param mode 模式 @return 结果 */
    QByteArray encode(const QByteArray& data, Mode mode);

    /** @brief 解码 @param data 数据 @param mode 模式 @return 结果 */
    QByteArray decode(const QByteArray& data, Mode mode);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encoded(int inputSize, int outputSize);
    void decoded(int inputSize, int outputSize);

private:
    Stats m_stats;
    double m_timeSum;
};

#endif // RUNLENGTHENCODER2_H

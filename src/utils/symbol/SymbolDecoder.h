/**
 * @file SymbolDecoder.h
 * @brief 符号解码引擎 — NRZ/NRZI/Manchester/4B5B/差分编码
 *
 * 功能: 5种线路编码的解码，从采样信号中恢复数字比特流，
 *       支持时钟恢复和同步检测。
 *
 * 协作: Demodulator(解调后解码) / EyeDiagramEngine(眼图分析)
 */
#ifndef SYMBOLDECODER_H
#define SYMBOLDECODER_H

#include <QObject>
#include <QVector>

class SymbolDecoder : public QObject {
    Q_OBJECT

public:
    /** @brief 编码类型 */
    enum class Encoding {
        NRZ,            ///< 不归零编码
        NRZI,           ///< 不归零反转
        Manchester,     ///< Manchester编码
        DiffManchester, ///< 差分Manchester
        RZ              ///< 归零编码
    };
    Q_ENUM(Encoding)

    /** @brief 统计 */
    struct Stats {
        quint64 totalBitsDecoded = 0;
        quint64 totalErrors = 0;
        quint64 totalSyncLosses = 0;
        double  bitErrorRate = 0.0;
    };

    explicit SymbolDecoder(QObject* parent = nullptr);

    void setEncoding(Encoding enc);
    void setThreshold(double threshold);

    /** @brief 解码信号为比特流 @param data 采样信号 @return 比特序列 */
    QVector<int> decode(const QVector<double>& data);

    /** @brief 从Manchester信号恢复时钟 @param data 信号 @return 时钟边沿位置 */
    QVector<int> recoverClock(const QVector<double>& data);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decoded(int bitCount);

private:
    QVector<int> decodeNRZ(const QVector<double>& data) const;
    QVector<int> decodeNRZI(const QVector<double>& data) const;
    QVector<int> decodeManchester(const QVector<double>& data) const;
    QVector<int> decodeDiffManchester(const QVector<double>& data) const;
    QVector<int> decodeRZ(const QVector<double>& data) const;

    Encoding m_encoding;
    double m_threshold;
    Stats m_stats;
};

#endif // SYMBOLDECODER_H

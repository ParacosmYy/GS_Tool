#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief Reed-Solomon纠错码编解码器
 *
 * 基于有限域运算的RS码实现，适用于突发错误纠错。
 */
class ReedSolomon8 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalBlocksEncoded = 0;
        int totalBlocksDecoded = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ReedSolomon8(QObject* parent = nullptr);

    /** @brief 编码数据，添加nSym个校验符号 */
    QVector<int> encode(const QVector<int>& data, int nSym);

    /** @brief 解码并纠正错误(最多nSym/2个错误符号) */
    QVector<int> decode(const QVector<int>& codeword, int nSym);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decodeCompleted(int blockIndex, int errorsCorrected);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_fieldSize = 256;
};

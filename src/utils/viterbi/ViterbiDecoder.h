/**
 * @file ViterbiDecoder.h
 * @brief 维特比解码器 — 卷积码最优解码
 *
 * 功能: 实现Viterbi算法用于卷积码解码，支持可配置约束长度/
 *       生成多项式/软硬判决，统计解码次数/纠正比特/耗时。
 */
#ifndef VITERBIDECODER_H
#define VITERBIDECODER_H

#include <QObject>
#include <QVector>

class ViterbiDecoder : public QObject {
    Q_OBJECT
public:
    /** 解码统计 */
    struct Stats {
        quint64 totalDecodings = 0;
        quint64 totalBitsCorrected = 0;
        quint64 totalBitsProcessed = 0;
        double  averageProcessingTimeMs = 0.0;
    };

    /** @brief 构造 @param constraintLength 约束长度K @param generators 生成多项式(八进制) @param parent 父对象 */
    explicit ViterbiDecoder(int constraintLength = 7,
                            const QVector<int>& generators = {0171, 0133},
                            QObject* parent = nullptr);

    /** @brief 编码 @param input 输入比特流 @return 编码输出 */
    QVector<int> encode(const QVector<int>& input) const;

    /** @brief 硬判决解码 @param received 接收比特流 @return 解码比特流 */
    QVector<int> decodeHard(const QVector<int>& received);

    /** @brief 软判决解码 @param softReceived 软判决值(正≈1, 负≈0) @return 解码比特流 */
    QVector<int> decodeSoft(const QVector<double>& softReceived);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decodingCompleted(int inputBits, int outputBits, int corrections);

private:
    /** 计算分支度量(硬) */
    int branchMetricHard(int state, int input, const QVector<int>& received) const;
    /** 计算分支度量(软) */
    double branchMetricSoft(int state, int input,
                            const QVector<double>& softReceived) const;
    /** 获取编码输出 */
    QVector<int> getOutput(int state, int input) const;
    /** 回溯 */
    QVector<int> traceback(const QVector<QVector<int>>& paths,
                           int finalState, int length) const;

    int m_constraintLength;
    QVector<int> m_generators;
    int m_numStates;
    int m_rate; ///< 1/r码率

    Stats m_stats;
    double m_timeSum;
};

#endif // VITERBIDECODER_H

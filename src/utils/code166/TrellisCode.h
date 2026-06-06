/**
 * @file TrellisCode.h
 * @brief 网格编码调制(Ungerboeck分割+Viterbi译码) — Trellis-Coded Modulation with Ungerboeck Partitioning and Viterbi Decoding
 *
 * 功能: 实现网格编码调制(TCM)，支持Ungerboeck集合分割、卷积编码器
 *       和Viterbi算法最优序列检测，适用于AWGN信道下的带宽高效传输。
 *
 * 协作: ViterbiCodec(维特比译码) / ConvolutionalCode(卷积码) / QamModulator(QAM调制)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 网格编码调制器/解调器
 */
class TrellisCode : public QObject {
    Q_OBJECT

public:
    /** @brief 调制星座类型 */
    enum Modulation {
        PSK8,   ///< 8-PSK
        QAM16   ///< 16-QAM
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalEncodes = 0;          ///< 累计编码次数
        quint64 totalDecodes = 0;          ///< 累计译码次数
        double avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
        int lastBlockSize = 0;             ///< 最近块大小
    };

    explicit TrellisCode(QObject* parent = nullptr);
    ~TrellisCode() override;

    /** @brief 设置调制方式 */
    void setModulation(Modulation mod);

    /** @brief 设置约束长度 */
    void setConstraintLength(int k);

    /**
     * @brief TCM编码
     * @param bits 输入比特流
     * @return 编码后符号序列(复数:I,Q对)
     */
    QVector<QPair<double, double>> encode(const QVector<int>& bits);

    /**
     * @brief Viterbi译码
     * @param symbols 接收符号序列(I,Q对)
     * @return 译码比特流
     */
    QVector<int> decode(const QVector<QPair<double, double>>& symbols);

    /** @brief 获取网格分支数 */
    int trellisStates() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 编码完成 @param symbolCount 符号数 */
    void encodeCompleted(int symbolCount);
    /** @brief 译码完成 @param bitCount 比特数 */
    void decodeCompleted(int bitCount);

private:
    /** @brief Ungerboeck集合分割(8-PSK) */
    void partition8PSK();

    /** @brief 卷积编码一个比特对 */
    int convolutionalEncode(int state, int input) const;

    /** @brief 计算欧氏距离度量 */
    double branchMetric(const QPair<double, double>& received, int symbolIdx) const;

    /** @brief 计算两个星座点间的欧氏距离 */
    static double symbolDistance(const QPair<double, double>& a,
                                const QPair<double, double>& b);

    Modulation m_mod = PSK8;
    int m_constraintLen = 3;
    int m_numStates = 4;

    /* 8-PSK constellation: (I, Q) pairs */
    QVector<QPair<double, double>> m_constellation;

    /* Trellis: nextState[state][input], outputSymbol[state][input] */
    QVector<QVector<int>> m_nextState;
    QVector<QVector<int>> m_outputSymbol;

    /* Partition mapping */
    QVector<QVector<int>> m_partitions; ///< subsets of symbol indices

    Stats m_stats;
    double m_timeSum = 0.0;
};

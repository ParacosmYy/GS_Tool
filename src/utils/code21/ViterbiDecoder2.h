/**
 * @file ViterbiDecoder2.h
 * @brief 软判决维特比解码器 — 分支度量计算+幸存路径管理+SOVA软输出
 *
 * 功能: 支持 AWGN/PAM 信道的软判决维特比解码，包含幸存路径管理、
 *       SOVA 软输出计算、滑动窗回溯，适用于卷积码译码。
 *
 * 协作: TurboCode2(级联码) / DataClassifier(符号判决)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>

/**
 * @brief 软判决维特比解码器
 *
 * 通过网格图搜索最大似然路径，支持硬判决和软判决输出。
 * 滑动窗回溯控制延迟，SOVA模式输出软信息(可靠性)。
 */
class ViterbiDecoder2 : public QObject {
    Q_OBJECT

public:
    /** @brief 信道模型 */
    enum class ChannelModel {
        AWGN,       ///< 加性高斯白噪声
        PAM         ///< 脉冲幅度调制
    };
    Q_ENUM(ChannelModel)

    /** @brief 解码结果 */
    struct DecodeResult {
        QVector<int> hardBits;                  ///< 硬判决比特
        QVector<double> softOutputs;            ///< SOVA软输出(可靠性)
        double metric = 0.0;                    ///< 最终路径度量
    };

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalDecodedBlocks = 0;     ///< 累计解码块数
        quint64 totalBitsDecoded = 0;       ///< 累计解码比特数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理时间(ms)
        double  avgPathMetric = 0.0;        ///< 平均路径度量
    };

    explicit ViterbiDecoder2(QObject* parent = nullptr);

    void setConstraintLength(int K);
    void setGeneratorPolynomials(const QVector<int>& polys);
    void setTracebackDepth(int depth);
    void setChannelModel(ChannelModel model);
    void setSovaEnabled(bool enable);

    DecodeResult decode(const QVector<double>& received);
    DecodeResult decodePam(const QVector<double>& symbols, int bitsPerSymbol);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void blockDecoded(int bits, double metric);

private:
    void buildTrellis();
    double branchMetric(const QVector<double>& received, int state, int input) const;
    double branchMetricPam(double symbol, int state, int input, int bitsPerSymbol) const;
    QVector<int> outputBits(int state, int input) const;
    void traceback(DecodeResult& result);

    int m_constraintLength;             ///< 约束长度K
    QVector<int> m_polynomials;         ///< 生成多项式
    int m_tracebackDepth;               ///< 回溯深度
    ChannelModel m_channelModel;        ///< 信道模型
    bool m_sovaEnabled;                 ///< SOVA软输出使能

    int m_numStates;                    ///< 状态数 = 2^(K-1)
    int m_rate;                         ///< 码率倒数(输出比特数)

    /* 网格结构 */
    QVector<QVector<int>> m_nextState;  ///< [state][input] -> 下一状态
    QVector<QVector<QVector<int>>> m_output; ///< [state][input] -> 输出比特

    /* ACS 存活路径 */
    QVector<double> m_pathMetric;       ///< 当前路径度量
    QVector<QVector<int>> m_survivors;  ///< 幸存路径历史

    double m_metricSum;                 ///< 度量累积用于统计

    Stats m_stats;
    double m_timeSum = 0.0;
};

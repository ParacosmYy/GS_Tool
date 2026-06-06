/**
 * @file ConvolutionalCode6.h
 * @brief 卷积码Viterbi译码器(软判决+回溯) — Convolutional Code Viterbi Decoder with Trace-back and Soft Decisions
 *
 * 功能: 实现卷积码的Viterbi最大似然译码，支持硬/软判决输入、
 *       多项式生成矩阵配置和回溯式路径选择。
 *
 * 协作: ReedSolomon14(RS码) / TurboCode(迭代译码) / CrcCalculator(CRC)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 卷积码Viterbi译码器
 */
class ConvolutionalCode6 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalDecodes = 0;          ///< 累计译码次数
        quint64 totalBitErrors = 0;        ///< 累计纠正比特错误
        double avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
        double lastPathMetric = 0.0;       ///< 最近最优路径度量
    };

    explicit ConvolutionalCode6(QObject* parent = nullptr);
    ~ConvolutionalCode6() override;

    /**
     * @brief 配置卷积码参数
     * @param constraintLength 约束长度(K)
     * @param generators 生成多项式(八进制表示)
     */
    void configure(int constraintLength, const QVector<int>& generators);

    /** @brief 设置回溯深度 */
    void setTracebackDepth(int depth);

    /**
     * @brief Viterbi译码(软判决)
     * @param softBits 软判决输入，每帧含 n*(trellisLen) 个值
     * @return 译码后信息比特序列
     */
    QVector<int> decodeSoft(const QVector<double>& softBits);

    /**
     * @brief Viterbi译码(硬判决)
     * @param hardBits 硬判决输入(0/1)
     * @return 译码后信息比特序列
     */
    QVector<int> decodeHard(const QVector<int>& hardBits);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 译码完成 @param frameLen 帧长度 */
    void decodeCompleted(int frameLen);

private:
    /** @brief 计算下一个状态 */
    int nextState(int state, int input) const;

    /** @brief 获取输出符号 */
    QVector<int> outputBits(int state, int input) const;

    /** @brief Viterbi核心ACS(加-比-选) */
    QVector<int> viterbiACS(const QVector<QVector<double>>& metrics);

    /** @brief 回溯路径 */
    QVector<int> traceback(const QVector<QVector<int>>& survivors,
                           int trellisLen);

    int m_constraintLength = 7;
    QVector<int> m_generators;
    int m_nOutput = 2;
    int m_nStates = 64;
    int m_tracebackDepth = 30;

    Stats m_stats;
    double m_timeSum = 0.0;
};

/**
 * @file ViterbiDecoder.h
 * @brief Viterbi算法解码器 — 网格构造 + ACS + 回溯 + 软/硬判决
 *
 * 功能: 完整的Viterbi解码实现，包括网格构造、加-比较-选择(ACS)、
 *       回溯路径搜索。支持可配置约束长度、任意生成多项式、
 *       软/硬判决度量。统计解码次数/纠正比特数/平均耗时。
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @class ViterbiDecoder
 * @brief Viterbi卷积码解码器
 */
class ViterbiDecoder : public QObject {
    Q_OBJECT
public:
    /** 解码统计 */
    struct Stats {
        quint64 totalDecodings = 0;        ///< 总解码次数
        quint64 totalBitsCorrected = 0;    ///< 累计纠正比特数
        quint64 totalBitsProcessed = 0;    ///< 累计处理比特数
        quint64 totalTracebacks = 0;       ///< 累计回溯次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /** 网格节点 */
    struct TrellisNode {
        int prevState;        ///< 前驱状态
        int inputBit;         ///< 对应输入比特
        double pathMetric;    ///< 路径度量
    };

    /**
     * @brief 构造函数
     * @param constraintLength 约束长度K(3~9)
     * @param generators 生成多项式(八进制表示)
     * @param parent 父对象
     */
    explicit ViterbiDecoder(int constraintLength = 7,
                            const QVector<int>& generators = {0171, 0133},
                            QObject* parent = nullptr);

    /** @brief 卷积编码 @param input 输入比特流 @return 编码输出 */
    QVector<int> encode(const QVector<int>& input) const;

    /** @brief 硬判决解码 @param received 硬判决接收比特 @return 解码比特 */
    QVector<int> decodeHard(const QVector<int>& received);

    /** @brief 软判决解码 @param softReceived 软判决值(正≈1,负≈0) @return 解码比特 */
    QVector<int> decodeSoft(const QVector<double>& softReceived);

    /** @brief 获取网格深度(最近一次解码) */
    int lastTrellisDepth() const { return m_lastDepth; }

    /** @brief 获取编码码率(1/r) */
    int rate() const { return m_rate; }

    /** @brief 获取状态数 */
    int numStates() const { return m_numStates; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 解码完成 @param inputBits 输入位数 @param outputBits 输出位数 @param corrections 纠正数 */
    void decodingCompleted(int inputBits, int outputBits, int corrections);

private:
    /** 构建网格(ACS过程) — 硬判决 */
    QVector<QVector<TrellisNode>> buildTrellisHard(
        const QVector<int>& received, int depth) const;
    /** 构建网格(ACS过程) — 软判决 */
    QVector<QVector<TrellisNode>> buildTrellisSoft(
        const QVector<double>& softReceived, int depth) const;
    /** 回溯 */
    QVector<int> traceback(const QVector<QVector<TrellisNode>>& trellis,
                           int depth) const;
    /** 计算硬判决分支度量 */
    int branchMetricHard(int state, int input,
                         const QVector<int>& recv) const;
    /** 计算软判决分支度量 */
    double branchMetricSoft(int state, int input,
                            const QVector<double>& recv) const;
    /** 获取编码器输出 */
    QVector<int> getEncoderOutput(int state, int input) const;

    int           m_constraintLength; ///< 约束长度
    QVector<int>  m_generators;       ///< 生成多项式
    int           m_numStates;        ///< 状态数 = 2^(K-1)
    int           m_rate;             ///< 1/r码率
    int           m_lastDepth;        ///< 最近网格深度

    Stats  m_stats;         ///< 统计信息
    double m_timeSum = 0.0; ///< 累计耗时
};

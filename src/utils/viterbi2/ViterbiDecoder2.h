/**
 * @file ViterbiDecoder2.h
 * @brief 维特比解码器增强版(Viterbi Decoder Enhanced)
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @class ViterbiDecoder2
 * @brief 维特比解码器增强版 — 支持软判决和回溯路径
 *
 * 支持硬/软判决解码、状态转移追踪、路径度量排序。
 * 适用于卷积码译码、隐马尔可夫模型解码等场景。
 */
class ViterbiDecoder2 : public QObject
{
    Q_OBJECT

public:
    /** @brief 状态转移 */
    struct Transition {
        int fromState;      /**< 源状态 */
        int toState;        /**< 目标状态 */
        int output;         /**< 输出符号 */
        int input;          /**< 输入符号 */
    };

    /** @brief 解码结果 */
    struct DecodeResult {
        QVector<int> bits;          /**< 解码比特序列 */
        double pathMetric;          /**< 路径度量 */
        int bitErrors;              /**< 估计错误比特数 */
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalDecoded = 0;       /**< 总解码次数 */
        int totalBits = 0;          /**< 总比特数 */
        int totalErrors = 0;        /**< 总估计错误数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 构造函数 */
    explicit ViterbiDecoder2(QObject* parent = nullptr);

    /**
     * @brief 设置网格参数
     * @param constraintLength 约束长度
     * @param polynomials 生成多项式列表(八进制表示)
     */
    void setup(int constraintLength, const QVector<int>& polynomials);

    /**
     * @brief 硬判决解码
     * @param received 接收的硬判决序列
     * @return 解码结果
     */
    DecodeResult decodeHard(const QVector<int>& received);

    /**
     * @brief 软判决解码
     * @param received 接收的软判决值列表
     * @return 解码结果
     */
    DecodeResult decodeSoft(const QVector<double>& received);

    /**
     * @brief 获取所有幸存路径
     * @return 每个状态的路径度量和回溯信息
     */
    QVector<QPair<int, double>> survivorPaths() const;

    /** @brief 获取网格状态数 */
    int stateCount() const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 解码完成信号 */
    void decodingCompleted(int bitCount, double metric);

private:
    int nextTransition(int state, int input) const;
    int outputSymbol(int state, int input) const;
    double branchMetric(int symbol, int expected) const;
    double softBranchMetric(double received, int expected) const;

    int m_constraintLength;
    int m_stateCount;
    int m_rate;  /**< 1/rate */
    QVector<int> m_polynomials;
    QVector<QVector<Transition>> m_transitions;
    QVector<QPair<int, double>> m_survivors;

    Stats m_stats;
    double m_timeSum;
};

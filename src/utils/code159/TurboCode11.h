/**
 * @file TurboCode11.h
 * @brief Turbo码编解码器 — Turbo Code Encoder/Decoder
 *
 * 功能: 支持SOVA/MaxLogMAP/LogMAP三种SISO解码算法，可配置约束长度
 *       与码率。适用于高可靠性通信场景的前向纠错编码。
 *
 * 协作: ConvolutionalCode(卷积码) / ViterbiDecoder(维特比解码)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Turbo码编解码器，支持三种SISO解码模式
 */
class TurboCode11 : public QObject {
    Q_OBJECT

public:
    /** @brief SISO解码模式 */
    enum class DecodeMode {
        SOVA,       ///< Soft Output Viterbi Algorithm
        MaxLogMAP,  ///< Max-Log-MAP近似
        LogMAP      ///< Log-MAP精确对数域
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalEncoded = 0;           ///< 累计编码块数
        quint64 totalDecoded = 0;           ///< 累计解码块数
        quint64 bitErrors = 0;              ///< 累计比特错误数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
    };

    explicit TurboCode11(QObject* parent = nullptr);

    /**
     * @brief 设置解码模式
     * @param mode SISO解码算法
     */
    void setDecodeMode(DecodeMode mode);

    /**
     * @brief 设置Turbo解码迭代次数
     * @param iterations 迭代次数，默认6
     */
    void setIterations(int iterations);

    /**
     * @brief 设置约束长度(生成多项式阶数+1)
     * @param length 约束长度，常见值3~7
     */
    void setConstraintLength(int length);

    /**
     * @brief Turbo编码
     * @param bits 输入信息比特序列(0/1)
     * @return 编码后比特序列(含系统位+校验位+交织校验位)
     */
    QVector<int> encode(const QVector<int>& bits);

    /**
     * @brief Turbo解码
     * @param received 接收到的软判决序列
     * @return 解码后的硬判决比特序列
     */
    QVector<int> decode(const QVector<double>& received);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 编码完成 @param blockSize 信息块长度 */
    void encodeCompleted(int blockSize);
    /** @brief 解码完成 @param blockSize 块长度 @param bitErrs 本次比特错误 */
    void decodeCompleted(int blockSize, int bitErrs);

private:
    /** @brief 生成伪随机交织表 */
    void generateInterleaver(int length);

    /** @brief 单个RSC编码器(递归系统卷积) */
    QVector<int> rscEncode(const QVector<int>& bits) const;

    /** @brief SOVA SISO解码 */
    QVector<double> sisoDecode(const QVector<double>& systematic,
                               const QVector<double>& parity,
                               const QVector<double>& extrinsic) const;

    /** @brief Max-Log-MAP SISO解码 */
    QVector<double> maxLogMapDecode(const QVector<double>& systematic,
                                    const QVector<double>& parity,
                                    const QVector<double>& extrinsic) const;

    /** @brief Log-MAP SISO解码(含修正项) */
    QVector<double> logMapDecode(const QVector<double>& systematic,
                                 const QVector<double>& parity,
                                 const QVector<double>& extrinsic) const;

    /** @brief log-sum-exp运算 */
    static double logSumExp(double a, double b);

    DecodeMode m_mode = DecodeMode::MaxLogMAP;
    int m_iterations = 6;
    int m_constraintLength = 3;
    QVector<int> m_interleaver;
    QVector<int> m_generator = {1, 1, 1};  ///< 默认(7,5)八进制

    Stats m_stats;
    double m_timeSum = 0.0;
};

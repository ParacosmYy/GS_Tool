/**
 * @file TurboCode2.h
 * @brief Turbo码编解码器 — 并行级联RSC + MAP/Log-MAP/SOVA译码
 *
 * 功能: 实现Turbo码编码器和迭代译码器，支持可配置约束长度、
 *       交织器设计、MAP/Log-MAP/SOVA三种译码算法、外信息交换。
 *       适用于通信系统前向纠错、深空通信、3G/4G链路仿真。
 *
 * 协作: ConvolutionalCode2(卷积码) / CrcStreamVerifier(校验)
 */
#pragma once

#include <QObject>
#include <QVector>

#include <vector>

/**
 * @brief Turbo码编解码器 — 并行级联卷积码
 *
 * 编码: 两个并行RSC编码器 + 交织器
 * 译码: 迭代MAP/Log-MAP/SOVA, 外信息交换
 */
class TurboCode2 : public QObject {
    Q_OBJECT

public:
    /** @brief 译码算法 */
    enum DecodingAlgorithm {
        MAP = 0,            ///< MAP(最大后验概率)
        LogMAP = 1,         ///< Log-MAP(对数域,数值稳定)
        SOVA = 2            ///< SOVA(软输出Viterbi)
    };
    Q_ENUM(DecodingAlgorithm)

    /** @brief 编码器配置 */
    struct Config {
        int constraintLength = 3;          ///< 约束长度K(典型3~5)
        QVector<int> generators;           ///< 生成多项式(八进制,默认[7,5])
        int interleaverSize = 256;         ///< 交织器大小
        int maxIterations = 8;             ///< 最大迭代次数
        DecodingAlgorithm algorithm = LogMAP; ///< 译码算法
        double earlyStopThreshold = 1e-4;  ///< 提前终止阈值(LLR变化)
        bool enableTermination = true;     ///< 是否添加尾比特
    };

    /** @brief 译码结果 */
    struct DecodeResult {
        QByteArray decoded;                ///< 译码后数据
        QVector<double> llr;               ///< 最终LLR(对数似然比)
        int iterations = 0;                ///< 实际迭代次数
        double finalMetric = 0.0;          ///< 最终路径度量
        bool converged = false;            ///< 是否提前收敛
        bool success = false;              ///< 是否成功
    };

    /** @brief 操作统计 */
    struct Stats {
        quint64 totalEncodes = 0;          ///< 累计编码次数
        quint64 totalDecodes = 0;          ///< 累计译码次数
        quint64 totalBitsEncoded = 0;      ///< 累计编码比特数
        quint64 totalBitsDecoded = 0;      ///< 累计译码比特数
        quint64 totalIterations = 0;       ///< 累计迭代次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit TurboCode2(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~TurboCode2() override;

    // ── 配置 ──

    /** @brief 配置编解码参数 @param config 配置 */
    void configure(const Config& config);

    /** @brief 获取当前配置 @return 配置 */
    Config configuration() const;

    // ── 编码 ──

    /**
     * @brief Turbo编码(比特流)
     * @param bits 输入比特(0/1)
     * @return 编码后比特(系统位+校验1+校验2)
     */
    QVector<int> encodeBits(const QVector<int>& bits);

    /**
     * @brief Turbo编码(字节数据)
     * @param data 输入字节数据
     * @return 编码后比特
     */
    QVector<int> encodeBytes(const QByteArray& data);

    // ── 译码 ──

    /**
     * @brief 迭代译码(软输入)
     * @param systematic 系统位软值
     * @param parity1 校验1软值
     * @param parity2 校验2软值
     * @return 译码结果
     */
    DecodeResult decode(const QVector<double>& systematic,
                        const QVector<double>& parity1,
                        const QVector<double>& parity2);

    // ── 辅助 ──

    /** @brief 获取交织索引 @return 交织映射表 */
    QVector<int> interleaver() const;

    /** @brief 获取编码速率 @return 速率(如3表示rate 1/3) */
    int rate() const;

    // ── 统计 ──

    /** @brief 获取统计 @return 统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 编码完成 @param inputBits 输入比特数 @param outputBits 输出比特数 */
    void encodeCompleted(int inputBits, int outputBits);

    /** @brief 译码迭代 @param iter 当前迭代 @param metric 路径度量 */
    void decodeIteration(int iter, double metric);

    /** @brief 译码完成 @param decodedBytes 译码字节数 @param iterations 迭代次数 */
    void decodeCompleted(int decodedBytes, int iterations);

private:
    /** @brief RSC编码一步 @param state 当前状态 @param inputBit 输入比特 @return (新状态,校验输出) */
    QPair<int, int> rscEncode(int state, int inputBit) const;

    /** @brief RSC网格转移 @param fromState 起始状态 @param input 输入 @return (nextState,parity) */
    QPair<int, int> rscTransition(int fromState, int input) const;

    /** @brief MAP算法译码 @param sys 系统位LLR @param parity 校验LLR @param extrinsic 外信息 @return (后验LLR,新外信息) */
    QPair<QVector<double>, QVector<double>> decodeMAP(
        const QVector<double>& sys, const QVector<double>& parity,
        const QVector<double>& extrinsic) const;

    /** @brief Log-MAP译码 @param sys 系统位LLR @param parity 校验LLR @param extrinsic 外信息 @return (后验LLR,新外信息) */
    QPair<QVector<double>, QVector<double>> decodeLogMAP(
        const QVector<double>& sys, const QVector<double>& parity,
        const QVector<double>& extrinsic) const;

    /** @brief SOVA译码 @param sys 系统位LLR @param parity 校验LLR @param extrinsic 外信息 @return (后验LLR,新外信息) */
    QPair<QVector<double>, QVector<double>> decodeSOVA(
        const QVector<double>& sys, const QVector<double>& parity,
        const QVector<double>& extrinsic) const;

    /** @brief 生成伪随机交织器 @param size 大小 @return 交织索引表 */
    QVector<int> generateInterleaver(int size) const;

    /** @brief log(sum(exp(a),exp(b))) 数值稳定计算 */
    double logSumExp(double a, double b) const;

    Config m_config;                     ///< 编解码配置
    QVector<int> m_interleaver;          ///< 交织索引表
    QVector<int> m_deinterleaver;        ///< 解交织索引表
    int m_numStates;                     ///< RSC状态数
    int m_mask;                          ///< 状态掩码

    Stats m_stats;                       ///< 操作统计
    double m_timeSum = 0.0;              ///< 累计耗时
};

/**
 * @file ConvolutionalCode2.h
 * @brief 卷积码编解码器 — Viterbi硬/软判决译码
 *
 * 功能: 实现卷积码编码器和Viterbi译码器，支持可配置约束长度K、
 *       生成多项式、硬判决和软判决译码。适用于通信系统前向纠错、
 *       串口协议可靠性增强、无线数据链路仿真。
 *
 * 协作: CrcStreamVerifier(校验) / DataCompressor(压缩后纠错)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 卷积码编解码器 — Viterbi算法
 *
 * 编码: rate 1/R, 约束长度K, R个生成多项式
 * 译码: Viterbi硬/软判决, 网格图回溯
 */
class ConvolutionalCode2 : public QObject {
    Q_OBJECT

public:
    /** @brief 编码器配置 */
    struct Config {
        int constraintLength = 7;        ///< 约束长度K
        QVector<int> generators;         ///< 生成多项式(八进制)
        bool enableTermination = true;    ///< 是否添加终止比特
    };

    /** @brief 译码结果 */
    struct DecodeResult {
        QByteArray decoded;              ///< 译码后数据
        double pathMetric = 0.0;         ///< 最优路径度量
        int bitErrors = 0;              ///< 估计比特错误数
        bool success = false;           ///< 是否成功
    };

    /** @brief 操作统计 */
    struct Stats {
        quint64 totalEncodes = 0;        ///< 总编码次数
        quint64 totalDecodes = 0;        ///< 总译码次数
        quint64 totalBitsEncoded = 0;    ///< 总编码比特数
        quint64 totalBitsDecoded = 0;    ///< 总译码比特数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit ConvolutionalCode2(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~ConvolutionalCode2() override;

    // ── 配置 ──

    /**
     * @brief 配置编码参数
     * @param config 编码器配置
     */
    void configure(const Config& config);

    /**
     * @brief 获取当前配置
     * @return 当前配置
     */
    Config configuration() const;

    // ── 编码 ──

    /**
     * @brief 编码比特流
     * @param bits 输入比特(0/1)
     * @return 编码后比特(0/1)
     */
    QVector<int> encodeBits(const QVector<int>& bits);

    /**
     * @brief 编码字节数据
     * @param data 输入字节数据
     * @return 编码后比特
     */
    QVector<int> encodeBytes(const QByteArray& data);

    // ── 译码 ──

    /**
     * @brief Viterbi硬判决译码
     * @param encodedBits 接收到的硬比特(0/1)
     * @return 译码结果
     */
    DecodeResult decodeHard(const QVector<int>& encodedBits);

    /**
     * @brief Viterbi软判决译码
     * @param softValues 接收到的软判决值(整数, 如±1, ±3, ±7)
     * @param precision 软判决量化位数(默认3)
     * @return 译码结果
     */
    DecodeResult decodeSoft(const QVector<int>& softValues, int precision = 3);

    // ── 辅助 ──

    /**
     * @brief 获取编码速率(输出/输入比特比)
     * @return 速率(如2表示rate 1/2)
     */
    int rate() const;

    /**
     * @brief 获取状态数(2^(K-1))
     * @return 状态数
     */
    int stateCount() const;

    // ── 统计 ──

    /** @brief 获取当前统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 编码完成 @param inputBits 输入比特数 @param outputBits 输出比特数 */
    void encodeCompleted(int inputBits, int outputBits);
    /** @brief 译码完成 @param decodedBytes 译码字节数 @param metric 路径度量 */
    void decodeCompleted(int decodedBytes, double metric);

private:
    /**
     * @brief 网格图一步转移计算
     * @param fromState 起始状态
     * @param inputBit 输入比特
     * @return (nextState, 输出比特向量)
     */
    QPair<int, QVector<int>> transition(int fromState, int inputBit) const;

    /**
     * @brief 计算分支度量(硬判决)
     * @param received 接收比特
     * @param expected 期望输出
     * @return 汉明距离
     */
    int branchMetricHard(const QVector<int>& received,
                         const QVector<int>& expected) const;

    /**
     * @brief 计算分支度量(软判决)
     * @param received 接收软值
     * @param expected 期望输出(0/1)
     * @return 软距离
     */
    int branchMetricSoft(const QVector<int>& received,
                         const QVector<int>& expected) const;

    /**
     * @brief 构建完整网格图(硬判决)
     * @param encodedBits 接收比特流
     * @param numSteps 步数
     */
    void buildTrellisHard(const QVector<int>& encodedBits, int numSteps);

    /**
     * @brief 构建完整网格图(软判决)
     * @param softValues 软判决值流
     * @param numSteps 步数
     * @param precision 量化位数
     */
    void buildTrellisSoft(const QVector<int>& softValues,
                          int numSteps, int precision);

    /**
     * @brief 回溯最优路径
     * @param numSteps 步数
     * @param terminate 是否包含终止阶段
     * @return 译码比特流
     */
    QVector<int> traceback(int numSteps, bool terminate);

    Config m_config;                  ///< 编码配置
    int m_numStates;                  ///< 状态数 = 2^(K-1)
    int m_mask;                       ///< 状态掩码

    /* 网格图存储 */
    QVector<QVector<int>> m_pathMetric;  ///< 路径度量[step][state]
    QVector<QVector<int>> m_survivor;    ///< 幸存路径[step][state] = prev_state
    QVector<QVector<int>> m_inputBit;    ///< 输入比特[step][state]

    Stats m_stats;                    ///< 操作统计
    double m_timeSum = 0.0;           ///< 累计耗时
};

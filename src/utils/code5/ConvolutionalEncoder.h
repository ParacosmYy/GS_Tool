/**
 * @file ConvolutionalEncoder.h
 * @brief 卷积码编码器 — 多项式生成器实现
 *
 * 功能: 支持可配置约束长度和生成多项式的卷积码编码器,
 *       提供Viterbi解码所需的网格信息, 支持打孔(puncturing)模式。
 *
 * 协作: CrossCorrelator(信道估计) / DataQualityScorer(质量评估)
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QByteArray>

/**
 * @brief 卷积码编码器
 *
 * 使用移位寄存器和异或生成多项式实现卷积编码,
 * 支持 (n, k, K) 参数配置, 其中 n=输出比特数,
 * k=输入比特数(当前仅支持k=1), K=约束长度。
 */
class ConvolutionalEncoder : public QObject
{
    Q_OBJECT

public:
    /** @brief 编码器配置参数 */
    struct Config {
        int constraintLength = 7;           ///< 约束长度 K
        QVector<quint32> generators;        ///< 生成多项式(八进制表示)
        bool enableTermination = true;      ///< 是否添加尾比特终止
        bool enablePuncturing = false;      ///< 是否启用打孔
        QByteArray puncturePattern;         ///< 打孔模式(1=保留, 0=删除)
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalBlocksEncoded = 0;         ///< 累计编码块数
        int totalInputBits = 0;             ///< 累计输入比特数
        int totalOutputBits = 0;            ///< 累计输出比特数
        int totalPuncturedBits = 0;         ///< 累计打孔删除比特数
        double codeRate = 0.0;              ///< 有效码率
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
    };

    explicit ConvolutionalEncoder(QObject* parent = nullptr);

    /**
     * @brief 配置编码器参数
     * @param config 编码器配置
     */
    void configure(const Config& config);

    /**
     * @brief 对字节数组进行卷积编码
     * @param data 输入数据
     * @return 编码后的比特流(每字节8比特, 高位在前)
     */
    QByteArray encode(const QByteArray& data);

    /**
     * @brief 对比特数组进行卷积编码
     * @param bits 输入比特数组(0或1)
     * @return 编码后的比特数组
     */
    QVector<quint8> encodeBits(const QVector<quint8>& bits);

    /**
     * @brief 添加尾比特终止(清零移位寄存器)
     * @param bits 输入比特数组
     * @return 添加尾比特后的比特数组
     */
    QVector<quint8> addTermination(const QVector<quint8>& bits);

    /**
     * @brief 应用打孔模式
     * @param encodedBits 已编码比特
     * @return 打孔后的比特数组
     */
    QVector<quint8> applyPuncturing(const QVector<quint8>& encodedBits);

    /**
     * @brief 获取当前编码器状态
     * @return 移位寄存器状态值
     */
    quint32 encoderState() const;

    /**
     * @brief 重置编码器状态
     */
    void reset();

    /**
     * @brief 获取网格图中某状态的分支输出
     * @param state 当前状态
     * @param input 输入比特
     * @return 输出比特向量
     */
    QVector<quint8> branchOutput(quint32 state, quint8 input) const;

    /**
     * @brief 获取网格图中下一状态
     * @param state 当前状态
     * @param input 输入比特
     * @return 下一状态
     */
    quint32 nextState(quint32 state, quint8 input) const;

    Config config() const;
    Stats stats() const;
    void resetStatistics();

signals:
    /** @brief 编码完成信号 @param inputBits 输入比特数 @param outputBits 输出比特数 */
    void encoded(int inputBits, int outputBits);

private:
    /**
     * @brief 计算单个生成多项式的输出
     * @param state 移位寄存器状态
     * @param generator 生成多项式
     * @return 输出比特(0或1)
     */
    quint8 computeOutput(quint32 state, quint32 generator) const;

    Config m_config;
    quint32 m_state = 0;          ///< 移位寄存器当前状态
    int m_mask = 0;               ///< 状态掩码(用于截断)
    Stats m_stats;
    double m_timeSum = 0.0;
};

/**
 * @file ConvolutionalCoder.h
 * @brief 卷积编码器/维特比译码器
 *
 * 功能: 实现卷积编码和硬判决维特比译码，支持自定义约束长度
 *       和生成多项式。核心算法为基于网格(Trellis)的维特比
 *       最大似然序列估计。
 *
 * 协作: CrcStreamVerifier(流校验) / SerialFrameDecoder(帧解码)
 * @author Serial Tool Team
 * @date 2026-06-05
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <QtGlobal>

/**
 * @class ConvolutionalCoder
 * @brief 卷积编码与维特比硬判决译码
 *
 * 编码器: 将输入比特流按约束长度和生成多项式产生编码输出。
 * 译码器: 使用维特比算法在网格图上执行最大似然路径搜索，
 *         支持硬判决(Hamming距离度量)。
 */
class ConvolutionalCoder : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息结构体 */
    struct Stats {
        quint64 totalEncoded = 0;           ///< 累计编码比特数(输入)
        quint64 totalDecoded = 0;           ///< 累计译码比特数(输出)
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理时间(ms)
    };

    /**
     * @brief 构造函数
     * @param constraintLength 约束长度(默认7)
     * @param generators 生成多项式列表(八进制表示，默认[171,133])
     * @param parent 父对象
     */
    explicit ConvolutionalCoder(int constraintLength = 7,
                                const QVector<int> &generators = {171, 133},
                                QObject *parent = nullptr);

    /**
     * @brief 卷积编码
     * @param input 输入比特流(0/1)
     * @return 编码输出比特流(长度 = input.size() * numGenerators)
     */
    QVector<int> encode(const QVector<int> &input);

    /**
     * @brief 维特比硬判决译码
     * @param received 接收的编码比特流(0/1)
     * @return 译码输出比特流
     */
    QVector<int> decode(const QVector<int> &received);

    /**
     * @brief 设置生成多项式
     * @param gens 生成多项式列表(八进制表示)
     */
    void setGenerators(const QVector<int> &gens);

    /** @brief 获取约束长度 @return 约束长度 */
    int constraintLength() const { return m_constraintLength; }

    /** @brief 获取生成多项式 @return 生成多项式列表 */
    QVector<int> generators() const { return m_generators; }

    /** @brief 获取统计信息 @return 常量引用 */
    const Stats &stats() const { return m_stats; }

    /** @brief 重置统计计数器 */
    void resetStatistics();

signals:
    /** @brief 编码完成信号 @param inputBits 输入比特数 @param outputBits 输出比特数 */
    void encodeCompleted(int inputBits, int outputBits);
    /** @brief 译码完成信号 @param outputBits 译码输出比特数 */
    void decodeCompleted(int outputBits);

private:
    /**
     * @brief 计算给定状态和输入的编码输出
     * @param state 移位寄存器状态
     * @param input 输入比特
     * @return 各生成多项式的输出比特列表
     */
    QVector<int> computeOutput(int state, int input) const;

    /**
     * @brief 计算生成多项式的输出位
     * @param poly 生成多项式(八进制)
     * @param state 寄存器状态
     * @return 0或1
     */
    int applyPolynomial(int poly, int state) const;

    int m_constraintLength;         ///< 约束长度K
    QVector<int> m_generators;      ///< 生成多项式(八进制)
    int m_numStates;                ///< 状态数 = 2^(K-1)
    Stats m_stats;                  ///< 统计信息
    double m_timeSum = 0.0;         ///< 处理时间累加器
};

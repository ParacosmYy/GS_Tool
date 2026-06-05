/**
 * @file ReedMullerCode.h
 * @brief Reed-Muller纠错码 — 一阶RM编码/解码
 *
 * 功能: 实现一阶Reed-Muller码 RM(1,m)，编码使用生成矩阵G，
 *       解码使用快速Hadamard变换(FHT)实现最大似然解码。
 *       统计编码/解码次数、纠正比特数、平均处理耗时。
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @class ReedMullerCode
 * @brief 一阶 Reed-Muller 纠错码编码器/解码器
 */
class ReedMullerCode : public QObject {
    Q_OBJECT
public:
    /** 统计信息 */
    struct Stats {
        quint64 totalEncodings = 0;        ///< 总编码次数
        quint64 totalDecodings = 0;        ///< 总解码次数
        quint64 totalBitsCorrected = 0;    ///< 累计纠正比特数
        quint64 totalBitsProcessed = 0;    ///< 累计处理比特数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /**
     * @brief 构造函数
     * @param order RM码阶数(当前仅支持一阶 order=1)
     * @param m 参数m，码长 n = 2^m，信息位 k = m+1
     * @param parent 父对象
     */
    explicit ReedMullerCode(int order = 1, int m = 4, QObject* parent = nullptr);

    /**
     * @brief 编码
     * @param message 信息比特(m+1位)
     * @return 编码后码字(2^m位)
     */
    QVector<int> encode(const QVector<int>& message) const;

    /**
     * @brief 硬判决解码(基于FHT)
     * @param received 接收的硬判决比特(2^m位)
     * @return 解码后的信息比特(m+1位)
     */
    QVector<int> decodeHard(const QVector<int>& received);

    /**
     * @brief 软判决解码(基于FHT)
     * @param softReceived 软判决值(正≈1, 负≈0)
     * @return 解码后的信息比特(m+1位)
     */
    QVector<int> decodeSoft(const QVector<double>& softReceived);

    /** @brief 获取码长 */
    int blockLength() const { return m_n; }

    /** @brief 获取信息位长度 */
    int messageLength() const { return m_k; }

    /** @brief 获取最小距离 */
    int minDistance() const { return m_minDist; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 编码完成 @param msgBits 信息位数 @param codeBits 码字位数 */
    void encodingComplete(int msgBits, int codeBits);
    /** @brief 解码完成 @param codeBits 码字位数 @param corrections 纠正比特数 */
    void decodingComplete(int codeBits, int corrections);

private:
    /** 快速Hadamard变换(原地, Walsh序) */
    void hadamardTransform(QVector<double>& data) const;
    /** 从FHT峰值索引恢复信息位 */
    QVector<int> indexToMessage(int peakIndex) const;
    /** 构建生成矩阵 */
    void buildGeneratorMatrix();

    int m_order;            ///< RM码阶数
    int m_m;                ///< 参数m
    int m_n;                ///< 码长 = 2^m
    int m_k;                ///< 信息位 = m+1
    int m_minDist;          ///< 最小距离 = 2^(m-1)

    /** 生成矩阵 [k x n] */
    QVector<QVector<int>> m_generator;

    Stats  m_stats;         ///< 统计信息
    double m_timeSum = 0.0; ///< 累计耗时
};

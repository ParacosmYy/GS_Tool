/**
 * @file LdpcEncoder.h
 * @brief LDPC码编码器/解码器(Low-Density Parity-Check Code)
 *
 * 功能: 实现LDPC纠错码的编码与比特翻转(Bit-Flipping)解码算法。
 *       支持自定义奇偶校验矩阵，统计编码/解码次数/平均耗时。
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QByteArray>

/**
 * @class LdpcEncoder
 * @brief LDPC码编码器/解码器
 *
 * 使用稀疏奇偶校验矩阵H进行编码(系统码形式)，
 * 解码采用硬判决比特翻转算法。适用于二进制对称信道。
 */
class LdpcEncoder : public QObject
{
    Q_OBJECT

public:
    /** @brief 运行统计信息 */
    struct Stats {
        quint64 totalEncoded = 0;    ///< 总编码次数
        quint64 totalDecoded = 0;    ///< 总解码次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /**
     * @brief 构造函数
     * @param parent QObject父对象
     */
    explicit LdpcEncoder(QObject* parent = nullptr);

    /**
     * @brief 设置奇偶校验矩阵H
     * @param H (n-k)×n 的稀疏二进制奇偶校验矩阵
     */
    void setParityMatrix(const QVector<QVector<int>>& H);

    /**
     * @brief 编码数据
     * @param data 输入数据(长度必须等于k = n - rows(H))
     * @return 编码后的码字(长度n)
     */
    QByteArray encode(const QByteArray& data);

    /**
     * @brief 解码接收数据(比特翻转算法)
     * @param received 接收的码字(长度n)
     * @param maxIter 最大迭代次数
     * @return 解码后的数据(长度k)
     */
    QByteArray decode(const QByteArray& received, int maxIter = 50);

    /** @brief 获取码长n */
    int codeLength() const { return m_n; }

    /** @brief 获取信息位长度k */
    int messageLength() const { return m_k; }

    /** @brief 获取统计信息 */
    Stats stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 解码完成信号 @param iterations 实际迭代次数 @param converged 是否收敛 */
    void decodingCompleted(int iterations, bool converged);

private:
    /**
     * @brief 高斯消元求生成矩阵G (系统码形式)
     * @return 是否成功(矩阵必须满秩)
     */
    bool computeGeneratorMatrix();

    /**
     * @brief 计算校验子和(硬判决)
     * @param codeword 码字
     * @return 不满足的校验方程数
     */
    int computeSyndromeWeight(const QVector<int>& codeword) const;

    /**
     * @brief 计算每个比特位的校验失败次数
     * @param codeword 码字
     * @return 每个位置的失败次数
     */
    QVector<int> computeBitFailures(const QVector<int>& codeword) const;

    mutable Stats m_stats;                 ///< 统计信息
    double m_timeSum = 0.0;                ///< 累计耗时
    int m_totalOps = 0;                    ///< 总操作次数

    QVector<QVector<int>> m_H;             ///< 奇偶校验矩阵
    QVector<QVector<int>> m_G;             ///< 生成矩阵(系统码)
    int m_n = 0;                           ///< 码长
    int m_k = 0;                           ///< 信息位长度
    int m_m = 0;                           ///< 校验位长度(n-k)
    bool m_matrixReady = false;            ///< 生成矩阵是否就绪
};

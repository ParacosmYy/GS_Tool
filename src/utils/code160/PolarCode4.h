/**
 * @file PolarCode4.h
 * @brief Polar码编码器/译码器(SC) — Polar Code Encoder/Decoder with SC Decoding
 *
 * 功能: 支持任意码长(N=2^n)和码率构造Polar码。使用Bhattacharyya参数
 *       进行信道极化排序，实现逐次消除(SC)译码算法。
 *
 * 协作: ReedSolomon(纠错码) / ConvolutionalDecoder(卷积码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Polar码编码器与SC译码器
 */
class PolarCode4 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalEncodes = 0;           ///< 累计编码次数
        quint64 totalDecodes = 0;           ///< 累计译码次数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
        double lastBER = 0.0;               ///< 最近一次比特错误率
    };

    explicit PolarCode4(QObject* parent = nullptr);

    /**
     * @brief 设置码长N(必须为2的幂)
     * @param n 码长
     */
    void setCodeLength(int n);

    /**
     * @brief 设置信息位数量K
     * @param k 信息位数量，<= N
     */
    void setInfoBits(int k);

    /**
     * @brief 设置设计SNR(dB)用于信道极化
     * @param snrDb 设计信噪比
     */
    void setDesignSNR(double snrDb);

    /**
     * @brief 构造Polar码(计算信道可靠性排序)
     */
    void buildCode();

    /**
     * @brief Polar码编码
     * @param infoBits 信息位(长度K)
     * @return 编码后码字(长度N)
     */
    QVector<int> encode(const QVector<int>& infoBits);

    /**
     * @brief SC逐次消除译码
     * @param llr 接收端对数似然比(长度N)
     * @return 译码信息位(长度K)
     */
    QVector<int> decode(const QVector<double>& llr);

    /** @brief 获取信息位索引集合 */
    QVector<int> infoIndices() const { return m_infoIndices; }

    /** @brief 获取冻结位索引集合 */
    QVector<int> frozenIndices() const { return m_frozenIndices; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 编码完成 @param n 码长 @param k 信息位数 */
    void encodeCompleted(int n, int k);
    /** @brief 译码完成 @param errors 错误比特数 */
    void decodeCompleted(int errors);

private:
    /** @brief 计算Bhattacharyya参数 */
    QVector<double> computeBhattacharyya() const;

    /** @brief Polar变换(编码核) */
    void polarTransform(QVector<int>& bits) const;

    /** @brief SC译码递归核心 */
    double scDecodeRecursive(const QVector<double>& llr,
                             QVector<int>& uEstimates,
                             int bitIndex) const;

    int m_N = 256;
    int m_K = 128;
    double m_designSNR = 0.0;

    QVector<int> m_infoIndices;     ///< 信息位索引
    QVector<int> m_frozenIndices;   ///< 冻结位索引
    QVector<int> m_frozenBits;      ///< 冻结位的值(全0)

    Stats m_stats;
    double m_timeSum = 0.0;
};

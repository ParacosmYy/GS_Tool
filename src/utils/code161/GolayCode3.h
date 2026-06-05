/**
 * @file GolayCode3.h
 * @brief 扩展Golay码[24,12,8]编解码器 — Extended Golay [24,12,8] Encoder/Decoder
 *
 * 功能: 实现扩展Golay码[24,12,8]的前向纠错编码与解码。
 *       最小汉明距离8，可纠正任意3位错误。使用伴随式查表法
 *       实现高效解码，支持编码、解码和错误统计。
 *
 * 协作: ReedSolomon2(里德-所罗门码) / ConvolutionalDecoder(卷积码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 扩展Golay码[24,12,8]编解码器
 */
class GolayCode3 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalEncodes = 0;           ///< 累计编码次数
        quint64 totalDecodes = 0;           ///< 累计解码次数
        quint64 totalCorrections = 0;       ///< 累计纠错次数
        quint64 uncorrectableErrors = 0;    ///< 不可纠正错误次数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
    };

    explicit GolayCode3(QObject* parent = nullptr);

    /**
     * @brief 编码12位数据为24位码字
     * @param data 12位数据(每元素0或1)
     * @return 24位编码码字
     */
    QVector<int> encode(const QVector<int>& data);

    /**
     * @brief 解码24位码字为12位数据
     * @param codeword 24位接收码字
     * @param correctedData 输出纠正后的12位数据
     * @return 纠错结果: 0=无错误, >0=纠正错误数, -1=不可纠正
     */
    int decode(const QVector<int>& codeword, QVector<int>& correctedData);

    /**
     * @brief 计算汉明重量(1的个数)
     * @param bits 比特向量
     * @return 汉明重量
     */
    static int hammingWeight(const QVector<int>& bits);

    /**
     * @brief 计算两个比特向量的汉明距离
     */
    static int hammingDistance(const QVector<int>& a, const QVector<int>& b);

    /**
     * @brief 引入随机错误(测试用)
     * @param codeword 码字
     * @param numErrors 错误位数
     * @return 含错误的码字
     */
    QVector<int> injectErrors(const QVector<int>& codeword, int numErrors);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 编码完成 @param dataSize 数据位数 */
    void encodeCompleted(int dataSize);
    /** @brief 解码完成 @param corrected 纠错位数, -1=不可纠正 */
    void decodeCompleted(int corrected);

private:
    /** @brief 12x12生成矩阵B(二进制) */
    static const int s_generatorB[12][12];

    /** @brief 计算伴随式 s = r * H^T */
    QVector<int> syndrome(const QVector<int>& codeword) const;

    /** @brief 向量模2加法 */
    static QVector<int> xorVectors(const QVector<int>& a, const QVector<int>& b);

    /** @brief 构建标准生成矩阵G = [I12 | B] */
    void buildGeneratorMatrix();

    /** @brief 构建奇偶校验矩阵 H = [B^T | I12] */
    void buildParityMatrix();

    QVector<QVector<int>> m_generatorG;    ///< 24x12生成矩阵
    QVector<QVector<int>> m_parityH;       ///< 24x12校验矩阵

    Stats m_stats;
    double m_timeSum = 0.0;
};

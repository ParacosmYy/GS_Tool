/**
 * @file GolayCode2.h
 * @brief 扩展 Golay (24,12,8) 码 — 软判决译码
 *
 * 功能: 实现二进制扩展 Golay 码的编码和软判决译码，
 *       码参数 [n=24, k=12, d=8]，可纠正 ≤3 位错误。
 *       软判决利用信道可靠性信息提升纠错性能约 2dB。
 *
 * 协作: CrcStreamVerifier(校验) / SerialFrameDecoder(帧解码)
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QByteArray>
#include <QPair>

/**
 * @brief 扩展 Golay (24,12,8) 码编解码器
 *
 * 扩展 Golay 码是最优纠错码之一:
 *   - 码长 n = 24, 信息位 k = 12, 最小距离 d = 8
 *   - 可纠正 t = 3 位随机错误
 *   - 生成矩阵基于 I12 + B，B 为 12x12 循环矩阵
 * 软判决译码使用 Chase 算法，比硬判决多约 2dB 编码增益。
 */
class GolayCode2 : public QObject {
    Q_OBJECT

public:
    /** @brief 译码结果 */
    struct DecodeResult {
        QVector<int> decodedBits;     ///< 译码后的信息位 (12 bits)
        QVector<int> codeword;        ///< 译码后的完整码字 (24 bits)
        int correctedErrors = 0;      ///< 纠正的错误位数
        bool success = false;         ///< 译码是否成功
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalEncodes = 0;             ///< 累计编码次数
        int totalDecodes = 0;             ///< 累计译码次数
        int totalCorrectedBits = 0;       ///< 累计纠正的比特数
        int totalUncorrectable = 0;       ///< 累计不可纠正次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit GolayCode2(QObject* parent = nullptr);

    /**
     * @brief 编码 12 位信息为 24 位码字
     * @param message 12 位信息向量 (每个元素为 0 或 1)
     * @return 24 位码字
     */
    QVector<int> encode(const QVector<int>& message);

    /**
     * @brief 硬判决译码
     * @param received 接收到的 24 位硬判决序列
     * @return 译码结果
     */
    DecodeResult decodeHard(const QVector<int>& received);

    /**
     * @brief 软判决译码 (Chase 算法)
     * @param softValues 接收到的 24 个软判决值 (正=0倾向, 负=1倾向)
     * @return 译码结果
     */
    DecodeResult decodeSoft(const QVector<double>& softValues);

    /**
     * @brief 计算伴随式
     * @param codeword 24 位码字
     * @return 12 位伴随式
     */
    QVector<int> syndrome(const QVector<int>& codeword) const;

    /**
     * @brief 计算码字重量 (汉明重量)
     * @param bits 比特序列
     * @return 重量
     */
    int hammingWeight(const QVector<int>& bits) const;

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 编码完成 @param bits 输出码字长度 */
    void encoded(int bits);

    /** @brief 译码完成 @param corrected 纠正的错误数 @param success 是否成功 */
    void decoded(int corrected, bool success);

private:
    void initGeneratorMatrix();
    QVector<int> matrixMultiply(const QVector<int>& vec,
                                const QVector<QVector<int>>& mat) const;
    QVector<int> xorVectors(const QVector<int>& a,
                            const QVector<int>& b) const;
    int findClosestCodeword(const QVector<int>& hardDecision,
                            const QVector<double>& softValues);

    QVector<QVector<int>> m_generatorB;  ///< 生成矩阵的 B 部分 (12x12)
    QVector<QVector<int>> m_parityCheck; ///< 校验矩阵 (12x24)

    Stats m_stats;                ///< 统计信息
    double m_timeSum = 0.0;       ///< 处理时间累加器
};

/**
 * @file ModifiedDCT.h
 * @brief 改进离散余弦变换(MDCT/IMDCT) — Modified Discrete Cosine Transform via Time-Domain Aliasing Cancellation with Sine Window
 *
 * 功能: 实现MDCT/IMDCT变换，基于时域混叠消除(TDAC)原理，
 *       支持正弦窗函数，50%重叠加分析与合成。广泛用于AAC/MP3/Vorbis。
 *
 * 协作: DiscreteCosineTransform(DCT) / DiscreteSineTransform(DST) / FftEngine(FFT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief MDCT/IMDCT处理器
 */
class ModifiedDCT : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalForward = 0;           ///< 累计正变换次数
        quint64 totalInverse = 0;           ///< 累计逆变换次数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
        double lastPeakBin = 0.0;           ///< 最近峰值频点
    };

    explicit ModifiedDCT(QObject* parent = nullptr);
    ~ModifiedDCT() override;

    /** @brief 设置变换块大小(必须为偶数) */
    void setBlockSize(int N);

    /**
     * @brief MDCT正变换(分析)
     * @param input 长度2N的输入(前后各N/2重叠)
     * @return N个MDCT系数
     */
    QVector<double> forward(const QVector<double>& input);

    /**
     * @brief IMDCT逆变换(合成)
     * @param coeffs N个MDCT系数
     * @return 长度2N的时域输出
     */
    QVector<double> inverse(const QVector<double>& coeffs);

    /**
     * @brief 50%重叠相加完整重建
     * @param blocks 连续MDCT系数块
     * @return 完整时域重建信号
     */
    QVector<double> overlapAdd(const QVector<QVector<double>>& blocks);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 正变换完成 @param blockSize 块大小 */
    void forwardCompleted(int blockSize);
    /** @brief 逆变换完成 @param blockSize 块大小 */
    void inverseCompleted(int blockSize);

private:
    /** @brief 生成正弦窗 */
    QVector<double> sineWindow(int len) const;

    /** @brief MDCT核心计算 */
    void computeMDCT(const QVector<double>& windowed, QVector<double>& out) const;

    /** @brief IMDCT核心计算 */
    void computeIMDCT(const QVector<double>& coeffs, QVector<double>& out) const;

    int m_blockSize = 1024;

    Stats m_stats;
    double m_timeSum = 0.0;
};

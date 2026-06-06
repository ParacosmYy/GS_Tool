/**
 * @file DiscreteCosineTransform.h
 * @brief 离散余弦变换(Lee快速DCT-II/III) — Discrete Cosine Transform via Lee's Fast Algorithm
 *
 * 功能: 实现DCT-II(正变换)和DCT-III(逆变换)，使用Lee快速算法。
 *       输入长度自动补零至2的幂。支持能量压缩分析和频带截断。
 *
 * 协作: HaarWaveletTransform(Haar小波) / FftEngine(FFT) / ConstantQTransform(常Q变换)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 离散余弦变换处理器
 */
class DiscreteCosineTransform : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalForward = 0;           ///< 累计正变换次数
        quint64 totalInverse = 0;           ///< 累计逆变换次数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
        double energyCompaction = 0.0;      ///< 能量集中度(前N/4系数能量占比)
    };

    explicit DiscreteCosineTransform(QObject* parent = nullptr);
    ~DiscreteCosineTransform() override;

    /**
     * @brief DCT-II正变换
     * @param signal 输入信号
     * @return DCT系数
     */
    QVector<double> forward(const QVector<double>& signal);

    /**
     * @brief DCT-III逆变换
     * @param coeffs DCT系数
     * @param originalLength 原始长度(截断补零)
     * @return 重构信号
     */
    QVector<double> inverse(const QVector<double>& coeffs, int originalLength);

    /**
     * @brief 频带截断：仅保留前keepCoeffs个系数
     * @param signal 输入信号
     * @param keepCoeffs 保留系数数
     * @return 截断后重构的信号
     */
    QVector<double> truncate(const QVector<double>& signal, int keepCoeffs);

    /**
     * @brief 计算能量集中度(前quarter系数占比)
     * @param coeffs DCT系数
     * @return 占比[0,1]
     */
    double energyCompaction(const QVector<double>& coeffs) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 正变换完成 @param length 信号长度 */
    void forwardCompleted(int length);
    /** @brief 逆变换完成 @param length 信号长度 */
    void inverseCompleted(int length);

private:
    /** @brief Lee快速DCT-II递归 */
    void fastDCT2(QVector<double>& data, int start, int len) const;

    /** @brief Lee快速DCT-III递归 */
    void fastDCT3(QVector<double>& data, int start, int len) const;

    /** @brief 补零到2的幂 */
    static int nextPowerOf2(int n);

    Stats m_stats;
    double m_timeSum = 0.0;
};

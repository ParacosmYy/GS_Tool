/**
 * @file HaarWaveletTransform.h
 * @brief Haar小波变换 — Haar Wavelet Forward/Inverse Transform with Multi-Level Decomposition
 *
 * 功能: 实现Haar小波正变换和逆变换，支持多级分解与重构。
 *       输入长度自动补零至2的幂。提供各层近似/细节系数提取、
 *       阈值去噪和能量分布统计。
 *
 * 协作: ConstantQTransform(常Q变换) / ShortTimeFourier(STFT) / FftEngine(FFT核心)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Haar小波变换处理器
 */
class HaarWaveletTransform : public QObject {
    Q_OBJECT

public:
    /** @brief 小波系数 */
    struct WaveletCoeffs {
        QVector<double> approximation;  ///< 近似系数(cA)
        QVector<double> detail;         ///< 细节系数(cD)
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalTransforms = 0;        ///< 累计变换次数
        quint64 totalLevels = 0;            ///< 累计分解层数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
        double energyRatio = 0.0;           ///< 最终近似能量占比
    };

    explicit HaarWaveletTransform(QObject* parent = nullptr);

    /**
     * @brief 设置最大分解层数(0=自动取log2(N))
     * @param levels 层数，0表示自动
     */
    void setMaxLevels(int levels);

    /**
     * @brief 正变换：多级Haar小波分解
     * @param signal 输入信号
     * @return 各层{近似,细节}系数，从低频到高频排列
     */
    QVector<WaveletCoeffs> forward(const QVector<double>& signal);

    /**
     * @brief 逆变换：从小波系数重构信号
     * @param coeffs 各层小波系数
     * @param originalLength 原始信号长度(用于截断补零)
     * @return 重构信号
     */
    QVector<double> inverse(const QVector<WaveletCoeffs>& coeffs, int originalLength);

    /**
     * @brief 软阈值去噪
     * @param signal 输入信号
     * @param threshold 阈值
     * @return 去噪后信号
     */
    QVector<double> denoise(const QVector<double>& signal, double threshold);

    /**
     * @brief 计算各层能量分布
     * @param coeffs 小波系数
     * @return 各层能量值
     */
    QVector<double> energyDistribution(const QVector<WaveletCoeffs>& coeffs) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 变换完成 @param levels 分解层数 @param signalLength 信号长度 */
    void transformCompleted(int levels, int signalLength);

private:
    /** @brief 单级Haar正变换 */
    WaveletCoeffs singleLevelForward(const QVector<double>& input) const;

    /** @brief 单级Haar逆变换 */
    QVector<double> singleLevelInverse(const QVector<double>& approx,
                                         const QVector<double>& detail) const;

    /** @brief 补零到2的幂 */
    static int nextPowerOf2(int n);

    int m_maxLevels = 0;    ///< 最大分解层数(0=自动)

    Stats m_stats;
    double m_timeSum = 0.0;
};

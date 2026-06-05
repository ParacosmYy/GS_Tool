/**
 * @file WaveletDenoiser.h
 * @brief 小波去噪器
 */

#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 小波去噪器
 *
 * 支持硬阈值、软阈值和半软阈值去噪,
 * 可配置小波基和分解层数。
 */
class WaveletDenoiser : public QObject
{
    Q_OBJECT

public:
    /** @brief 阈值类型 */
    enum ThresholdType {
        Hard = 0,       ///< 硬阈值
        Soft = 1,       ///< 软阈值
        SemiSoft = 2    ///< 半软阈值
    };
    Q_ENUM(ThresholdType)

    /** @brief 统计信息 */
    struct Stats {
        int totalDenoised = 0;          ///< 总去噪次数
        double totalInputSNR = 0.0;     ///< 总输入SNR
        double totalOutputSNR = 0.0;    ///< 总输出SNR
        double avgProcessingTimeMs = 0.0;
    };

    explicit WaveletDenoiser(QObject* parent = nullptr);

    /**
     * @brief 对信号进行小波去噪
     * @param signal 含噪信号
     * @param level 分解层数(0=自动)
     * @param type 阈值类型
     * @return 去噪后的信号
     */
    QVector<double> denoise(const QVector<double>& signal,
                            int level = 0,
                            ThresholdType type = Soft);

    /**
     * @brief 估计噪声标准差(使用MAD)
     * @param detailCoeffs 细节系数
     * @return 噪声标准差估计
     */
    double estimateNoiseStd(const QVector<double>& detailCoeffs) const;

    /**
     * @brief 计算通用阈值(VisuShrink)
     * @param sigma 噪声标准差
     * @param n 信号长度
     * @return 阈值
     */
    double universalThreshold(double sigma, int n) const;

    /**
     * @brief 计算SURE阈值(SureShrink)
     * @param detailCoeffs 细节系数
     * @return 最优阈值
     */
    double sureThreshold(const QVector<double>& detailCoeffs) const;

    /**
     * @brief 计算信噪比(SNR)
     * @param signal 原始信号
     * @param noisy 含噪信号
     * @return SNR(dB)
     */
    double computeSNR(const QVector<double>& signal,
                       const QVector<double>& noisy) const;

    Stats stats() const;
    void resetStatistics();

signals:
    /** @brief 去噪完成信号 */
    void denoised(int length, double estimatedSNR);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    void haarForward(QVector<double>& data, int n);
    void haarInverse(QVector<double>& data, int n);
    double applyThreshold(double coeff, double threshold,
                          ThresholdType type) const;
};

/**
 * @file NoiseProfiler.h
 * @brief 噪声轮廓分析器 — Noise Floor Estimation with Percentile-Based Frequency Band Profiling
 *
 * 功能: 基于百分位数的噪声底估计器。将频谱划分为多个子带，
 *       在每个子带内统计幅度分布，以可配置百分位数作为噪声底。
 *       支持时间平滑、频谱泄漏补偿和噪声底追踪。
 *
 * 协作: FftEngine(FFT核心) / SpectralSubtraction(谱减去噪) / VoiceActivityDetector(语音活动检测)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 噪声轮廓分析器
 */
class NoiseProfiler : public QObject {
    Q_OBJECT

public:
    /** @brief 频段噪声信息 */
    struct BandProfile {
        double lowFreq = 0.0;           ///< 频段下界(Hz)
        double highFreq = 0.0;          ///< 频段上界(Hz)
        double noiseFloor = 0.0;        ///< 噪声底(dB)
        double meanLevel = 0.0;         ///< 平均电平(dB)
        double variance = 0.0;          ///< 方差
        int binCount = 0;               ///< 包含频点数
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalProfiles = 0;          ///< 累计分析次数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
        double globalNoiseFloor = 0.0;      ///< 全局噪声底(dB)
        int bandCount = 0;                  ///< 频段数
    };

    explicit NoiseProfiler(QObject* parent = nullptr);

    /**
     * @brief 设置采样率
     * @param rate 采样率(Hz)
     */
    void setSampleRate(double rate);

    /**
     * @brief 设置FFT大小
     * @param size FFT点数(2的幂)
     */
    void setFFTSize(int size);

    /**
     * @brief 设置噪声底百分位数
     * @param percentile 百分位(0.0-1.0，如0.5=中位数)
     */
    void setPercentile(double percentile);

    /**
     * @brief 设置频段数(OCTAVE分带)
     * @param bands 频段数
     */
    void setBandCount(int bands);

    /**
     * @brief 设置时间平滑系数
     * @param alpha 平滑系数(0.0-1.0，0=无平滑，1=完全保留)
     */
    void setSmoothing(double alpha);

    /**
     * @brief 分析频谱，更新噪声轮廓
     * @param magnitude 频谱幅度(线性值，FFT输出)
     * @return 各频段噪声轮廓
     */
    QVector<BandProfile> profile(const QVector<double>& magnitude);

    /**
     * @brief 获取当前噪声底(各频段)
     */
    QVector<BandProfile> currentProfile() const { return m_bands; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 分析完成 @param bands 频段数 @param noiseFloor 全局噪声底 */
    void profileCompleted(int bands, double noiseFloor);

private:
    /** @brief 初始化频段划分 */
    void initBands();

    /** @brief 线性值转dB */
    static double toDB(double linear);

    double m_sampleRate = 44100.0;
    int m_fftSize = 2048;
    double m_percentile = 0.5;
    int m_bandCount = 8;
    double m_smoothing = 0.9;

    QVector<BandProfile> m_bands;          ///< 各频段轮廓
    QVector<QVector<double>> m_history;    ///< 各频段历史幅度(dB)
    bool m_initialized = false;

    Stats m_stats;
    double m_timeSum = 0.0;
};

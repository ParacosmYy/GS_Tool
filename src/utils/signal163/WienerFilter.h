/**
 * @file WienerFilter.h
 * @brief 频域维纳滤波器 — Frequency-Domain Wiener Filter with Noise PSD Estimation
 *
 * 功能: 实现频域维纳滤波器用于信号降噪。支持噪声PSD估计(最小值跟踪法)、
 *       先验SNR估计(决策引导法)、频谱减法增益控制。
 *
 * 协作: MultibandGate(噪声门) / ShortTimeFourier(STFT) / FftEngine(FFT核心)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 频域维纳滤波器
 */
class WienerFilter : public QObject {
    Q_OBJECT

public:
    /** @brief 噪声估计方法 */
    enum class NoiseEstimation {
        MinimumStatistics,      ///< 最小值统计跟踪
        SpectralSubtraction,    ///< 频谱减法
        DecisionDirected        ///< 决策引导
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalFrames = 0;            ///< 累计处理帧数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
        double estimatedNoiseFloor = 0.0;   ///< 估计的噪声底(dB)
        double avgSNR = 0.0;                ///< 平均信噪比(dB)
    };

    explicit WienerFilter(QObject* parent = nullptr);

    /**
     * @brief 设置FFT点数
     * @param nfft FFT点数(2的幂)
     */
    void setFftSize(int nfft);

    /**
     * @brief 设置采样率
     * @param rate 采样率(Hz)
     */
    void setSampleRate(double rate);

    /**
     * @brief 设置噪声估计方法
     * @param method 噪声估计方法
     */
    void setNoiseEstimation(NoiseEstimation method);

    /**
     * @brief 设置先验SNR平滑系数
     * @param alpha 平滑系数(0.0-1.0)
     */
    void setSmoothingFactor(double alpha);

    /**
     * @brief 设置噪声过减因子
     * @param beta 过减因子(>= 1.0)
     */
    void setOverSubtraction(double beta);

    /**
     * @brief 提供噪声参考信号进行噪声PSD估计
     * @param noise 纯噪声采样数据
     */
    void estimateNoiseProfile(const QVector<double>& noise);

    /**
     * @brief 处理一帧含噪信号
     * @param noisySignal 含噪时域信号
     * @return 降噪后的时域信号
     */
    QVector<double> process(const QVector<double>& noisySignal);

    /**
     * @brief 重置滤波器状态
     */
    void reset();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 帧处理完成 @param snr 当前帧SNR(dB) */
    void frameProcessed(double snr);

private:
    /** @brief 基2 FFT(就地) */
    void fft(QVector<double>& real, QVector<double>& imag) const;
    /** @brief 基2 IFFT */
    void ifft(QVector<double>& real, QVector<double>& imag) const;

    /** @brief 计算维纳增益 */
    QVector<double> computeWienerGain(const QVector<double>& signalPSD,
                                      const QVector<double>& noisePSD);

    /** @brief 最小值统计噪声跟踪 */
    void updateNoiseEstimate(const QVector<double>& magnitude);

    int m_nfft = 512;
    double m_sampleRate = 44100.0;
    NoiseEstimation m_noiseMethod = NoiseEstimation::MinimumStatistics;
    double m_alpha = 0.98;
    double m_beta = 1.5;

    QVector<double> m_noisePSD;             ///< 噪声功率谱密度
    QVector<double> m_priorSNR;             ///< 先验SNR
    QVector<double> m_noiseMinBuf;          ///< 最小值跟踪缓冲
    int m_minTrackLen = 0;                  ///< 最小值跟踪计数
    bool m_noiseEstimated = false;          ///< 噪声PSD是否已估计

    Stats m_stats;
    double m_timeSum = 0.0;
};

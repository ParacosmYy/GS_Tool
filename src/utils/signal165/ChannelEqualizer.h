/**
 * @file ChannelEqualizer.h
 * @brief 频域信道均衡器(逆滤波) — Frequency-Domain Channel Equalization with Inverse Filter
 *
 * 功能: 基于频域逆滤波的信道均衡器。估计信道频率响应，
 *       构造逆滤波器补偿信道失真。支持ZF(迫零)和MMSE均衡，
 *       提供频域和时域处理。
 *
 * 协作: FftEngine(FFT) / WienerFilter(维纳滤波) / AdaptiveFilter6(自适应滤波)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 频域信道均衡器
 */
class ChannelEqualizer : public QObject {
    Q_OBJECT

public:
    /** @brief 均衡算法 */
    enum class EqualizerType {
        ZeroForcing,    ///< 迫零均衡(无噪声优化)
        MMSE            ///< 最小均方误差均衡
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalEqualized = 0;         ///< 累计均衡帧数
        quint64 totalSamples = 0;           ///< 累计采样数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
        double lastSNR = 0.0;               ///< 最近一次均衡后SNR(dB)
        double lastMSE = 0.0;               ///< 最近一次MSE
    };

    explicit ChannelEqualizer(QObject* parent = nullptr);
    ~ChannelEqualizer() override;

    /**
     * @brief 设置均衡类型
     * @param type 均衡算法
     */
    void setEqualizerType(EqualizerType type);

    /**
     * @brief 设置噪声方差(MMSE模式)
     * @param variance 噪声方差
     */
    void setNoiseVariance(double variance);

    /**
     * @brief 设置FFT大小
     * @param size FFT大小(2的幂)
     */
    void setFFTSize(int size);

    /**
     * @brief 从训练序列估计信道响应
     * @param transmitted 发送信号
     * @param received 接收信号
     * @return 估计的信道脉冲响应
     */
    QVector<double> estimateChannel(const QVector<double>& transmitted,
                                     const QVector<double>& received);

    /**
     * @brief 设置已知信道脉冲响应
     * @param impulseResponse 信道脉冲响应
     */
    void setChannelResponse(const QVector<double>& impulseResponse);

    /**
     * @brief 均衡接收信号
     * @param received 接收信号
     * @return 均衡后的信号
     */
    QVector<double> equalize(const QVector<double>& received);

    /**
     * @brief 获取信道频率响应
     */
    QVector<QPair<double, double>> channelFrequencyResponse() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 均衡完成 @param samples 采样数 @param snr 均衡后SNR */
    void equalizeCompleted(int samples, double snr);

private:
    /** @brief 简单FFT */
    static void fft(QVector<double>& real, QVector<double>& imag, bool inverse);

    /** @brief 构造均衡滤波器(频域) */
    void buildEqualizerFilter();

    EqualizerType m_type = EqualizerType::MMSE;
    double m_noiseVariance = 0.01;
    int m_fftSize = 256;

    QVector<double> m_channelIR;                    ///< 信道脉冲响应
    QVector<QPair<double, double>> m_eqFilter;      ///< 均衡滤波器(real, imag)
    bool m_filterReady = false;

    Stats m_stats;
    double m_timeSum = 0.0;
};

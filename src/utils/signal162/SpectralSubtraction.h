/**
 * @file SpectralSubtraction.h
 * @brief 谱减法降噪器 — Spectral Subtraction Noise Reduction
 *
 * 功能: 基于频域谱减法的单通道降噪，支持噪声谱估计、过减因子
 *       调节、谱平滑和频谱地板。适用于稳态噪声环境下的语音增强。
 *
 * 协作: ShortTimeFourier(STFT引擎) / WienerFilter(维纳滤波降噪)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 谱减法降噪器，支持噪声估计/过减/谱平滑
 */
class SpectralSubtraction : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalFrames = 0;                ///< 累计处理帧数
        double avgNoisePowerDb = 0.0;           ///< 平均噪声功率(dB)
        double avgSnrImprovementDb = 0.0;       ///< 平均SNR改善量(dB)
        double avgProcessingTimeMs = 0.0;       ///< 平均处理耗时(ms)
    };

    explicit SpectralSubtraction(QObject* parent = nullptr);

    /**
     * @brief 设置FFT帧长
     * @param size 帧长(采样点数)，推荐512/1024/2048
     */
    void setFrameSize(int size);

    /**
     * @brief 设置帧移
     * @param hop 帧移采样点数
     */
    void setHopSize(int hop);

    /**
     * @brief 设置过减因子alpha
     * @param alpha 过减因子(1.0~3.0)，越大降噪越激进
     */
    void setOversubtraction(double alpha);

    /**
     * @brief 设置谱地板因子beta
     * @param beta 谱地板(0.0~1.0)，防止频谱出现负值
     */
    void setSpectralFloor(double beta);

    /**
     * @brief 设置噪声估计帧数
     * @param frames 用于估计噪声谱的起始帧数
     */
    void setNoiseEstimationFrames(int frames);

    /**
     * @brief 对音频信号执行谱减法降噪
     * @param input 含噪输入信号
     * @return 降噪后的信号
     */
    QVector<double> process(const QVector<double>& input);

    /**
     * @brief 手动设置噪声谱(跳过噪声估计阶段)
     * @param noiseMagnitude 噪声幅度谱
     */
    void setNoiseSpectrum(const QVector<double>& noiseMagnitude);

    /** @brief 获取当前噪声谱估计 */
    QVector<double> noiseSpectrum() const { return m_noiseSpectrum; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 帧处理完成 @param frameIdx 帧索引 @param snrDb 当前SNR(dB) */
    void frameProcessed(int frameIdx, double snrDb);

private:
    /** @brief FFT(就地) */
    void fft(QVector<double>& real, QVector<double>& imag) const;
    /** @brief IFFT(就地) */
    void ifft(QVector<double>& real, QVector<double>& imag) const;

    /** @brief 汉宁窗 */
    QVector<double> hanningWindow(int size) const;

    /** @brief 估计噪声幅度谱 */
    void estimateNoise(const QVector<double>& signal);

    int m_frameSize = 1024;
    int m_hopSize = 512;
    double m_alpha = 2.0;       ///< 过减因子
    double m_beta = 0.01;       ///< 谱地板
    int m_noiseFrames = 10;     ///< 噪声估计帧数
    double m_sampleRate = 44100.0;

    QVector<double> m_noiseSpectrum;    ///< 噪声幅度谱估计
    bool m_noiseEstimated = false;

    Stats m_stats;
    double m_timeSum = 0.0;
    double m_noisePowerSum = 0.0;
    double m_snrImprovementSum = 0.0;
};

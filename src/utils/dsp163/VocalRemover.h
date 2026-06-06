/**
 * @file VocalRemover.h
 * @brief 中心声道人声消除(相位对消) — Center-Channel Vocal Removal with Phase Cancellation
 *
 * 功能: 基于立体声左右声道差异的中心人声消除算法。
 *       利用相位对消原理，假设人声位于中心声道(左右等量)，
 *       通过L-R差值提取伴奏信号。支持频域掩蔽增强。
 *
 * 协作: ShortTimeFourier(STFT) / ConstantQTransform(常Q变换) / SpectralFilter5(频谱滤波)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 中心声道人声消除器
 */
class VocalRemover : public QObject {
    Q_OBJECT

public:
    /** @brief 消除模式 */
    enum class RemovalMode {
        SimpleSubtract,     ///< 简单L-R差值
        FrequencyDomain     ///< 频域掩蔽增强
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalProcessed = 0;        ///< 累计处理帧数
        quint64 totalSamples = 0;          ///< 累计采样点数
        double avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
        double vocalReductionDb = 0.0;     ///< 人声消除量(dB)
    };

    explicit VocalRemover(QObject* parent = nullptr);
    ~VocalRemover() override;

    /**
     * @brief 设置消除模式
     * @param mode 消除模式
     */
    void setMode(RemovalMode mode);

    /**
     * @brief 设置低通截止频率(Hz)
     * @param freq 截止频率，低于此频率保留低频
     */
    void setLowPassFreq(double freq);

    /**
     * @brief 设置FFT大小(频域模式)
     * @param size FFT大小(2的幂)
     */
    void setFFTSize(int size);

    /**
     * @brief 处理立体声帧
     * @param left 左声道采样
     * @param right 右声道采样
     * @return QPair<伴奏左, 伴奏右>
     */
    QPair<QVector<double>, QVector<double>> process(
        const QVector<double>& left, const QVector<double>& right);

    /**
     * @brief 处理单帧(返回单声道差值)
     * @param left 左声道
     * @param right 右声道
     * @return 人声消除后的单声道
     */
    QVector<double> processMono(const QVector<double>& left, const QVector<double>& right);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 处理完成 @param samples 采样数 @param mode 消除模式 */
    void processCompleted(int samples, int mode);

private:
    /** @brief 频域模式处理 */
    QPair<QVector<double>, QVector<double>> processFrequencyDomain(
        const QVector<double>& left, const QVector<double>& right);

    /** @brief 简单DFT(用于频域模式的小FFT) */
    static void fft(QVector<double>& real, QVector<double>& imag, bool inverse);

    /** @brief 计算Hann窗 */
    QVector<double> hannWindow(int size) const;

    RemovalMode m_mode = RemovalMode::SimpleSubtract;
    double m_lowPassFreq = 200.0;
    int m_fftSize = 2048;

    Stats m_stats;
    double m_timeSum = 0.0;
};

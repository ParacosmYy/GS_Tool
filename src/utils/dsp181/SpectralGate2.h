/**
 * @file SpectralGate2.h
 * @brief 频谱门限(STFT bin级门限+噪声底估计+二元掩码平滑降噪) — Spectral Gate with STFT Bin-level Gating, Noise Floor Estimation and Musical-noise Reduction via Binary Mask Smoothing
 *
 * 功能: 实现频谱门限降噪，支持STFT bin级门限控制、噪声底自适应估计、
 *       二元掩码平滑减少音乐噪声。
 *
 * 协作: WienerFilter2(维纳滤波) / SpectralSub3(谱减法) / VAD4(语音活动检测)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 频谱门限降噪器(STFT+噪声底+掩码平滑)
 */
class SpectralGate2 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalFrames = 0;
        int fftSize = 0;
        int hopSize = 0;
        double noiseFloorDb = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SpectralGate2(QObject *parent = nullptr);
    ~SpectralGate2() override;

    void setFftSize(int size);
    void setHopSize(int size);
    void setThreshold(double db);
    void setNoiseEstimateFrames(int frames);
    void setSmoothingWidth(int bins);
    void setAttack(double ms);
    void setRelease(double ms);

    /** @brief 处理音频数据 */
    QVector<double> process(const QVector<double>& input);

    /** @brief 估计噪声底(前N帧平均) */
    QVector<double> estimateNoiseFloor(const QVector<QVector<double>>& spectra) const;

    /** @brief 计算STFT */
    QVector<QVector<double>> stft(const QVector<double>& input) const;

    /** @brief 计算逆STFT */
    QVector<double> istft(const QVector<QVector<double>>& spectra) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int numFrames, double noiseFloorDb);

private:
    int m_fftSize = 1024;
    int m_hopSize = 512;
    double m_thresholdDb = -40.0;
    int m_noiseFrames = 10;
    int m_smoothWidth = 3;
    double m_attackMs = 5.0;
    double m_releaseMs = 50.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    QVector<double> m_noiseFloor; ///< Per-bin noise floor (magnitude)

    /** @brief Apply Hann window */
    QVector<double> hannWindow(int size) const;

    /** @brief Magnitude spectrum from real signal */
    QVector<double> magnitudeSpectrum(const QVector<double>& frame) const;

    /** @brief Apply binary mask with smoothing */
    QVector<double> smoothMask(const QVector<double>& mask) const;

    /** @brief Overlap-add synthesis */
    QVector<double> overlapAdd(const QVector<QVector<double>>& frames) const;
};

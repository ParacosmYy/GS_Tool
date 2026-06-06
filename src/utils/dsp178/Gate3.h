/**
 * @file Gate3.h
 * @brief 噪声门(频谱检测+自适应阈值+双频段) — Noise Gate with Spectral Detection, Adaptive Threshold from Noise Profile and Dual-band Operation
 *
 * 功能: 实现噪声门效果器，支持频谱噪声检测、从噪声轮廓自适应阈值、
 *       双频段独立门控和包络跟随器。
 *
 * 协作: Compressor3(压缩器) / FilterBank3(滤波器组) / Analyzer3(分析器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 噪声门处理器(频谱检测+双频段)
 */
class Gate3 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalFrames = 0;
        int frameSize = 0;
        int sampleRate = 0;
        double lowThreshold = 0.0;
        double highThreshold = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Gate3(QObject *parent = nullptr);
    ~Gate3() override;

    void setSampleRate(int sr);
    void setThreshold(double lowDb, double highDb);
    void setAttack(double ms);
    void setRelease(double ms);
    void setHold(double ms);
    void setRatio(double r);

    /** @brief 学习噪声轮廓(采集噪声期间调用) */
    void learnNoiseProfile(const QVector<double>& frame);

    /** @brief 处理一帧音频(双频段门控) */
    QVector<double> process(const QVector<double>& frame);

    /** @brief 获取当前频谱能量(低/高频段) */
    QPair<double, double> bandEnergies() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void frameProcessed(int frameSize, double gainReduction);

private:
    int m_sampleRate = 44100;
    double m_lowThreshDb = -40.0;
    double m_highThreshDb = -30.0;
    double m_attackMs = 1.0;
    double m_releaseMs = 50.0;
    double m_holdMs = 10.0;
    double m_ratio = 10.0;

    /* Noise profile */
    QVector<double> m_noiseSpectrum;
    bool m_noiseLearned = false;

    /* Envelope state */
    double m_envLow = 0.0;
    double m_envHigh = 0.0;
    double m_gainLow = 1.0;
    double m_gainHigh = 1.0;
    int m_holdCount = 0;

    QPair<double, double> m_lastBands;

    Stats m_stats;
    double m_timeSum = 0.0;

    void fftSpectrum(const QVector<double>& frame,
                     QVector<double>& magnitude) const;
    double adaptiveThreshold(const QVector<double>& spectrum,
                              int bandStart, int bandEnd) const;
    double computeGain(double envDb, double threshDb);
};

/**
 * @file Resampler11.h
 * @brief 重采样器(多相抗混叠滤波器与sinc插值实现线性相位保持的任意比采样率转换) — Resampler with Polyphase Anti-aliasing Filter and Sinc-interpolation for Arbitrary Ratio Sample Rate Conversion with Linear-phase Preservation
 *
 * 功能: 实现重采样器(Resampler)，采用多相抗混叠滤波器(polyphase anti-aliasing filter)
 *       与sinc插值(sinc-interpolation)实现线性相位保持的任意比采样率转换(arbitrary ratio sample rate conversion with linear-phase preservation)。
 *
 * 协作: Flanger9(镶边效果) / ChorusEffect8(合唱效果) / DelayLine6(延迟线)
 */
#pragma once

#include <QObject>
#include <QVector>

class Resampler11 : public QObject {
    Q_OBJECT

public:
    /** @brief Resampling quality preset */
    enum Quality { Fast, Medium, High };

    /** @brief Resampling result */
    struct ResampleResult {
        QVector<double> output;
        int inputSamples = 0;
        int outputSamples = 0;
        double actualRatio = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalSamples = 0;
        double inputRate = 0.0;
        double outputRate = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Resampler11(QObject *parent = nullptr);
    ~Resampler11() override;

    /** @brief Set input and output sample rates */
    void setRates(double inputRate, double outputRate);

    /** @brief Set filter quality */
    void setQuality(Quality quality);

    /** @brief Process a block of samples */
    ResampleResult process(const QVector<double>& input);

    /** @brief Reset internal state (clear filter history) */
    void resetState();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processDone(int inSamples, int outSamples, double timeMs);

private:
    double m_inputRate = 44100.0;
    double m_outputRate = 44100.0;
    Quality m_quality = Medium;
    Stats m_stats;
    double m_timeSum = 0.0;

    // Polyphase filter coefficients
    QVector<QVector<double>> m_polyphaseCoeffs;
    int m_filterLength = 64;
    int m_numPhases = 32;

    // Filter state for overlap-save
    QVector<double> m_filterState;
    int m_statePos = 0;

    // Fractional sample position
    double m_phaseAccum = 0.0;
    double m_ratio = 1.0;      // outputRate / inputRate

    /** @brief Design polyphase filter using windowed sinc */
    void designFilter();

    /** @brief Compute windowed sinc kernel */
    double sinc(double x) const;

    /** @brief Blackman window */
    double blackman(int n, int N) const;

    /** @brief Apply polyphase filter at given fractional delay */
    double applyPolyphase(const QVector<double>& history, int phase) const;
};

/**
 * @file Resampler3.h
 * @brief 重采样器(多相抗混叠滤波器组+Sinc插值+线性/三次回退) — Resampler with Polyphase Anti-Alias Filter Bank, Sinc Interpolation and Linear/Cubic Fallback Modes
 *
 * 功能: 实现信号重采样，支持多相抗混叠滤波器组、
 *       Sinc插值、线性/三次回退模式和分数倍采样率转换。
 *
 * 协作: FIRFilter3(FIR滤波) / CICFilter3(CIC抽取) / Resampler2(基本重采样)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 重采样器(多相滤波+Sinc插值+线性/三次回退)
 */
class Resampler3 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSamplesIn = 0;
        quint64 totalSamplesOut = 0;
        double inputRate = 0.0;
        double outputRate = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Interpolation mode */
    enum Mode { Linear, Cubic, Sinc };

    explicit Resampler3(QObject *parent = nullptr);
    ~Resampler3() override;

    void setRatio(double ratio);
    void setMode(Mode mode);
    void setFilterTaps(int taps);
    void setSincLobes(int lobes);

    /** @brief Resample a block of samples */
    QVector<double> process(const QVector<double>& input);

    /** @brief Reset internal state */
    void reset();

    double ratio() const { return m_ratio; }
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void resamplingCompleted(int samplesIn, int samplesOut, double timeMs);

private:
    double m_ratio = 1.0;
    Mode m_mode = Sinc;
    int m_filterTaps = 64;
    int m_sincLobes = 16;

    // Polyphase filter bank: m_polyPhase[phase][tap]
    QVector<QVector<double>> m_polyPhase;
    int m_numPhases = 0;

    // Input history buffer for interpolation
    QVector<double> m_history;
    int m_historySize = 0;

    // Fractional position tracking
    double m_phaseAccum = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build polyphase filter bank from windowed sinc */
    void buildPolyPhase();

    /** @brief Sinc function: sin(pi*x)/(pi*x) */
    double sinc(double x) const;

    /** @brief Blackman window */
    double blackman(int n, int N) const;

    /** @brief Linear interpolation */
    double interpLinear(double frac, double y0, double y1) const;

    /** @brief Cubic (Hermite) interpolation */
    double interpCubic(double frac, double y0, double y1, double y2, double y3) const;
};

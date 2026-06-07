/**
 * @file ChirpZ6.h
 * @brief 线性调频Z变换(IIR滤波器连续扫频分析) — Chirp-Z Transform with IIR Filter-Based Implementation for Continuous Frequency Sweep Analysis
 *
 * 功能: 实现Chirp-Z变换，支持IIR滤波器实现、
 *       连续频率扫描分析和任意频段高分辨率频谱。
 *
 * 协作: WinogradFFT5(Winograd大FFT) / Goertzel5(Goertzel算法) / FftEngine3(FFT引擎)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 线性调频Z变换(IIR滤波器实现)
 */
class ChirpZ6 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalTransforms = 0;
        int inputSize = 0;
        int outputSize = 0;
        int numPoints = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ChirpZ6(QObject *parent = nullptr);
    ~ChirpZ6() override;

    /** @brief Set the frequency range for analysis */
    void setFrequencyRange(double fMin, double fMax, int numPoints);

    /** @brief Set spiral parameters (contour) */
    void setSpiral(double radius, double angleStep);

    /** @brief Compute Chirp-Z transform */
    QVector<QVector<double>> transform(const QVector<double>& input);

    /** @brief Compute inverse Chirp-Z */
    QVector<double> inverse(const QVector<QVector<double>>& spectrum, int outLen);

    /** @brief Analyze with IIR filter-based continuous sweep */
    QVector<double> iirSweep(const QVector<double>& input);

    /** @brief Get frequency bins */
    QVector<double> frequencyBins() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int nIn, int nOut, double timeMs);

private:
    double m_fMin = 0.0;
    double m_fMax = 1.0;
    int m_numPoints = 512;
    double m_radius = 1.0;
    double m_angleStep = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute twiddle chirp sequence */
    QVector<QVector<double>> computeChirp(int N, int M) const;

    /** @brief Circular convolution via FFT */
    QVector<QVector<double>> circularConvolve(
        const QVector<QVector<double>>& a,
        const QVector<QVector<double>>& b, int len) const;

    /** @brief Simple radix-2 FFT for internal use */
    void fft(QVector<double>& re, QVector<double>& im, bool inverse) const;

    /** @brief Next power of 2 */
    static int nextPow2(int n);
};

/**
 * @file WindowFunction4.h
 * @brief 窗函数(Dolph-Chebyshev+Slepian DPSS+频谱泄漏分析) — Window Function with Dolph-Chebyshev and Slepian DPSS Windows and Spectral Leakage Analysis Metrics
 *
 * 功能: 实现窗函数(window function)库，包括Dolph-Chebyshev窗和Slepian DPSS窗
 *       (discrete prolate spheroidal sequences)，并提供频谱泄漏分析指标(spectral
 *       leakage analysis metrics)如等效噪声带宽、旁瓣衰减和3dB带宽。
 *
 * 协作: Goertzel8(Goertzel算法) / SplitRadixFFT7(分裂基FFT) / MultibandGate4(多频段门控)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 窗函数(Dolph-Chebyshev+Slepian DPSS+频谱泄漏分析)
 */
class WindowFunction4 : public QObject {
    Q_OBJECT

public:
    /** @brief Window type enumeration */
    enum WindowType {
        Hann = 0,
        Hamming = 1,
        Blackman = 2,
        DolphChebyshev = 3,
        SlepianDPSS = 4,
        Kaiser = 5
    };

    /** @brief Spectral leakage metrics */
    struct LeakageMetrics {
        double enbw = 0.0;          // Equivalent noise bandwidth (bins)
        double sidelobeLevel = 0.0; // Peak sidelobe level (dB)
        double threeDbBw = 0.0;     // 3dB bandwidth (bins)
        double scallopLoss = 0.0;   // Maximum scalloping loss (dB)
        double coherenceGain = 0.0; // Coherent gain
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int windowSize = 0;
        int windowType = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit WindowFunction4(QObject *parent = nullptr);
    ~WindowFunction4() override;

    /** @brief Generate window of given type and size */
    QVector<double> generate(WindowType type, int size);

    /** @brief Generate Dolph-Chebyshev window with sidelobe attenuation (dB) */
    QVector<double> dolphChebyshev(int size, double sidelobeAttenuationDb = 60.0);

    /** @brief Generate Slepian DPSS window with time-bandwidth product */
    QVector<double> slepianDPSS(int size, double timeBandwidth = 3.0);

    /** @brief Apply window to signal */
    QVector<double> apply(const QVector<double>& signal, WindowType type);

    /** @brief Analyze spectral leakage metrics of a window */
    LeakageMetrics analyzeLeakage(const QVector<double>& window) const;

    /** @brief Get normalized frequency response of window */
    QVector<double> frequencyResponse(const QVector<double>& window, int fftSize) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void windowGenerated(int type, int size, double enbw);
    void analysisCompleted(double sidelobeDb, double scallopLoss);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Modified Bessel function I0 for Kaiser window */
    double besselI0(double x) const;

    /** @brief Compute Chebyshev polynomial T_n(x) */
    double chebyshevPoly(int n, double x) const;

    /** @brief Solve tridiagonal system for DPSS */
    QVector<double> solveTridiagonal(int n, const QVector<double>& diag,
                                      const QVector<double>& offDiag) const;
};

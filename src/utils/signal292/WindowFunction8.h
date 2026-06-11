/**
 * @file WindowFunction8.h
 * @brief 窗函数(卷积平底设计与等误码率旁瓣优化实现精确幅度测量) — Window Function with Convolution-based Flat-top Design and Equal-error-rate Sidelobe Optimization for Precision Amplitude Measurement
 *
 * 功能: 实现窗函数(window function)，采用卷积平底设计(convolution-based flat-top design)
 *       与等误码率旁瓣优化(equal-error-rate sidelobe optimization)实现精确幅度测量(precision amplitude measurement)。
 *
 * 协作: SplitRadixFFT11(FFT) / Goertzel12(Goertzel) / WindowFunction7(窗函数基类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 窗函数(卷积平底设计与等误码率旁瓣优化实现精确幅度测量)
 */
class WindowFunction8 : public QObject {
    Q_OBJECT

public:
    /** @brief Window type enumeration */
    enum WindowType {
        FlatTop = 0,
        ConvolvedFlatTop,
        EqualErrorSidelobe,
        KaiserOptimized,
        DolphChebyshev,
        GaussianPrecise,
        BlackmanHarris,
        Custom
    };

    /** @brief Window analysis result */
    struct WindowAnalysis {
        double coherentGain = 0.0;
        double scallopLoss = 0.0;
        double mainlobeWidth = 0.0;     // In bins
        double highestSidelobe = 0.0;   // In dB
        double noiseBandwidth = 0.0;    // ENBW in bins
        QVector<double> frequencyResponse;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int windowSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit WindowFunction8(QObject *parent = nullptr);
    ~WindowFunction8() override;

    void setWindowType(WindowType type);
    void setWindowSize(int N);
    void setKaiserBeta(double beta);
    void setChebyshevAttenuation(double attenDb);

    /** @brief Generate the window coefficients */
    QVector<double> generate() const;

    /** @brief Apply window to a signal block */
    QVector<double> apply(const QVector<double>& signal) const;

    /** @brief Analyze window properties in frequency domain */
    WindowAnalysis analyze() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void windowGenerated(int N, WindowType type, double timeMs);

private:
    WindowType m_type = FlatTop;
    int m_N = 256;
    double m_kaiserBeta = 8.0;
    double m_chebyAtten = 80.0;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Flat-top window (exact coefficients) */
    QVector<double> genFlatTop(int N) const;

    /** @brief Convolved flat-top: convolve two windows for wider flat region */
    QVector<double> genConvolvedFlatTop(int N) const;

    /** @brief Equal-error-rate sidelobe optimized window */
    QVector<double> genEqualErrorSidelobe(int N) const;

    /** @brief Kaiser window with given beta */
    QVector<double> genKaiser(int N, double beta) const;

    /** @brief Dolph-Chebyshev window */
    QVector<double> genDolphChebyshev(int N, double attenDb) const;

    /** @brief Modified Bessel function I0(x) for Kaiser window */
    double besselI0(double x) const;

    /** @brief Compute DFT magnitude at a given bin (zero-padded) */
    double dftBin(const QVector<double>& win, double freqBin) const;
};

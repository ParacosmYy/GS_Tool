/**
 * @file WindowFunction5.h
 * @brief 窗函数(Kaiser-Bessel与Gauss可调参数窗+旁瓣电平比较度量) — Window Function with Kaiser-Bessel and Gaussian Adjustable Parameter Windows and Side-Lobe Level Comparison Metrics
 *
 * 功能: 实现多种窗函数(Window Functions)，支持Kaiser-Bessel与Gaussian
 *       可调参数窗(Kaiser-Bessel/Gaussian adjustable parameter windows)，
 *       提供旁瓣电平比较度量(side-lobe level comparison metrics)。
 *
 * 协作: Goertzel9(Goertzel算法) / SplitRadixFFT8(分裂基FFT) / MultibandGate5(多频段门控)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 窗函数(Kaiser-Bessel/Gauss可调参数+旁瓣电平比较)
 */
class WindowFunction5 : public QObject {
    Q_OBJECT

public:
    /** @brief Supported window types */
    enum WindowType {
        Rectangular, Hann, Hamming, Blackman, BlackmanHarris,
        KaiserBessel, Gaussian, FlatTop, Nuttall, Tukey
    };

    /** @brief Window quality metrics */
    struct WindowMetrics {
        double coherentGain = 0.0;
        double sideLobeLevelDb = 0.0;
        double mainLobeWidth = 0.0;
        double scallopingLossDb = 0.0;
        double enbw = 0.0;    // Equivalent noise bandwidth
        double processingGain = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int windowSize = 0;
        int numWindows = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit WindowFunction5(QObject *parent = nullptr);
    ~WindowFunction5() override;

    /** @brief Generate a window of given type and size */
    QVector<double> generate(WindowType type, int size);

    /** @brief Generate Kaiser window with beta parameter */
    QVector<double> kaiser(int size, double beta);

    /** @brief Generate Gaussian window with sigma parameter */
    QVector<double> gaussian(int size, double sigma);

    /** @brief Apply window to signal */
    QVector<double> apply(const QVector<double>& window,
                           const QVector<double>& signal) const;

    /** @brief Compute window quality metrics */
    WindowMetrics computeMetrics(const QVector<double>& window) const;

    /** @brief Compare side-lobe levels of all window types */
    QVector<QPair<WindowType, double>> compareSideLobes(int size) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void windowGenerated(int size, int type, double sideLobeDb, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Modified Bessel function I0(x) for Kaiser window */
    double besselI0(double x) const;

    /** @brief Compute ENBW (equivalent noise bandwidth) */
    double computeENBW(const QVector<double>& w) const;

    /** @brief Compute coherent gain */
    double computeCoherentGain(const QVector<double>& w) const;

    /** @brief Estimate side-lobe level from window spectrum */
    double estimateSideLobeDb(const QVector<double>& w) const;

    /** @brief Compute scalloping loss */
    double computeScallopingLoss(const QVector<double>& w) const;

    /** @brief Basic window generation helpers */
    QVector<double> genHann(int size) const;
    QVector<double> genHamming(int size) const;
    QVector<double> genBlackman(int size) const;
    QVector<double> genBlackmanHarris(int size) const;
    QVector<double> genFlatTop(int size) const;
    QVector<double> genNuttall(int size) const;
    QVector<double> genTukey(int size, double alpha) const;
};

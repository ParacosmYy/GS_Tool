/**
 * @file WindowFunction6.h
 * @brief 窗函数(Kaiser参数化旁瓣衰减平顶加权精确振幅测量) — Window Function with Kaiser Parameterized Sidelobe Attenuation and Flat-top Weight for Accurate Amplitude Measurement
 *
 * 功能: 实现窗函数(window function)，采用Kaiser参数化旁瓣衰减(Kaiser parameterized
 *       sidelobe attenuation)和平顶加权(flat-top weight)用于精确振幅测量(accurate
 *       amplitude measurement)。
 *
 * 协作: Goertzel10(Goertzel算法) / SplitRadixFFT9(分裂基数FFT) / SlidingDFT9(滑动DFT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 窗函数(Kaiser参数化旁瓣衰减平顶加权精确振幅测量)
 */
class WindowFunction6 : public QObject {
    Q_OBJECT

public:
    /** @brief Window type enumeration */
    enum WindowType {
        Rectangular = 0,
        Hann,
        Hamming,
        Blackman,
        BlackmanHarris,
        Kaiser,
        FlatTop,
        Nuttall,
        Gaussian
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int windowSize = 0;
        int windowType = Rectangular;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Window analysis metrics */
    struct WindowMetrics {
        double coherentGain = 1.0;
        double sidelobeLevelDb = 0.0;
        double noiseBandwidth = 1.0;
        double threeDbBandwidth = 1.0;
        double scallopingLoss = 0.0;
    };

    explicit WindowFunction6(QObject *parent = nullptr);
    ~WindowFunction6() override;

    /** @brief Generate window coefficients */
    QVector<double> generate(WindowType type, int size, double parameter = 0.0);

    /** @brief Apply window to signal */
    QVector<double> apply(const QVector<double>& signal, WindowType type,
                           double parameter = 0.0);

    /** @brief Compute window analysis metrics */
    WindowMetrics computeMetrics(const QVector<double>& window) const;

    /** @brief Get Kaiser beta from desired sidelobe attenuation (dB) */
    double kaiserBetaFromAttenuation(double attenuationDb) const;

    /** @brief Compute Kaiser window coefficient at index */
    double kaiserCoefficient(int n, int N, double beta) const;

    /** @brief Compute flat-top window coefficient at index */
    double flatTopCoefficient(int n, int N) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void windowGenerated(int size, int type, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Bessel function I0(x) for Kaiser window */
    double besselI0(double x) const;

    /** @brief Generate specific window type */
    QVector<double> generateRectangular(int n) const;
    QVector<double> generateHann(int n) const;
    QVector<double> generateHamming(int n) const;
    QVector<double> generateBlackman(int n) const;
    QVector<double> generateBlackmanHarris(int n) const;
    QVector<double> generateNuttall(int n) const;
    QVector<double> generateGaussian(int n, double sigma) const;
};

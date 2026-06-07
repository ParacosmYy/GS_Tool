/**
 * @file WindowFunction3.h
 * @brief 窗函数设计器(Dolph-Chebyshev旁瓣控制+Kaiser参数扫描) — Window Function Designer with Dolph-Chebyshev Sidelobe Control and Kaiser Parameter Sweep
 *
 * 功能: 实现多种窗函数生成器，支持Dolph-Chebyshev旁瓣控制、
 *       Kaiser参数扫描优化和窗函数性能指标分析。
 *
 * 协作: ChirpZ7(CZT) / Goertzel6(频率检测) / MultibandGate3(多频段门控)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QMap>

/**
 * @brief 窗函数设计器(Dolph-Chebyshev旁瓣控制+Kaiser参数扫描)
 */
class WindowFunction3 : public QObject {
    Q_OBJECT

public:
    /** @brief Window type enumeration */
    enum WindowType {
        Rectangular, Hann, Hamming, Blackman, BlackmanHarris,
        Kaiser, DolphChebyshev, FlatTop, Nuttall, Gaussian
    };
    Q_ENUM(WindowType)

    /** @brief Window performance metrics */
    struct WindowMetrics {
        double sidelobeLevel = 0.0;     // dB
        double mainlobeWidth = 0.0;     // bins
        double coherentGain = 0.0;
        double enbw = 0.0;              // equivalent noise bandwidth
        double scallopLoss = 0.0;       // dB
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int windowSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit WindowFunction3(QObject *parent = nullptr);
    ~WindowFunction3() override;

    /** @brief Generate a window of given type and size */
    QVector<double> generate(WindowType type, int size) const;

    /** @brief Generate Kaiser window with specific beta */
    QVector<double> kaiserWindow(int size, double beta) const;

    /** @brief Generate Dolph-Chebyshev window with sidelobe attenuation */
    QVector<double> dolphChebyshevWindow(int size, double sidelobeDb) const;

    /** @brief Optimize Kaiser beta for desired sidelobe level */
    double optimizeKaiserBeta(double targetSidelobeDb) const;

    /** @brief Sweep Kaiser beta and return metrics for each */
    QMap<double, WindowMetrics> kaiserSweep(int size, double betaMin, double betaMax,
                                             double step) const;

    /** @brief Compute window performance metrics */
    WindowMetrics computeMetrics(const QVector<double>& window) const;

    /** @brief Apply window to signal */
    QVector<double> apply(const QVector<double>& signal, const QVector<double>& window) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void windowGenerated(const QString& type, int size, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Bessel function I0(x) for Kaiser window */
    static double besselI0(double x);

    /** @brief Compute DFT magnitude at bin k */
    static double dftMagnitude(const QVector<double>& window, double freq);

    /** @brief Chebyshev polynomial T_n(x) */
    static double chebyshevPoly(int n, double x);
};

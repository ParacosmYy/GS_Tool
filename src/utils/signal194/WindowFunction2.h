/**
 * @file WindowFunction2.h
 * @brief 窗函数比较器(主瓣宽度+旁瓣电平+扇贝损耗指标) — Window Function Comparator with Mainlobe Width, Sidelobe Level and Scalloping Loss Metrics
 *
 * 功能: 实现窗函数比较分析器，支持主瓣宽度测量、
 *       旁瓣电平分析和扇贝损耗指标计算。
 *
 * 协作: FftEngine3(FFT引擎) / Goertzel5(Goertzel) / DHT4(哈特利)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 窗函数比较器(主瓣+旁瓣+扇贝损耗)
 */
class WindowFunction2 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalAnalysis = 0;
        int windowSize = 0;
        int numWindowsCompared = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Window analysis metrics */
    struct Metrics {
        double mainlobeWidth3dB = 0.0;
        double mainlobeWidth6dB = 0.0;
        double peakSidelobeLevel = 0.0;   // in dB
        double sidelobeRollOff = 0.0;     // dB/octave
        double scallopingLoss = 0.0;      // in dB
        double coherentGain = 0.0;
        double enbw = 0.0;                // Equivalent noise bandwidth
    };

    /** @brief Supported window types */
    enum WindowType {
        Rectangular, Hann, Hamming, Blackman, BlackmanHarris,
        Nuttall, FlatTop, Kaiser, Gaussian, Tukey
    };

    explicit WindowFunction2(QObject *parent = nullptr);
    ~WindowFunction2() override;

    void setWindowSize(int n);
    void setFFTSize(int fftSize);

    /** @brief Generate window coefficients */
    QVector<double> generate(WindowType type, double param = 0.0) const;

    /** @brief Analyze a single window function */
    Metrics analyze(WindowType type, double param = 0.0);

    /** @brief Compare multiple windows, return sorted by metric */
    QVector<QPair<WindowType, Metrics>> compare(
        const QVector<WindowType>& types);

    /** @brief Get window name as string */
    static QString windowName(WindowType type);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void analysisCompleted(const QString& name, double sidelobe, double timeMs);

private:
    int m_windowSize = 256;
    int m_fftSize = 1024;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute magnitude spectrum of window (zero-padded) */
    QVector<double> magnitudeSpectrum(const QVector<double>& window) const;

    /** @brief Find mainlobe width at given dB level */
    double findMainlobeWidth(const QVector<double>& mag, double dbLevel) const;

    /** @brief Find peak sidelobe level in dB */
    double findPeakSidelobe(const QVector<double>& mag) const;

    /** @brief Compute scalloping loss */
    double computeScallopingLoss(const QVector<double>& mag) const;

    /** @brief Compute equivalent noise bandwidth */
    double computeENBW(const QVector<double>& window) const;
};

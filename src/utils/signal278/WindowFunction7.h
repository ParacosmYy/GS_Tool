/**
 * @file WindowFunction7.h
 * @brief 窗函数(Dolph-Chebyshev等旁瓣设计与可调alpha高斯窗的频谱分析) — Window Function with Dolph-Chebyshev Equi-sidelobe Design and Gaussian Window with Adjustable Alpha for Spectral Analysis
 *
 * 功能: 实现窗函数(Window function)，采用Dolph-Chebyshev等旁瓣设计(Dolph-Chebyshev equi-sidelobe design)
 *       与可调alpha高斯窗(Gaussian window with adjustable alpha)实现频谱分析(spectral analysis)。
 *
 * 协作: Goertzel11(Goertzel算法) / SplitRadixFFT10(分裂基数FFT) / FirFilter10(FIR滤波器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 窗函数(Dolph-Chebyshev等旁瓣设计与可调alpha高斯窗)
 */
class WindowFunction7 : public QObject {
    Q_OBJECT

public:
    /** @brief Supported window types */
    enum WindowType {
        DolphChebyshev = 0,   // Equi-sidelobe design with specified SLL
        Gaussian,             // Adjustable alpha parameter
        Kaiser,               // Adjustable beta parameter
        Hann,                 // Raised cosine
        Hamming,              // Optimized cosine
        BlackmanHarris,       // 4-term cosine
        FlatTop,              // Flat passband for measurement
        FlattenedCosine       // Hybrid flat-top + cosine
    };

    /** @brief Window analysis metrics */
    struct WindowMetrics {
        double mainLobeWidth3dB = 0.0;    // 3-dB bandwidth (bins)
        double mainLobeWidth6dB = 0.0;    // 6-dB bandwidth (bins)
        double sideLobeLevel = 0.0;       // Peak sidelobe level (dB)
        double sideLobeDecay = 0.0;       // Sidelobe decay rate (dB/oct)
        double coherentGain = 0.0;        // Coherent gain (normalized)
        double ENBW = 0.0;                // Equivalent noise bandwidth (bins)
        double scallopingLoss = 0.0;      // Scalloping loss (dB)
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int windowLength = 0;
        int numWindows = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit WindowFunction7(QObject *parent = nullptr);
    ~WindowFunction7() override;

    /** @brief Set window length N */
    void setLength(int n);

    /** @brief Set window type */
    void setWindowType(WindowType type);

    /** @brief Set Dolph-Chebyshev sidelobe attenuation (dB) */
    void setChebyshevSLL(double sllDb);

    /** @brief Set Gaussian alpha parameter */
    void setGaussianAlpha(double alpha);

    /** @brief Set Kaiser beta parameter */
    void setKaiserBeta(double beta);

    /** @brief Generate window coefficients */
    QVector<double> generate();

    /** @brief Apply window to signal */
    QVector<double> apply(const QVector<double>& signal);

    /** @brief Compute window analysis metrics */
    WindowMetrics computeMetrics();

    /** @brief Get current coefficients */
    QVector<double> coefficients() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void windowGenerated(int length, int type, double mainLobeWidth, double timeMs);

private:
    int m_length = 256;
    WindowType m_type = DolphChebyshev;
    double m_chebyshevSLL = -60.0;   // Sidelobe level in dB
    double m_gaussianAlpha = 2.5;    // Gaussian width parameter
    double m_kaiserBeta = 8.0;       // Kaiser window beta

    QVector<double> m_coeffs;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Generate Dolph-Chebyshev window */
    QVector<double> generateDolphChebyshev(int n, double sllDb) const;

    /** @brief Generate Gaussian window */
    QVector<double> generateGaussian(int n, double alpha) const;

    /** @brief Generate Kaiser window */
    QVector<double> generateKaiser(int n, double beta) const;

    /** @brief Generate standard cosine window */
    QVector<double> generateCosine(int n, const QVector<double>& coeffs) const;

    /** @brief Compute modified Bessel function I0(x) */
    double besselI0(double x) const;

    /** @brief Compute Chebyshev polynomial T_n(x) */
    double chebyshevPoly(int n, double x) const;
};

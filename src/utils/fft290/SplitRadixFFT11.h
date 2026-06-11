/**
 * @file SplitRadixFFT11.h
 * @brief 分裂基FFT(共轭对蝶形与旋转因子表复用最小化算术运算量) — Split-radix FFT with Conjugate-pair Butterfly and Twiddle Factor Table Reuse for Minimized Arithmetic Operation Count
 *
 * 功能: 实现分裂基FFT(Split-radix FFT)，采用共轭对蝶形(conjugate-pair butterfly)
 *       与旋转因子表复用(twiddle factor table reuse)实现最小化算术运算量(minimized arithmetic operation count)。
 *
 * 协作: MixedRadixFFT11(混合基FFT) / ZoomFFT9(缩放FFT) / SlidingDFT11(滑动DFT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 分裂基FFT(共轭对蝶形与旋转因子表复用最小化算术运算量)
 */
class SplitRadixFFT11 : public QObject {
    Q_OBJECT

public:
    /** @brief FFT result */
    struct FFTResult {
        QVector<double> real;
        QVector<double> imag;
        QVector<double> magnitude;
        QVector<double> phase;
        int n = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int lastN = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SplitRadixFFT11(QObject *parent = nullptr);
    ~SplitRadixFFT11() override;

    /** @brief Forward FFT (real input) */
    FFTResult forward(const QVector<double>& input);

    /** @brief Forward FFT (complex input) */
    FFTResult forwardComplex(const QVector<double>& re, const QVector<double>& im);

    /** @brief Inverse FFT */
    FFTResult inverse(const QVector<double>& re, const QVector<double>& im);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void fftDone(int n, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Pre-computed twiddle factor tables */
    QVector<double> m_twidRe;
    QVector<double> m_twidIm;
    int m_twidN = 0;

    /** @brief Build twiddle factor table for size n */
    void buildTwiddleTable(int n);

    /** @brief Core split-radix FFT (in-place) */
    void splitRadixCore(QVector<double>& re, QVector<double>& im,
                        int n, int stride, int base, bool inverse);

    /** @brief Bit-reverse permutation */
    void bitReverse(QVector<double>& re, QVector<double>& im, int n);

    /** @brief Next power of 2 >= n */
    static int nextPow2(int n);
};

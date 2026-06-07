/**
 * @file ChirpZ7.h
 * @brief Chirp Z变换(Bluestein递归+任意轮廓采样优化) — Chirp Z-Transform with Bluestein Recursion and Arbitrary Contour Sampling Optimization
 *
 * 功能: 实现Chirp Z变换(CZT)，支持Bluestein递归算法、
 *       任意Z平面轮廓采样和频谱细化分析。
 *
 * 协作: Goertzel6(单频检测) / SplitRadixFFT5(FFT) / WindowFunction3(窗函数)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Chirp Z变换(Bluestein递归+任意轮廓采样优化)
 */
class ChirpZ7 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalTransforms = 0;
        int inputSize = 0;
        int outputSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ChirpZ7(QObject *parent = nullptr);
    ~ChirpZ7() override;

    /** @brief Set the contour parameters: A (start), W (step ratio) */
    void setContour(const QPair<double, double>& A, const QPair<double, double>& W);

    /** @brief Set number of output samples M */
    void setOutputSize(int M);

    /** @brief Compute CZT using Bluestein recursion */
    QVector<QPair<double, double>> transform(const QVector<double>& input) const;

    /** @brief Compute CZT on complex input */
    QVector<QPair<double, double>> transformComplex(
        const QVector<QPair<double, double>>& input) const;

    /** @brief Compute CZT on unit circle (equivalent to zoomed FFT) */
    QVector<QPair<double, double>> zoomFFT(const QVector<double>& input,
                                            double fLow, double fHigh) const;

    /** @brief Inverse CZT: recover time-domain from CZT result */
    QVector<QPair<double, double>> inverseTransform(
        const QVector<QPair<double, double>>& spectrum, int N) const;

    /** @brief Bit-reverse an index for FFT */
    static int bitReverse(int x, int bits);

    /** @brief Next power of 2 */
    static int nextPow2(int n);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int inputSize, int outputSize, double timeMs);

private:
    double m_Ar = 1.0, m_Ai = 0.0;   // Contour start (complex)
    double m_Wr = 1.0, m_Wi = 0.0;   // Contour step (complex)
    int m_M = 64;                      // Output size

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief In-place radix-2 Cooley-Tukey FFT */
    static void fftImpl(QVector<QPair<double, double>>& data, bool inverse);

    /** @brief Complex multiply */
    static QPair<double, double> cmul(const QPair<double, double>& a,
                                       const QPair<double, double>& b);

    /** @brief Complex conjugate */
    static QPair<double, double> conj(const QPair<double, double>& z);
};

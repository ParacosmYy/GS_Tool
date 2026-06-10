/**
 * @file SplitRadixFFT10.h
 * @brief 分裂基数FFT(共轭对旋转因子优化与实输入对称性N/2复数点变换) — Split-radix FFT with Conjugate-pair Twiddle Optimization and Real-valued Input Symmetry for N/2 Complex-point Transform
 *
 * 功能: 实现分裂基数FFT(Split-radix FFT)，采用共轭对旋转因子优化
 *       (conjugate-pair twiddle optimization)与实输入对称性(real-valued input symmetry)
 *       实现N/2复数点变换(N/2 complex-point transform)。
 *
 * 协作: MixedRadixFFT10(混合基数FFT) / RaderFFT6(Rader FFT) / BluesteinFFT7(Bluestein FFT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 分裂基数FFT(共轭对旋转因子优化与实输入对称性)
 */
class SplitRadixFFT10 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int transformSize = 0;
        int numTransforms = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SplitRadixFFT10(QObject *parent = nullptr);
    ~SplitRadixFFT10() override;

    /** @brief Set transform size N (must be power of 2, auto-rounded) */
    void setSize(int n);

    /** @brief Forward FFT of real signal using N/2 complex transform */
    QVector<double> forwardReal(const QVector<double>& input);

    /** @brief Forward FFT of complex signal (interleaved re,im pairs) */
    QVector<double> forwardComplex(const QVector<double>& re, const QVector<double>& im);

    /** @brief Inverse FFT returning real signal */
    QVector<double> inverseReal(const QVector<double>& spectrum);

    /** @brief Inverse FFT of complex spectrum */
    QVector<double> inverseComplex(const QVector<double>& re, const QVector<double>& im);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformDone(int size, int numTransforms, double timeMs);

private:
    int m_n = 256;

    // Precomputed twiddle: conjugate-pair stored as [cos_k, sin_k]
    QVector<double> m_twiddlesCos;
    QVector<double> m_twiddlesSin;

    // Bit-reversal permutation table
    QVector<int> m_bitRev;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build bit-reversal table for current N */
    void buildBitRevTable();

    /** @brief Build conjugate-pair twiddle factors W_N^k */
    void buildTwiddles();

    /** @brief Core split-radix recursive butterfly */
    void splitRadixCore(QVector<double>& re, QVector<double>& im,
                        int start, int length, int stride);

    /** @brief Bit-reversal permutation in-place */
    void bitReversePermute(QVector<double>& re, QVector<double>& im) const;

    /** @brief Pack real signal into N/2 complex points */
    void packReal(const QVector<double>& input,
                  QVector<double>& re, QVector<double>& im) const;

    /** @brief Unpack N/2 complex spectrum to full N-point real spectrum */
    QVector<double> unpackRealSpectrum(const QVector<double>& re,
                                       const QVector<double>& im) const;
};

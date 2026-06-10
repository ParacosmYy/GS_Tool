/**
 * @file SplitRadixFFT9.h
 * @brief 分裂基数FFT(原位比特反转共轭对称实值输入变换) — Split-radix FFT with In-place Bit-reversal Permutation and Conjugate-symmetry Exploitation for Real-valued Inputs
 *
 * 功能: 实现分裂基数FFT(split-radix FFT)，采用原位比特反转置换(in-place
 *       bit-reversal permutation)和共轭对称利用(conjugate-symmetry
 *       exploitation)针对实值输入(real-valued inputs)进行高效变换。
 *
 * 协作: MixedRadixFFT9(混合基数FFT) / FFT4(基2FFT) / SlidingDFT9(滑动DFT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 分裂基数FFT(原位比特反转共轭对称实值输入变换)
 */
class SplitRadixFFT9 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int transformSize = 0;
        int numForwardTransforms = 0;
        int numInverseTransforms = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SplitRadixFFT9(QObject *parent = nullptr);
    ~SplitRadixFFT9() override;

    /** @brief Forward real-valued FFT, returns interleaved complex [re,im,...] of N/2+1 bins */
    QVector<double> forwardReal(const QVector<double>& input);

    /** @brief Inverse real-valued FFT from interleaved complex N/2+1 bins */
    QVector<double> inverseReal(const QVector<double>& spectrum);

    /** @brief General forward complex FFT (interleaved) */
    QVector<double> forwardComplex(const QVector<double>& interleaved);

    /** @brief General inverse complex FFT (interleaved) */
    QVector<double> inverseComplex(const QVector<double>& interleaved);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int n, bool realMode, bool inverse, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief In-place bit-reversal permutation */
    void bitReversePermute(QVector<double>& re, QVector<double>& im) const;

    /** @brief Reverse bits for index */
    int reverseBits(int val, int bits) const;

    /** @brief Compute log2 of a power-of-2 integer */
    int log2Int(int n) const;

    /** @brief Split-radix butterfly core (in-place) */
    void splitRadixCore(QVector<double>& re, QVector<double>& im, int n, bool inverse) const;

    /** @brief Pack real input into half-size complex for RFFT */
    void packReal(const QVector<double>& input,
                  QVector<double>& re, QVector<double>& im) const;

    /** @brief Unpack half-size complex spectrum back to full real spectrum */
    QVector<double> unpackSpectrum(const QVector<double>& re,
                                    const QVector<double>& im) const;

    /** @brief Pre-process for IRFFT: reconstruct half-complex from full spectrum */
    void preprocessInverse(const QVector<double>& spectrum,
                           QVector<double>& re, QVector<double>& im) const;
};

/**
 * @file WinogradFFT6.h
 * @brief Winograd大FFT(二维张量积分解小型Winograd-DFT模块) — Winograd Large FFT via 2D Tensor Product Decomposition of Small Winograd DFT Modules
 *
 * 功能: 实现Winograd FFT算法，支持小DFT模块的张量积分解、
 *       二维矩阵转置和大N点快速傅里叶变换。
 *
 * 协作: DST5(DST变换) / RaderFFT5(Rader算法) / FFTW5(FFT引擎)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Winograd大FFT(二维张量积分解小型Winograd-DFT模块)
 */
class WinogradFFT6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalTransforms = 0;
        int transformSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit WinogradFFT6(QObject *parent = nullptr);
    ~WinogradFFT6() override;

    /** @brief Set transform size (must factor into small primes) */
    void setTransformSize(int n);

    /** @brief Forward FFT: real input, returns interleaved real/imag */
    QVector<double> forward(const QVector<double>& input);

    /** @brief Inverse FFT: interleaved real/imag input */
    QVector<double> inverse(const QVector<double>& spectrum);

    /** @brief Small Winograd DFT module (N=2,3,4,5,7,8) */
    static QVector<double> smallDFT(const QVector<double>& real,
                                     const QVector<double>& imag, int n);

    /** @brief Decompose N into factors suitable for Winograd */
    QVector<int> factorize(int n) const;

    /** @brief Tensor product of two small DFT modules */
    void tensorProduct(const QVector<double>& matA, int rowsA, int colsA,
                        const QVector<double>& matB, int rowsB, int colsB,
                        QVector<double>& result) const;

    /** @brief Matrix transpose for 2D decomposition */
    static void transpose(QVector<double>& matrix, int rows, int cols);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int size, double timeMs);

private:
    int m_size = 64;

    // Precomputed twiddle factors
    QVector<double> m_twiddleReal;
    QVector<double> m_twiddleImag;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Precompute twiddle factors */
    void precompute(int n);

    /** @brief Apply small Winograd DFT to a segment */
    void applySmallDFT(QVector<double>& real, QVector<double>& imag,
                         int offset, int stride, int n) const;

    /** @brief Cooley-Tukey style combination step */
    void combine(QVector<double>& real, QVector<double>& imag,
                  int n1, int n2) const;
};

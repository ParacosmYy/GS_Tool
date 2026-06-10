/**
 * @file HexFFT10.h
 * @brief 六角FFT(六角采样定理与三轴频率分解二维六角网格变换) — Hex FFT with Hexagonal Sampling Theorem and 3-axis Frequency Decomposition for 2D Hexagonal Grid Transforms
 *
 * 功能: 实现六角FFT(Hex FFT)，采用六角采样定理(hexagonal sampling theorem)
 *       与三轴频率分解(3-axis frequency decomposition)实现二维六角网格变换(2D hexagonal grid transforms)。
 *
 * 协作: SlidingDFT10(滑动DFT) / ZoomFFT7(缩放FFT) / WinogradFFT6(Winograd FFT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 六角FFT(六角采样定理与三轴频率分解二维六角网格变换)
 */
class HexFFT10 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int gridSize = 0;
        int numTransforms = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit HexFFT10(QObject *parent = nullptr);
    ~HexFFT10() override;

    /** @brief Set hex grid size (must be power of 2 along each axis) */
    void setGridSize(int n);

    /** @brief Forward hex FFT: 2D hex grid -> 3-axis frequency domain */
    QVector<QVector<double>> forward(const QVector<QVector<double>>& hexGrid);

    /** @brief Inverse hex FFT: 3-axis frequency domain -> 2D hex grid */
    QVector<QVector<double>> inverse(const QVector<QVector<double>>& spectrum);

    /** @brief Convert rectangular grid to hexagonal grid */
    QVector<QVector<double>> rectToHex(const QVector<QVector<double>>& rect) const;

    /** @brief Convert hexagonal grid back to rectangular */
    QVector<QVector<double>> hexToRect(const QVector<QVector<double>>& hex) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformDone(int gridSize, int numTransforms, double timeMs);

private:
    int m_gridSize = 64;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief 1D FFT along one axis of the hex grid */
    void fft1D(QVector<double>& real, QVector<double>& imag, bool inverse);

    /** @brief Compute twiddle factor for hex rotation angle */
    void hexTwiddle(int k1, int k2, double& wr, double& wi) const;

    /** @brief 3-axis decomposition step */
    void decompose3Axis(QVector<QVector<double>>& real,
                        QVector<QVector<double>>& imag, bool inverse);

    /** @brief Bit-reversal permutation */
    void bitReversePermute(QVector<double>& real, QVector<double>& imag) const;

    /** @brief Bit-reverse an index */
    int bitReverse(int x, int bits) const;
};

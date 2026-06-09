/**
 * @file HexFFT8.h
 * @brief 六边形FFT(交错网格采样+六边形频域重建的二维信号分析) — Hexagonal FFT with Staggered Grid Sampling and Hexagonal Frequency Domain Reconstruction for 2D Signal Analysis
 *
 * 功能: 实现六边形FFT(Hexagonal FFT)，采用交错网格采样(staggered grid
 *       sampling)和六边形频域重建(hexagonal frequency domain reconstruction)
 *       进行二维信号分析(2D signal analysis)。
 *
 * 协作: NumberTheoreticTransform5(数论变换) / SlidingDFT8(滑动DFT) / BruunFFT8(Bruun FFT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 六边形FFT(交错网格采样+六边形频域重建)
 */
class HexFFT8 : public QObject {
    Q_OBJECT

public:
    /** @brief Hexagonal grid coordinate */
    struct HexPoint {
        int q = 0;  // Axial q coordinate
        int r = 0;  // Axial r coordinate
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int gridSize = 0;
        int numSamples = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit HexFFT8(QObject *parent = nullptr);
    ~HexFFT8() override;

    /** @brief Set grid radius (grid has 3*R*(R+1)+1 points) */
    void setGridRadius(int radius);

    /** @brief Perform forward hexagonal FFT */
    QVector<QVector<double>> forward(const QVector<QVector<double>>& input);

    /** @brief Perform inverse hexagonal FFT */
    QVector<QVector<double>> inverse(const QVector<QVector<double>>& spectrum);

    /** @brief Sample from rectangular grid with staggered rows */
    QVector<double> staggeredSample(const QVector<QVector<double>>& rectData) const;

    /** @brief Reconstruct rectangular grid from hex spectrum */
    QVector<QVector<double>> reconstruct(const QVector<QVector<double>>& hexSpectrum) const;

    /** @brief Get hex grid coordinates in order */
    QVector<HexPoint> gridCoordinates() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int gridSize, double timeMs);

private:
    int m_radius = 8;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Precomputed twiddle factors for hex lattice */
    QVector<QVector<QVector<double>>> m_twiddle;

    /** @brief Hex grid coordinate list */
    QVector<HexPoint> m_grid;

    /** @brief Build hexagonal grid coordinate list */
    void buildGrid();

    /** @brief Precompute twiddle factors on hex lattice */
    void precomputeTwiddle();

    /** @brief 1D DFT helper (radix-2 Cooley-Tukey) */
    void fft1D(QVector<double>& re, QVector<double>& im, bool inv) const;

    /** @brief Next power of two >= n */
    static int nextPow2(int n);

    /** @brief Axial-to-pixel coordinate conversion */
    QPair<double, double> axialToPixel(int q, int r) const;

    /** @brief Compute hex DFT matrix element */
    double hexDFTKernel(int q1, int r1, int q2, int r2, bool inv) const;
};

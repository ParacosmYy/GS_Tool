/**
 * @file HexFFT9.h
 * @brief 六角FFT(轴向坐标系+六重旋转对称六角格变换) — Hex FFT with Axial Coordinate System and Six-fold Rotational Symmetry Exploitation for Hexagonal Lattice Transansforms
 *
 * 功能: 实现六角网格FFT(Hex FFT)，采用轴向坐标系(axial coordinate
 *       system)和六重旋转对称(six-fold rotational symmetry)加速
 *       六角格(hexagonal lattice)变换，适用于图像处理与信号分析。
 *
 * 协作: FFT4(FFT) / SlidingDFT9(滑动DFT) / ZoomFFT6(缩放FFT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 六角FFT(轴向坐标系+六重旋转对称六角格变换)
 */
class HexFFT9 : public QObject {
    Q_OBJECT

public:
    /** @brief Axial coordinate (q, r) for hex grid */
    struct HexCoord {
        int q = 0;
        int r = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int gridSize = 0;
        int numTransforms = 0;
        int numInverseTransforms = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit HexFFT9(QObject *parent = nullptr);
    ~HexFFT9() override;

    /** @brief Set hex grid radius (grid spans -radius..+radius in q,r) */
    void setGridRadius(int radius);

    /** @brief Forward hex FFT transform */
    QVector<double> forward(const QVector<double>& inputData);

    /** @brief Inverse hex FFT transform */
    QVector<double> inverse(const QVector<double>& spectrumData);

    /** @brief Get axial coordinates for all grid points */
    QVector<HexCoord> gridCoordinates() const;

    /** @brief Compute hex distance between two axial coords */
    static int hexDistance(const HexCoord& a, const HexCoord& b);

    /** @brief Apply six-fold rotational symmetry to coefficient index */
    QVector<int> symmetryGroup(int index) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int gridSize, bool inverse, double timeMs);

private:
    int m_radius = 8;
    int m_gridSize = 0;

    QVector<HexCoord> m_coords;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build hex grid coordinates */
    void buildGrid();

    /** @brief 1D FFT for hex ring decomposition */
    void fft1D(QVector<double>& real, QVector<double>& imag, bool inverse) const;

    /** @brief Exploit six-fold symmetry to reduce computation */
    void applySymmetryOptimization(QVector<double>& data, bool inverse);

    /** @brief Bit-reversal permutation */
    void bitReverse(QVector<double>& real, QVector<double>& imag) const;

    /** @brief Convert axial to cube coordinates */
    void axialToCube(int q, int r, int& x, int& y, int& z) const;

    /** @brief Get ring indices for a given distance from center */
    QVector<int> ringIndices(int distance) const;
};

/**
 * @file HexFFT5.h
 * @brief 六角FFT(六角到矩形坐标映射+六重对称利用) — Hexagonal FFT with Hexagonal-to-Rectangular Coordinate Mapping and 6-Fold Symmetry Exploitation
 *
 * 功能: 实现六角形FFT，支持六角到矩形坐标映射、
 *       六重对称利用和六角采样数据处理。
 *
 * 协作: ChirpZ6(Chirp-Z变换) / WinogradFFT5(Winograd FFT) / FftEngine3(FFT引擎)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 六角FFT(坐标映射+六重对称)
 */
class HexFFT5 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalTransforms = 0;
        int gridSize = 0;
        int numHexPoints = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Hexagonal sample point */
    struct HexPoint {
        int q = 0;              // axial q coordinate
        int r = 0;              // axial r coordinate
        double value = 0.0;     // sample value
    };

    explicit HexFFT5(QObject *parent = nullptr);
    ~HexFFT5() override;

    void setGridRadius(int radius);
    void setSymmetryOrder(int order);
    void setNormalize(bool enable);

    /** @brief Transform hexagonal samples to frequency domain */
    QVector<QVector<double>> forward(const QVector<HexPoint>& samples);

    /** @brief Inverse transform back to hexagonal domain */
    QVector<HexPoint> inverse(const QVector<QVector<double>>& spectrum, int radius);

    /** @brief Map hex axial (q,r) to rectangular (x,y) */
    QPair<double, double> hexToRect(int q, int r) const;

    /** @brief Map rectangular (x,y) back to hex (q,r) */
    QPair<int, int> rectToHex(double x, double y) const;

    /** @brief Generate hexagonal grid of given radius */
    QVector<HexPoint> generateGrid(int radius) const;

    /** @brief Exploit 6-fold symmetry to reduce computation */
    QVector<HexPoint> reduceBySymmetry(const QVector<HexPoint>& samples) const;

    /** @brief Reconstruct full data from symmetry-reduced subset */
    QVector<HexPoint> reconstructSymmetry(const QVector<HexPoint>& reduced) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int numPoints, double timeMs);

private:
    int m_gridRadius = 8;
    int m_symmetryOrder = 6;
    bool m_normalize = true;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Internal radix-2 FFT */
    void fft(QVector<double>& re, QVector<double>& im, bool inv) const;

    /** @brief Next power of 2 */
    static int nextPow2(int n);

    /** @brief Hex distance */
    static int hexDist(int q, int r);

    /** @brief Rotate hex point by 60 degrees */
    static HexPoint rotate60(const HexPoint& p);

    /** @brief Count hex grid points for radius */
    static int hexGridSize(int radius);
};

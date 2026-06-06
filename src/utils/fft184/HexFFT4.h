/**
 * @file HexFFT4.h
 * @brief 六角FFT(非正交基六角网格采样数据) — Hexagonal FFT with Non-orthogonal Basis for Hex-grid Sampled Data
 *
 * 功能: 实现六角形网格采样的FFT算法，支持非正交基变换、
 *       六角网格到矩形网格转换、轴对称频谱分析和快速卷积。
 *
 * 协作: SplitRadixFFT4(分裂基) / MixedRadixFFT4(混合基) / RaderFFT4(Rader)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 六角FFT处理器(非正交基)
 */
class HexFFT4 : public QObject {
    Q_OBJECT

public:
    /** @brief Hex grid layout type */
    enum class HexLayout { PointyTop, FlatTop };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalTransforms = 0;
        int gridSize = 0;
        int hexSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit HexFFT4(QObject *parent = nullptr);
    ~HexFFT4() override;

    void setHexLayout(HexLayout layout);
    void setHexSize(int size);

    /** @brief Forward hex FFT transform */
    void transform(const QVector<QVector<double>>& hexData,
                   QVector<QVector<double>>& outRe,
                   QVector<QVector<double>>& outIm);

    /** @brief Inverse hex FFT transform */
    void inverseTransform(const QVector<QVector<double>>& inRe,
                          const QVector<QVector<double>>& inIm,
                          QVector<QVector<double>>& hexData);

    /** @brief Convert hex grid to rectangular grid */
    QVector<QVector<double>> hexToRect(
        const QVector<QVector<double>>& hexData) const;

    /** @brief Convert rectangular grid to hex grid */
    QVector<QVector<double>> rectToHex(
        const QVector<QVector<double>>& rectData) const;

    /** @brief Compute non-orthogonal basis twiddle factors */
    void computeHexTwiddles(int N, QVector<double>& cosT,
                            QVector<double>& sinT) const;

    /** @brief Hex grid axial distance */
    int hexDistance(int q1, int r1, int q2, int r2) const;

    /** @brief Generate hex grid neighbor offsets */
    QVector<QPair<int, int>> hexNeighbors() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int gridSize, int hexSize);

private:
    HexLayout m_layout = HexLayout::PointyTop;
    int m_hexSize = 3;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief 1D FFT (Cooley-Tukey radix-2) */
    void fft1D(QVector<double>& re, QVector<double>& im, bool inverse) const;

    /** @brief Apply non-orthogonal basis transformation */
    void basisTransform(QVector<double>& re, QVector<double>& im,
                        int N, bool inverse) const;

    /** @brief Cube coordinate rounding for hex */
    QPair<int, int> hexRound(double q, double r) const;

    /** @brief Next power of 2 */
    int nextPow2(int n) const;
};

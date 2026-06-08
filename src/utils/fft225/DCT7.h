/**
 * @file DCT7.h
 * @brief IV型DCT(正交因式分解+前后加法蝶形网络快速计算) — Type-IV DCT with Orthogonal Factorization and Fast Computation via Pre/Post-Addition Butterfly Network
 *
 * 功能: 实现Type-IV离散余弦变换(DCT-IV)，采用正交因式分解，
 *       通过前加法和后加法蝶形网络实现O(n log n)快速计算。
 *
 * 协作: BruunFFT7(Bruun FFT) / SplitRadixFFT6(分裂基) / MDCT5(MDCT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief IV型DCT(正交因式分解+蝶形网络)
 */
class DCT7 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int transformSize = 0;
        int numButterflies = 0;
        int numStages = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DCT7(QObject *parent = nullptr);
    ~DCT7() override;

    /** @brief Prepare transform for size N (must be power of 2) */
    bool prepare(int n);

    /** @brief Forward DCT-IV transform */
    QVector<double> forward(const QVector<double>& input) const;

    /** @brief Inverse DCT-IV (self-inverse: forward = inverse up to normalization) */
    QVector<double> inverse(const QVector<double>& input) const;

    /** @brief Get prepared size */
    int size() const { return m_n; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int size, double timeMs);

private:
    int m_n = 0;
    int m_stages = 0;

    // Precomputed twiddle factors for orthogonal factorization
    QVector<double> m_twiddleCos;
    QVector<double> m_twiddleSin;

    // Pre-addition butterfly indices
    QVector<QVector<int>> m_preAddIndices;
    // Post-addition butterfly indices
    QVector<QVector<int>> m_postAddIndices;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build pre-addition butterfly network */
    void buildPreAddButterfly();

    /** @brief Build post-addition butterfly network */
    void buildPostAddButterfly();

    /** @brief Apply pre-addition butterfly stage */
    void applyPreAdd(QVector<double>& data) const;

    /** @brief Apply post-addition butterfly stage */
    void applyPostAdd(QVector<double>& data) const;

    /** @brief Apply twiddle factor multiplication */
    void applyTwiddles(QVector<double>& data) const;
};

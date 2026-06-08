/**
 * @file PrimeFactorFFT7.h
 * @brief 素因子FFT(无旋转因子索引+Winograd嵌套小N DFT模块) — Prime Factor FFT with Rotator-Free Indexing and Winograd Nested Small-N DFT Modules
 *
 * 功能: 实现素因子FFT算法，利用互素因子分解消除旋转因子，
 *       集成Winograd嵌套小N DFT模块提升计算效率。
 *
 * 协作: RaderFFT7(Rader FFT) / SplitRadixFFT6(分裂基) / Goertzel7(Goertzel)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 素因子FFT(无旋转因子+Winograd小N)
 */
class PrimeFactorFFT7 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int transformSize = 0;
        int numFactors = 0;
        bool isRotatorFree = false;
        double avgProcessingTimeMs = 0.0;
    };

    explicit PrimeFactorFFT7(QObject *parent = nullptr);
    ~PrimeFactorFFT7() override;

    /** @brief Prepare transform for size N (must factor into pairwise coprime) */
    bool prepare(int n);

    /** @brief Forward FFT: complex interleaved [re0,im0,re1,im1,...] */
    QVector<double> forward(const QVector<double>& input) const;

    /** @brief Inverse FFT */
    QVector<double> inverse(const QVector<double>& input) const;

    /** @brief Factorize n into pairwise coprime factors */
    static QVector<int> coprimeFactors(int n);

    /** @brief Get prepared size */
    int size() const { return m_n; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int size, double timeMs);

private:
    int m_n = 0;
    QVector<int> m_factors;     // pairwise coprime factors
    QVector<int> m_ni;          // individual factor sizes
    QVector<int> m_alpha;       // Ruritanian mapping alpha
    QVector<int> m_beta;        // Ruritanian mapping beta

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Extended GCD for coprime index mapping */
    static int extGcd(int a, int b, int& x, int& y);

    /** @brief Build Ruritanian index mapping */
    void buildIndexMapping();

    /** @brief Winograd 2-point DFT */
    void winograd2(double& re0, double& im0, double& re1, double& im1) const;

    /** @brief Winograd 3-point DFT */
    void winograd3(double* re, double* im) const;

    /** @brief Winograd 5-point DFT */
    void winograd5(double* re, double* im) const;

    /** @brief General small-N DFT fallback */
    void smallDFT(double* re, double* im, int n) const;

    /** @brief Apply nested small-N DFT along one dimension */
    void applyNestedDFT(double* re, double* im, int dim,
                          int factorIdx, bool inverse) const;
};

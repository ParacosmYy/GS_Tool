/**
 * @file MixedRadixFFT12.h
 * @brief 混合基FFT(缓存友好分块蝶形与自动基选择实现内存层次化FFT计算) — Mixed-Radix FFT with Cache-Friendly Tiled Butterfly and Automatic Radix Selection for Memory-Hierarchical FFT Computation
 *
 * 功能: 实现混合基FFT(mixed-radix FFT)，采用缓存友好分块蝶形(cache-friendly tiled butterfly)
 *       与自动基选择(automatic radix selection)实现内存层次化FFT计算(memory-hierarchical FFT computation)。
 *
 * 协作: NumberTheoreticTransform9(数论变换) / SplitRadixFFT11(分裂基FFT) / SlidingDFT12(滑动DFT)
 */
#pragma once

#include <QObject>
#include <QVector>

class MixedRadixFFT12 : public QObject {
    Q_OBJECT

public:
    /** @brief Complex number */
    struct Complex {
        double real = 0.0;
        double imag = 0.0;
        Complex operator+(const Complex& o) const { return {real + o.real, imag + o.imag}; }
        Complex operator-(const Complex& o) const { return {real - o.real, imag - o.imag}; }
        Complex operator*(const Complex& o) const { return {real*o.real - imag*o.imag, real*o.imag + imag*o.real}; }
    };

    /** @brief FFT plan with radix decomposition */
    struct Plan {
        QVector<int> radices;       // e.g. {4, 4, 2} for N=32
        int totalSize = 0;
        int tileSize = 64;          // L1 cache tile size
    };

    /** @brief Transform result */
    struct TransformResult {
        QVector<Complex> spectrum;
        int size = 0;
        double elapsedMs = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalTransforms = 0;
        int lastSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MixedRadixFFT12(QObject *parent = nullptr);
    ~MixedRadixFFT12() override;

    void setTileSize(int tileSize);

    /** @brief Create FFT plan for given size (factorizes N into radices) */
    Plan createPlan(int n) const;

    /** @brief Forward FFT with tiled butterflies */
    TransformResult forward(const QVector<Complex>& input);

    /** @brief Inverse FFT */
    TransformResult inverse(const QVector<Complex>& input);

    /** @brief Get twiddle factors for a given N */
    QVector<Complex> twiddleFactors(int n) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformDone(int size, double timeMs);

private:
    int m_tileSize = 64;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Factorize N into optimal radices (prefer 4, then 2, then 3, then 5) */
    QVector<int> factorize(int n) const;

    /** @brief Select best radix for remaining size */
    int selectRadix(int remaining) const;

    /** @brief Compute DFT of small prime size (2,3,4,5,7) */
    void smallDFT(Complex* data, int radix, bool inverse) const;

    /** @brief Execute tiled butterfly for one radix stage */
    void tiledButterfly(QVector<Complex>& data, int radix, int stageLen,
                        int stride, const Complex* twiddles, bool inverse) const;

    /** @brief Digit-reverse permutation for mixed radix */
    void digitReverse(QVector<Complex>& data, const QVector<int>& radices) const;
};

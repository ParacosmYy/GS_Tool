/**
 * @file FilterDesign8.h
 * @brief 滤波器设计(Parks-McClellan等波纹最优FIR Remez交换算法任意规格) — Filter Design with Parks-McClellan Equiripple Optimal FIR and Remez Exchange Algorithm for Arbitrary Specifications
 *
 * 功能: 实现滤波器设计(Filter design)，采用Parks-McClellan等波纹最优FIR
 *       (Parks-McClellan equiripple optimal FIR)和Remez交换算法(Remez exchange
 *       algorithm)设计任意规格(arbitrary specifications)的最优FIR滤波器。
 *
 * 协作: WindowFunction6(窗函数) / IIRFilter7(IIR滤波器) / BiquadFilter7(双二阶滤波器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 滤波器设计(Parks-McClellan等波纹最优FIR Remez交换算法任意规格)
 */
class FilterDesign8 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int filterOrder = 0;
        int numBands = 0;
        int numIterations = 0;
        double maxError = 0.0;
        bool converged = false;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Band specification */
    struct BandSpec {
        double lowFreq = 0.0;
        double highFreq = 0.0;
        double desiredGain = 1.0;
        double weight = 1.0;
    };

    explicit FilterDesign8(QObject *parent = nullptr);
    ~FilterDesign8() override;

    /** @brief Design equiripple FIR filter using Parks-McClellan / Remez */
    QVector<double> designEquiripple(int order, const QVector<BandSpec>& bands);

    /** @brief Compute frequency response of filter coefficients */
    QVector<double> frequencyResponse(const QVector<double>& coeffs, int numPoints) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void filterDesigned(int order, int numBands, double maxError, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute D(w) = desired response at normalized frequency w */
    double desiredResponse(double w, const QVector<BandSpec>& bands) const;

    /** @brief Compute W(w) = weight function at normalized frequency w */
    double weightFunction(double w, const QVector<BandSpec>& bands) const;

    /** @brief Remez exchange algorithm core */
    QVector<double> remezExchange(int order, const QVector<BandSpec>& bands,
                                  int maxIter = 40);

    /** @brief Dense grid of frequency points across all bands */
    QVector<double> buildDenseGrid(const QVector<BandSpec>& bands, int gridSize) const;

    /** @brief Lagrange interpolation on extremal set */
    double lagrangeInterp(double w, const QVector<double>& extremalFreqs,
                          const QVector<double>& extremalVals) const;
};

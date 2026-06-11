/**
 * @file EnvelopeDetector11.h
 * @brief 包络检测器(Hilbert-Huang经验模态分解与瞬时能量跟踪实现非平稳信号分析) — Envelope Detector with Hilbert-Huang Empirical Mode Decomposition and Instantaneous Energy Tracking for Non-stationary Signal Analysis
 *
 * 功能: 实现包络检测器(envelope detector)，采用Hilbert-Huang经验模态分解(Hilbert-Huang EMD)
 *       与瞬时能量跟踪(instantaneous energy tracking)实现非平稳信号分析(non-stationary signal analysis)。
 *
 * 协作: Limiter14(限制器) / HilbertTransform(希尔伯特变换) / Spectrogram(频谱图)
 */
#pragma once

#include <QObject>
#include <QVector>

class EnvelopeDetector11 : public QObject {
    Q_OBJECT

public:
    /** @brief Single IMF (Intrinsic Mode Function) */
    struct IMF {
        QVector<double> signal;
        QVector<double> instantaneousFreq;
        QVector<double> instantaneousAmp;
        double meanFreq = 0.0;
        double energy = 0.0;
    };

    /** @brief Full EMD decomposition result */
    struct EMDResult {
        QVector<IMF> imfs;
        QVector<double> residual;
        int numIMFs = 0;
    };

    /** @brief Envelope detection result */
    struct EnvelopeResult {
        QVector<double> upperEnvelope;
        QVector<double> lowerEnvelope;
        QVector<double> instantaneousEnergy;
        EMDResult emdResult;
        double peakEnvelope = 0.0;
        double rmsEnvelope = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalDetections = 0;
        int numIMFs = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit EnvelopeDetector11(QObject *parent = nullptr);
    ~EnvelopeDetector11() override;

    void setMaxIMFs(int maxIMFs);
    void setMaxSiftIterations(int maxIter);
    void setSiftTolerance(double tol);

    /** @brief Compute envelope via Hilbert-Huang EMD */
    EnvelopeResult detect(const QVector<double>& input);

    /** @brief Extract just the EMD decomposition */
    EMDResult decompose(const QVector<double>& input) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void detectionDone(int n, int numIMFs, double timeMs);

private:
    int m_maxIMFs = 10;
    int m_maxSiftIter = 100;
    double m_siftTol = 0.05;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Find local maxima indices */
    QVector<int> findMaxima(const QVector<double>& x) const;

    /** @brief Find local minima indices */
    QVector<int> findMinima(const QVector<double>& x) const;

    /** @brief Cubic spline interpolation at given x-positions */
    QVector<double> splineInterpolate(const QVector<double>& xData,
                                       const QVector<double>& yData,
                                       const QVector<double>& xEval) const;

    /** @brief Extract one IMF via sifting */
    QVector<double> sift(const QVector<double>& signal) const;

    /** @brief Compute instantaneous amplitude and frequency via analytic signal */
    void computeInstantaneous(IMF& imf) const;
};

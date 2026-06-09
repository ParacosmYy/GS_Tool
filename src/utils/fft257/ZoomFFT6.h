/**
 * @file ZoomFFT6.h
 * @brief 缩放FFT(复数频移抽取+多相分析滤波窄带高分辨率分析) — Zoom FFT with Complex Band-Shift Decimation and Polyphase Analysis Filter for High-Resolution Narrowband Analysis
 *
 * 功能: 实现缩放FFT(Zoom FFT)，通过复数频移抽取(complex band-shift
 *       decimation)和多相分析滤波器(polyphase analysis filter)实现
 *       窄带信号的高分辨率频谱分析(high-resolution narrowband analysis)。
 *
 * 协作: DistributedArithmetic9(分布式算术) / PolyphaseFilter7(多相滤波) / FFT4(FFT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 缩放FFT(复数频移抽取+多相分析滤波)
 */
class ZoomFFT6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int fftSize = 0;
        int decimationFactor = 0;
        int zoomBands = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ZoomFFT6(QObject *parent = nullptr);
    ~ZoomFFT6() override;

    /** @brief Set center frequency for zoom band (normalized 0..1) */
    void setCenterFrequency(double fc);

    /** @brief Set zoom bandwidth (normalized 0..1) */
    void setBandwidth(double bw);

    /** @brief Set FFT size for final analysis */
    void setFFTSize(int size);

    /** @brief Set decimation factor */
    void setDecimationFactor(int d);

    /** @brief Set number of polyphase branches */
    void setPolyphaseBranches(int branches);

    /** @brief Process input and return zoomed spectrum */
    QVector<double> process(const QVector<double>& input);

    /** @brief Get frequency axis for zoomed output */
    QVector<double> frequencyAxis() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void analysisCompleted(int bins, double centerFreq, double timeMs);

private:
    double m_centerFreq = 0.25;
    double m_bandwidth = 0.1;
    int m_fftSize = 512;
    int m_decimFactor = 4;
    int m_polyBranches = 4;

    QVector<double> m_polyCoeffs;    // Polyphase filter coefficients
    QVector<QVector<double>> m_polyState;  // Branch delay lines

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Design polyphase lowpass filter */
    void designPolyphaseFilter();

    /** @brief Complex frequency shift (heterodyne) */
    void frequencyShift(QVector<double>& real, QVector<double>& imag) const;

    /** @brief Polyphase decimation filter */
    QVector<double> polyphaseDecimate(const QVector<double>& real,
                                       const QVector<double>& imag) const;

    /** @brief Compute FFT magnitude from complex samples */
    QVector<double> computeFFT(const QVector<double>& samples) const;

    /** @brief Bit-reversal permutation index */
    static int bitReverse(int x, int bits);

    /** @brief Twiddle factor */
    static void twiddle(int k, int n, double& wr, double& wi);
};

/**
 * @file AdaptiveFilter6.h
 * @brief 自适应滤波器(频域LMS+子带分解长回声路径消除) — Adaptive Filter with Frequency-domain LMS and Subband Decomposition for Long Echo Path Cancellation
 *
 * 功能: 实现自适应滤波器，采用频域LMS(FLMS)算法并结合子带分解(subband)，
 *       降低计算复杂度并提升长回声路径(long echo path)消除的收敛性能。
 *
 * 协作: Compressor5(压缩器) / Equalizer3(均衡器) / WienerFilter4(Wiener滤波器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 自适应滤波器(频域LMS+子带分解)
 */
class AdaptiveFilter6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int filterLength = 0;
        int numSubbands = 0;
        int framesProcessed = 0;
        double misalignment = 0.0;
        double echoReturnLoss = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit AdaptiveFilter6(QObject *parent = nullptr);
    ~AdaptiveFilter6() override;

    /** @brief Configure filter length, subbands, and step size */
    bool configure(int filterLength, int numSubbands, double stepSize);

    /** @brief Process one frame: input + reference (echo) -> output (cleaned) */
    QVector<double> process(const QVector<double>& input,
                             const QVector<double>& reference);

    /** @brief Reset filter coefficients and subband state */
    void reset();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void frameProcessed(int frames, double erl, double timeMs);

private:
    int m_filterLen = 256;
    int m_numBands = 4;
    int m_blockSize = 64;
    int m_fftSize = 128;
    double m_stepSize = 0.3;

    // Frequency-domain weights per subband
    QVector<QVector<double>> m_weightsRe;
    QVector<QVector<double>> m_weightsIm;

    // Circular buffers per subband
    QVector<QVector<double>> m_inputBuf;
    QVector<QVector<double>> m_refBuf;
    QVector<int> m_bufPos;

    // Analysis/synthesis prototype filter
    QVector<double> m_protoFilter;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Analysis filterbank: split signal into subbands */
    QVector<QVector<double>> analysisFilterbank(
        const QVector<double>& signal) const;

    /** @brief Synthesis filterbank: merge subbands back */
    QVector<double> synthesisFilterbank(
        const QVector<QVector<double>>& subbands) const;

    /** @brief Frequency-domain LMS update for one subband */
    void flmsUpdate(int band, const QVector<double>& input,
                    const QVector<double>& ref, double& error);

    /** @brief In-place DFT (small size) */
    void computeDFT(const QVector<double>& in,
                    QVector<double>& re, QVector<double>& im) const;

    /** @brief In-place IDFT */
    void computeIDFT(QVector<double>& re, QVector<double>& im,
                     QVector<double>& out) const;
};


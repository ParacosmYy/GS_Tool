/**
 * @file ChirpZ5.h
 * @brief 线性调频Z变换(自动分辨率选择+频率导数分析) — Chirp-Z Transform with Automatic Resolution Selection and Frequency-Derivative Analysis
 *
 * 功能: 实现Chirp-Z变换(CZT)，支持自动分辨率选择、
 *       任意频率范围分析和频率导数(瞬时频率变化率)计算。
 *
 * 协作: Goertzel5(Goertzel) / DHT4(哈特利) / FftEngine3(FFT引擎)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 线性调频Z变换(自动分辨率+频率导数)
 */
class ChirpZ5 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalTransforms = 0;
        int lastInputSize = 0;
        int lastOutputSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ChirpZ5(QObject *parent = nullptr);
    ~ChirpZ5() override;

    void setFreqRange(double fmin, double fmax);
    void setOutputSize(int m);

    /** @brief Compute CZT over configured frequency range */
    QVector<QVector<double>> transform(const QVector<double>& input);

    /** @brief Compute frequency derivative (instantaneous rate of change) */
    QVector<double> frequencyDerivative(const QVector<QVector<double>>& spectrum) const;

    /** @brief Auto-select output size based on input length and range */
    int autoResolution(int inputLen) const;

    /** @brief Get frequency bins of last transform */
    QVector<double> frequencyBins() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int inputSize, int outputSize, double timeMs);

private:
    double m_fmin = 0.0;
    double m_fmax = 0.5;
    int m_outputSize = 0;  // 0 = auto

    QVector<double> m_freqBins;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Next power of 2 >= n */
    static int nextPow2(int n);

    /** @brief Bluestein chirp convolution via FFT */
    QVector<QVector<double>> bluesteinCZT(
        const QVector<double>& x, int M,
        const QVector<QVector<double>>& wk) const;

    /** @brief Generate chirp twiddle factors A^k, W^k */
    void generateTwiddles(int N, int M,
                          QVector<QVector<double>>& wk,
                          QVector<QVector<double>>& aExp) const;
};

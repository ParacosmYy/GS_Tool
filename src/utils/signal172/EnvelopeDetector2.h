/**
 * @file EnvelopeDetector2.h
 * @brief 包络检测(解析信号Hilbert变换+峰值追踪) — Envelope Detection via Analytic Signal (Hilbert Transform) and Peak Tracking
 *
 * 功能: 实现信号包络检测，支持Hilbert变换解析信号法、峰值追踪、
 *       可配置攻击/释放时间常数和多通道处理，适用于嵌入式调制解调。
 *
 * 协作: FftEngine(FFT) / HilbertFilter(Hilbert) / AmplitudeDemod7(幅度解调)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 包络检测器
 */
class EnvelopeDetector2 : public QObject {
    Q_OBJECT

public:
    /** @brief 检测方法 */
    enum Method { HilbertTransform, PeakTracking };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSamples = 0;        ///< 累计处理样本数
        quint64 totalFrames = 0;         ///< 累计处理帧数
        double avgProcessingTimeMs = 0.0;///< 平均耗时(ms)
        double lastPeakAmplitude = 0.0;  ///< 最近峰值幅度
    };

    explicit EnvelopeDetector2(QObject *parent = nullptr);
    ~EnvelopeDetector2() override;

    void setMethod(Method method);
    void setAttackTime(double ms);
    void setReleaseTime(double ms);
    void setSampleRate(double sr);

    /**
     * @brief 检测信号包络(Hilbert方法)
     * @param signal 输入信号
     * @return 包络幅度
     */
    QVector<double> detectEnvelope(const QVector<double>& signal);

    /**
     * @brief 峰值追踪法检测包络
     * @param signal 输入信号
     * @return 包络幅度
     */
    QVector<double> detectPeakTrack(const QVector<double>& signal);

    /** @brief 获取解析信号的瞬时相位 */
    QVector<double> instantaneousPhase(const QVector<double>& signal);

    /** @brief 获取解析信号的瞬时频率 */
    QVector<double> instantaneousFrequency(const QVector<double>& signal);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 处理完成 @param samples 样本数 */
    void processingCompleted(quint64 samples);

private:
    /** @brief 基2 FFT(就地) */
    void fft(QVector<double>& real, QVector<double>& imag, bool inverse) const;

    /** @brief Hilbert变换(FFT法) */
    QVector<double> hilbertTransform(const QVector<double>& signal) const;

    /** @brief 找到>=n的最小2的幂 */
    static int nextPow2(int n);

    Method m_method = HilbertTransform;
    double m_attackMs = 5.0;
    double m_releaseMs = 50.0;
    double m_sampleRate = 44100.0;

    double m_attackCoeff = 0.0;
    double m_releaseCoeff = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;
};

/**
 * @file SpectralGate.h
 * @brief 频谱门限(噪声底估计+可配置阈值曲线) — Spectral Gate with Noise Floor Estimation and Configurable Threshold Curve
 *
 * 功能: 实现频谱门限处理，支持噪声底自适应估计、可配置阈值曲线、
 *       FFT帧处理和时域信号重建，适用于嵌入式音频降噪。
 *
 * 协作: FftEngine(FFT) / WienerFilter(维纳滤波) / NoiseProfiler(噪声轮廓)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 频谱门限处理器
 */
class SpectralGate : public QObject {
    Q_OBJECT

public:
    /** @brief 阈值曲线类型 */
    enum ThresholdCurve { Flat, Linear, Logarithmic, Custom };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalFrames = 0;          ///< 累计处理帧数
        double avgProcessingTimeMs = 0.0; ///< 平均耗时(ms)
        double lastNoiseFloor = 0.0;      ///< 最近噪声底(dB)
        double avgReduction = 0.0;        ///< 平均衰减量(dB)
    };

    explicit SpectralGate(int fftSize = 1024, QObject *parent = nullptr);
    ~SpectralGate() override;

    void setThreshold(double threshDb);
    void setRatio(double ratio);
    void setAttack(double ms);
    void setRelease(double ms);
    void setThresholdCurve(ThresholdCurve curve);
    void setCustomCurve(const QVector<double>& curveDb);

    /** @brief 用前几帧估计噪声底 */
    void estimateNoiseFloor(const QVector<QVector<double>>& noiseFrames);

    /**
     * @brief 处理一帧频谱数据(幅度)
     * @param magnitude FFT幅度谱
     * @return 门限后的幅度谱
     */
    QVector<double> process(const QVector<double>& magnitude);

    /** @brief 设置噪声底轮廓 */
    void setNoiseFloor(const QVector<double>& floorDb);

    /** @brief 获取当前噪声底 */
    QVector<double> noiseFloor() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 帧处理完成 @param frameIdx 帧序号 */
    void frameProcessed(quint64 frameIdx);

private:
    /** @brief 计算当前bin的阈值 */
    double binThreshold(int bin) const;

    /** @brief dB转换 */
    static double toDb(double linear);
    static double fromDb(double db);

    int m_fftSize;
    double m_thresholdDb = -40.0;
    double m_ratio = 6.0;
    double m_attackMs = 5.0;
    double m_releaseMs = 50.0;
    ThresholdCurve m_curveType = Flat;

    QVector<double> m_noiseFloorDb;     ///< 噪声底(dB)
    QVector<double> m_customCurveDb;    ///< 自定义阈值曲线
    QVector<double> m_gainState;        ///< 平滑增益状态
    double m_attackCoeff = 0.0;
    double m_releaseCoeff = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;
};

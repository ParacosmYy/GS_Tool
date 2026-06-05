#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief ZoomFFT7 - 缩放FFT第7代实现
 *
 * 提供窄带高分辨率频谱分析，支持复数调制、
 * 低通滤波抽取及指定频带细分辨能力。
 */
class ZoomFFT7 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalTransforms = 0; double avgProcessingTimeMs = 0.0; };
    explicit ZoomFFT7(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 执行缩放FFT变换
     * @param inputSignal 输入时域信号
     * @param centerFreqHz 中心频率 (Hz)
     * @param bandwidthHz 分析带宽 (Hz)
     * @return 细分辨后的频域复数序列
     */
    QVector<QPair<double, double>> compute(const QVector<double>& inputSignal,
                                           double centerFreqHz, double bandwidthHz);

    /**
     * @brief 设置抽取因子
     * @param decimationFactor 抽取倍数
     */
    void setDecimationFactor(int decimationFactor);

    /**
     * @brief 获取指定频段的幅度谱
     * @param zoomResult 缩放FFT结果
     * @return 频率-幅度对序列
     */
    QVector<QPair<double, double>> getMagnitudeSpectrum(
        const QVector<QPair<double, double>>& zoomResult);

signals:
    void zoomCompleted(int binCount);

private:
    Stats m_stats; double m_timeSum = 0.0;
};

#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief ChirpZTransform6 - Chirp-Z变换第6代实现
 *
 * 提供沿螺旋等高线采样的Z变换计算，用于任意频率轴上的
 * 频谱分析，支持比FFT更灵活的频点分布。
 */
class ChirpZTransform6 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalTransforms = 0; double avgProcessingTimeMs = 0.0; };
    explicit ChirpZTransform6(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 执行Chirp-Z变换
     * @param inputSignal 输入时域信号
     * @param startAngle 起始角度 (rad)
     * @param angleStep 角度步进 (rad)
     * @param radiusStep 径向步进
     * @param numPoints 输出点数
     * @return 变换结果（复数序列）
     */
    QVector<QPair<double, double>> compute(const QVector<double>& inputSignal,
                                           double startAngle, double angleStep,
                                           double radiusStep, int numPoints);

    /**
     * @brief 在指定频率范围内进行细分辨分析
     * @param inputSignal 输入信号
     * @param startFreqHz 起始频率 (Hz)
     * @param endFreqHz 终止频率 (Hz)
     * @param numPoints 输出点数
     * @param sampleRate 采样率 (Hz)
     * @return 频率-复数对序列
     */
    QVector<QPair<double, QPair<double, double>>> frequencyRangeAnalysis(
        const QVector<double>& inputSignal, double startFreqHz, double endFreqHz,
        int numPoints, double sampleRate);

    /**
     * @brief 设置是否使用Bluestein FFT加速
     * @param enabled 是否启用加速
     */
    void setBluesteinAcceleration(bool enabled);

signals:
    void transformCompleted(int pointCount);

private:
    Stats m_stats; double m_timeSum = 0.0;
};

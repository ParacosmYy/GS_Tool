#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief RecursiveDFT6 - 递归DFT第6代实现
 *
 * 提供递归形式的离散傅里叶变换，支持Goertzel类递归、
 * 滑动窗口DFT及任意长度DFT计算。
 */
class RecursiveDFT6 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalTransforms = 0; double avgProcessingTimeMs = 0.0; };
    explicit RecursiveDFT6(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 执行递归DFT变换
     * @param inputSignal 输入时域信号
     * @return 频域复数序列
     */
    QVector<QPair<double, double>> compute(const QVector<double>& inputSignal);

    /**
     * @brief 滑动窗口DFT，支持逐样本更新
     * @param newSample 新输入采样值
     * @return 更新后的频域复数序列
     */
    QVector<QPair<double, double>> slidingUpdate(double newSample);

    /**
     * @brief 初始化滑动窗口参数
     * @param fftSize FFT大小
     * @param sampleRate 采样率 (Hz)
     */
    void initSlidingWindow(int fftSize, double sampleRate);

    /**
     * @brief 设置递归深度限制（防止数值不稳定）
     * @param maxRecursionDepth 最大递归深度
     */
    void setMaxRecursionDepth(int maxRecursionDepth);

signals:
    void transformCompleted(int binCount);

private:
    Stats m_stats; double m_timeSum = 0.0;
};

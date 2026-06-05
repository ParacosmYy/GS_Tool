#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief PrunedFFT6 - 剪枝FFT第6代实现
 *
 * 当只需少量频点输出时，通过剪枝蝶形网络大幅降低计算量，
 * 支持指定频点集变换及逆变换。
 */
class PrunedFFT6 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalTransforms = 0; double avgProcessingTimeMs = 0.0; };
    explicit PrunedFFT6(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 执行剪枝FFT变换，仅输出指定频点
     * @param inputSignal 输入时域信号
     * @param outputBins 需要输出的频点索引集合
     * @return 指定频点的复数结果
     */
    QVector<QPair<double, double>> compute(const QVector<double>& inputSignal,
                                           const QVector<int>& outputBins);

    /**
     * @brief 设置FFT大小（必须为2的幂）
     * @param fftSize FFT点数
     */
    void setFFTSize(int fftSize);

    /**
     * @brief 估算剪枝后的计算节省率
     * @param totalBins 总频点数
     * @param outputBins 需要输出的频点数
     * @return 节省百分比 [0, 1]
     */
    double estimateSavings(int totalBins, int outputBins) const;

signals:
    void prunedCompleted(int binCount);

private:
    Stats m_stats; double m_timeSum = 0.0;
};

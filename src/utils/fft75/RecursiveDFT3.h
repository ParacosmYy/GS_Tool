#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief RecursiveDFT3 - 递归离散傅里叶变换
 *
 * 基于Goertzel算法的递归DFT实现，支持逐样本
 * 滑动更新频谱，适用于实时频谱分析场景。
 */
class RecursiveDFT3 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalTransforms = 0;
        int totalSamplesProcessed = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit RecursiveDFT3(QObject* parent = nullptr);

    /** @brief 初始化DFT长度和目标频率bin */
    bool initialize(int fftSize, const QVector<int>& targetBins);

    /** @brief 推入新样本，递归更新频谱 */
    void pushSample(double sample);

    /** @brief 获取当前频谱幅度 */
    QVector<double> magnitudeSpectrum() const;

    /** @brief 获取当前频谱相位 */
    QVector<double> phaseSpectrum() const;

    /** @brief 重置递归状态 */
    void reset();

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void spectrumUpdated(int binCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_fftSize = 0;
    QVector<int> m_targetBins;
    QVector<double> m_state;
};

#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief ConstantQ8 - 常数Q变换第8代实现
 *
 * 提供恒Q值频谱分析（CQT），频率轴为对数分布，
 * 广泛用于音乐信号分析、音高检测及和弦识别。
 */
class ConstantQ8 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalTransforms = 0; double avgProcessingTimeMs = 0.0; };
    explicit ConstantQ8(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 执行常数Q变换
     * @param signal 输入时域信号
     * @return CQT系数矩阵 (频段 × 时间帧)
     */
    QVector<QVector<double>> compute(const QVector<double>& signal);

    /**
     * @brief 设置CQT参数
     * @param minFreqHz 最低频率 (Hz)
     * @param maxFreqHz 最高频率 (Hz)
     * @param binsPerOctave 每倍频程的频点数
     * @param sampleRate 采样率 (Hz)
     */
    void setParameters(double minFreqHz, double maxFreqHz,
                       int binsPerOctave, double sampleRate);

    /**
     * @brief 获取各频段的中心频率
     * @return 中心频率列表 (Hz)
     */
    QVector<double> centerFrequencies() const;

    /**
     * @brief 计算CQT核矩阵（用于快速变换）
     * @return 是否计算成功
     */
    bool precomputeKernel();

signals:
    void transformCompleted(int binsCount);

private:
    Stats m_stats; double m_timeSum = 0.0;
    double m_minFreq = 32.7;
    double m_maxFreq = 4186.0;
    int m_binsPerOctave = 12;
    double m_sampleRate = 44100.0;
    QVector<double> m_centerFreqs;
    bool m_kernelComputed = false;
    QVector<QVector<QPair<double, double>>> m_kernel;
};

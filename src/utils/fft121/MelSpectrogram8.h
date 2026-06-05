#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief MelSpectrogram8 - Mel频谱图第8代实现
 *
 * 将线性频谱转换为Mel刻度频谱，支持可配置滤波器组、
 * 对数压缩及标准的语音/音乐分析参数预设。
 */
class MelSpectrogram8 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalTransforms = 0; double avgProcessingTimeMs = 0.0; };
    explicit MelSpectrogram8(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 从功率谱计算Mel频谱
     * @param powerSpectrum 输入功率谱
     * @return Mel频带能量序列
     */
    QVector<double> compute(const QVector<double>& powerSpectrum);

    /**
     * @brief 批量计算Mel频谱图
     * @param frames 分帧后的功率谱序列
     * @return Mel频谱图矩阵 (帧数 × Mel频带数)
     */
    QVector<QVector<double>> computeSequence(const QVector<QVector<double>>& frames);

    /**
     * @brief 设置Mel滤波器组参数
     * @param numMelBands Mel频带数量
     * @param fftSize FFT大小
     * @param sampleRate 采样率 (Hz)
     * @param lowFreqHz 最低频率 (Hz)
     * @param highFreqHz 最高频率 (Hz)
     */
    void setFilterBank(int numMelBands, int fftSize, double sampleRate,
                       double lowFreqHz = 0.0, double highFreqHz = 0.0);

    /**
     * @brief 频率值转Mel刻度
     * @param frequencyHz 频率 (Hz)
     * @return Mel值
     */
    double hzToMel(double frequencyHz) const;

    /**
     * @brief Mel刻度转频率值
     * @param mel Mel值
     * @return 频率 (Hz)
     */
    double melToHz(double mel) const;

signals:
    void transformCompleted(int frameCount);

private:
    Stats m_stats; double m_timeSum = 0.0;
};

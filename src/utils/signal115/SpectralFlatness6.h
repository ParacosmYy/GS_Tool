#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief SpectralFlatness6 - 频谱平坦度分析第6代实现
 *
 * 计算信号频谱平坦度指标（几何均值/算术均值），
 * 用于区分类噪声信号与类音调信号，支持分频段分析。
 */
class SpectralFlatness6 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalAnalysisOps = 0; double avgProcessingTimeMs = 0.0; };
    explicit SpectralFlatness6(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 计算全频段频谱平坦度
     * @param powerSpectrum 功率谱序列
     * @return 频谱平坦度值 [0, 1]，1表示白噪声
     */
    double compute(const QVector<double>& powerSpectrum);

    /**
     * @brief 计算分频段频谱平坦度
     * @param powerSpectrum 功率谱序列
     * @param numBands 频段数量
     * @return 各频段的平坦度值
     */
    QVector<double> computePerBand(const QVector<double>& powerSpectrum, int numBands);

    /**
     * @brief 设置分析窗口参数
     * @param fftSize FFT大小
     * @param sampleRate 采样率 (Hz)
     */
    void setWindowParameters(int fftSize, double sampleRate);

signals:
    void analysisCompleted(int frameCount);

private:
    Stats m_stats; double m_timeSum = 0.0;
};

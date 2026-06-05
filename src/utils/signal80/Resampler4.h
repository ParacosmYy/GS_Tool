#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Resampler4 - 多相重采样器
 *
 * 支持任意有理比率的采样率转换，使用多相FIR
 * 滤波器和多级级联实现高效高质量重采样。
 */
class Resampler4 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalSamplesProcessed = 0;
        int totalResamples = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Resampler4(QObject* parent = nullptr);

    /** @brief 设置重采样比率(L/M)和滤波器质量 */
    bool setRatio(int upFactor, int downFactor, int filterLength = 0);

    /** @brief 重采样输入信号 */
    QVector<double> process(const QVector<double>& input);

    /** @brief 获取输出采样率 */
    double outputRate(double inputRate) const;

    /** @brief 设置抗混叠滤波器截止频率 */
    void setCutoffFrequency(double normalizedFreq);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void resamplingCompleted(int inputSamples, int outputSamples);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_upFactor = 1;
    int m_downFactor = 1;
    double m_cutoff = 0.0;
};

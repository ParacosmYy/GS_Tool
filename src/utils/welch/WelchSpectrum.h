/**
 * @file WelchSpectrum.h
 * @brief Welch谱估计 — 平均周期图法功率谱估计
 *
 * 功能: Welch法功率谱密度估计，支持分段/窗函数/重叠配置，
 *       50%重叠+Hanning窗默认，统计估计次数/耗时。
 */
#ifndef WELCHSPECTRUM_H
#define WELCHSPECTRUM_H

#include <QObject>
#include <QVector>

class WelchSpectrum : public QObject {
    Q_OBJECT
public:
    /** 窗函数类型 */
    enum Window { Hanning, Hamming, Blackman, Rectangular };

    struct Stats {
        quint64 totalEstimates = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit WelchSpectrum(QObject* parent = nullptr);

    /** @brief 估计功率谱密度 @param signal 输入信号 @param sampleRate 采样率 @return {频率数组, PSD数组} */
    QPair<QVector<double>, QVector<double>> estimate(
        const QVector<double>& signal, double sampleRate);

    /** @brief 配置参数 @param segmentLength 分段长度 @param overlap 重叠比例[0,0.9] @param window 窗函数 */
    void configure(int segmentLength = 256, double overlap = 0.5,
                   Window window = Hanning);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void estimateCompleted(int freqBins, double maxPSD);

private:
    QVector<double> createWindow(int length, Window type) const;

    int m_segmentLength;
    double m_overlap;
    Window m_window;
    Stats m_stats;
    double m_timeSum;
};

#endif // WELCHSPECTRUM_H

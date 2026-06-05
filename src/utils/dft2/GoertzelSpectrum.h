/**
 * @file GoertzelSpectrum.h
 * @brief Goertzel频谱分析 — 多频率快速DFT
 *
 * 功能: 使用Goertzel算法高效计算指定频率点的DFT值。
 *       比FFT更高效当只需要少量频率点时。
 *
 * 协作: GoertzelAlgorithm(单频) / RealFFT(全频谱)
 */
#ifndef GOERTZELSPECTRUM_H
#define GOERTZELSPECTRUM_H

#include <QObject>
#include <QVector>

/**
 * @brief Goertzel多频率频谱分析
 */
class GoertzelSpectrum : public QObject {
    Q_OBJECT

public:
    /** @brief 频谱点结果 */
    struct FreqBin {
        double frequency = 0.0;     ///< 频率(Hz)
        double magnitude = 0.0;     ///< 幅度
        double phase = 0.0;         ///< 相位(弧度)
        double power = 0.0;         ///< 功率
    };

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalAnalyses = 0;      ///< 累计分析次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit GoertzelSpectrum(QObject* parent = nullptr);

    /** @brief 分析指定频率点
     *  @param signal 输入信号
     *  @param frequencies 目标频率列表(Hz)
     *  @param sampleRate 采样率
     *  @return 各频率点结果 */
    QVector<FreqBin> analyze(const QVector<double>& signal,
                             const QVector<double>& frequencies,
                             double sampleRate);

    /** @brief 分析单个频率
     *  @param signal 输入信号
     *  @param frequency 目标频率(Hz)
     *  @param sampleRate 采样率
     *  @return 频率点结果 */
    FreqBin analyzeSingle(const QVector<double>& signal,
                          double frequency, double sampleRate);

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 分析完成 @param freqCount 频率点数 */
    void analysisCompleted(int freqCount);

private:
    double m_timeSum;   ///< 处理时间累加器
    Stats m_stats;      ///< 统计信息
};

#endif // GOERTZELSPECTRUM_H

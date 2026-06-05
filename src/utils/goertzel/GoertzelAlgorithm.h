/**
 * @file GoertzelDetector.h
 * @brief Goertzel算法 — 单频点DFT快速计算
 *
 * 功能: Goertzel算法用于快速计算指定频率的DFT分量，
 *       无需完整FFT，适合DTMF检测等场景，统计计算次数/耗时。
 */
#ifndef GOERTZELALGORITHM_H
#define GOERTZELALGORITHM_H

#include <QObject>
#include <QVector>

class GoertzelDetector : public QObject {
    Q_OBJECT
public:
    struct Stats {
        quint64 totalComputations = 0;
        quint64 totalMultiFreq = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit GoertzelDetector(QObject* parent = nullptr);

    /** @brief 计算单频点幅度 @param signal 输入信号 @param targetFreq 目标频率 @param sampleRate 采样率 @return 幅度 */
    double computeMagnitude(const QVector<double>& signal,
                            double targetFreq, double sampleRate);

    /** @brief 计算单频点功率 @param signal 输入信号 @param targetFreq 目标频率 @param sampleRate 采样率 @return 功率 */
    double computePower(const QVector<double>& signal,
                        double targetFreq, double sampleRate);

    /** @brief 批量计算多频点 @param signal 输入信号 @param frequencies 频率数组 @param sampleRate 采样率 @return 幅度数组 */
    QVector<double> computeMultiFrequency(
        const QVector<double>& signal,
        const QVector<double>& frequencies,
        double sampleRate);

    /** @brief DTMF检测 @param signal 输入信号 @param sampleRate 采样率 @return 检测到的按键字符 */
    QChar detectDTMF(const QVector<double>& signal, double sampleRate);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void magnitudeComputed(double freq, double magnitude);
    void dtmfDetected(QChar key);

private:
    Stats m_stats;
    double m_timeSum;
};

#endif // GOERTZELALGORITHM_H

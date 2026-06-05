#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 恒Q变换(CQT)频谱分析
 *
 * 对数频率分辨率频谱分析，适用于音乐信号处理。
 */
class ConstantQ4 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalTransforms = 0;
        int totalBinsComputed = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ConstantQ4(QObject* parent = nullptr);

    /** @brief 执行恒Q变换 */
    QVector<double> transform(const QVector<double>& samples, double sampleRate,
                               double minFreq, double maxFreq, int binsPerOctave = 12);

    /** @brief 获取各频 bin 的中心频率 */
    QVector<double> binFrequencies() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int binCount, double freqRange);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    QVector<double> m_binFreqs;
};

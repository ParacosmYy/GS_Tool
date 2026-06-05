#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief Goertzel单频检测算法
 *
 * 高效检测特定频率分量的幅值和相位，适用于DTMF等场景。
 */
class GoertzelAlgorithm5 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalFrequenciesDetected = 0;
        int totalBlocksAnalyzed = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GoertzelAlgorithm5(QObject* parent = nullptr);

    /** @brief 检测指定频率的幅值 */
    double detectMagnitude(const QVector<double>& samples, double targetFreqHz, double sampleRate);

    /** @brief 批量检测多个频率 */
    QVector<QPair<double, double>> detectMulti(const QVector<double>& samples,
                                                const QVector<double>& freqsHz,
                                                double sampleRate);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void frequencyDetected(double freqHz, double magnitude);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
};

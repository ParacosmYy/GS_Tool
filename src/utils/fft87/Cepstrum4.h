#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 倒谱(Cepstrum)分析器
 *
 * 计算实倒谱和复倒谱，用于基频检测和声道特征提取。
 */
class Cepstrum4 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalCepstraComputed = 0;
        int totalPeaksFound = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Cepstrum4(QObject* parent = nullptr);

    /** @brief 计算实倒谱 */
    QVector<double> realCepstrum(const QVector<double>& signal);

    /** @brief 计算复倒谱 */
    QVector<double> complexCepstrum(const QVector<double>& signal);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void cepstrumComputed(int quefrencyLength, double peakQuefrency);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
};

/**
 * @file CepstrumAnalyzer.h
 * @brief 倒谱分析器 — 倒频率/共振峰/基频提取
 *
 * 功能: 计算实倒谱/复倒谱，用于共振峰提取/基频检测，
 *       统计分析次数/帧数/耗时。
 */
#ifndef CEPSTRUMANALYZER_H
#define CEPSTRUMANALYZER_H

#include <QObject>
#include <QVector>

class CepstrumAnalyzer : public QObject {
    Q_OBJECT
public:
    struct Stats {
        quint64 totalAnalyses = 0;
        quint64 totalFramesProcessed = 0;
        double  averageProcessingTimeMs = 0.0;
    };

    explicit CepstrumAnalyzer(QObject* parent = nullptr);

    /** @brief 实倒谱 @param data 信号 @return 倒谱 */
    QVector<double> realCepstrum(const QVector<double>& data);

    /** @brief 复倒谱 @param data 信号 @return 复倒谱 */
    QVector<double> complexCepstrum(const QVector<double>& data);

    /** @brief 基频检测(倒谱法) @param data 信号 @param sampleRate 采样率 @param minFreq 最小频率 @param maxFreq 最大频率 @return 基频(Hz) */
    double detectPitch(const QVector<double>& data, double sampleRate,
                       double minFreq = 50.0, double maxFreq = 500.0);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void analysisCompleted(int frameSize, double pitchHz);

private:
    void computeDft(const QVector<double>& in, QVector<double>& mag,
                    QVector<double>& phase) const;
    void computeIdft(const QVector<double>& logMag, QVector<double>& out) const;

    Stats m_stats;
    double m_timeSum;
};

#endif // CEPSTRUMANALYZER_H

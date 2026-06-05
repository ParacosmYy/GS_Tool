/**
 * @file CepstralAnalysis.h
 * @brief 倒谱分析 — 基频和共振峰提取
 */
#ifndef CEPSTRALANALYSIS_H
#define CEPSTRALANALYSIS_H

#include <QObject>
#include <QVector>

class CepstralAnalysis : public QObject {
    Q_OBJECT
public:
    struct Stats {
        quint64 totalAnalyses = 0;
        double avgProcessingTimeMs = 0.0;
    };
    explicit CepstralAnalysis(QObject* parent = nullptr);
    QVector<double> compute(const QVector<double>& signal);
    double getPitch(const QVector<double>& cepstrum, double sampleRate) const;
    QVector<double> getFormants(const QVector<double>& cepstrum, int nFormants) const;
    const Stats& stats() const { return m_stats; }
    void resetStatistics();
signals:
    void analysisCompleted(double pitch);
private:
    double m_timeSum;
    Stats m_stats;
};
#endif

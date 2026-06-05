#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class TransientShaper2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalProcessings = 0; int totalSamples = 0; double avgProcessingTimeMs = 0.0; };
    explicit TransientShaper2(QObject* parent = nullptr);
    void setAttackSensitivity(double sens);
    void setSustainSensitivity(double sens);
    void setAmount(double amt);
    QVector<double> process(const QVector<double>& input);
    QVector<double> envelope() const { return m_env; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void processingCompleted(int samples, double peakTransient);
private:
    double m_attackSens = 1.0; double m_sustainSens = 0.5; double m_amount = 1.0;
    QVector<double> m_env;
    QVector<double> detectTransient(const QVector<double>& sig);
    Stats m_stats; double m_timeSum = 0.0;
};

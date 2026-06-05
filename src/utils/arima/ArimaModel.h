/**
 * @file ArimaModel.h
 * @brief ARIMA(p,d,q)时间序列模型
 */
#ifndef ARIMAMODEL_H
#define ARIMAMODEL_H

#include <QObject>
#include <QVector>

class ArimaModel : public QObject {
    Q_OBJECT
public:
    struct Stats {
        quint64 totalFits = 0;
        quint64 totalForecasts = 0;
        double avgProcessingTimeMs = 0.0;
    };
    explicit ArimaModel(QObject* parent = nullptr);
    double fit(const QVector<double>& series, int p, int d, int q);
    QVector<double> forecast(int steps);
    QVector<double> residuals() const { return m_residuals; }
    const Stats& stats() const { return m_stats; }
    void resetStatistics();
signals:
    void forecastCompleted(int horizon);
private:
    QVector<double> difference(const QVector<double>& s, int d) const;
    QVector<double> m_original, m_diffSeries, m_arCoeffs, m_maCoeffs, m_residuals;
    int m_p, m_d, m_q;
    double m_timeSum;
    Stats m_stats;
};
#endif

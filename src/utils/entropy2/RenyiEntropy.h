/**
 * @file RenyiEntropy.h
 * @brief Renyi/Tsallis广义熵
 */
#ifndef RENYIENTROPY_H
#define RENYIENTROPY_H

#include <QObject>
#include <QVector>

class RenyiEntropy : public QObject {
    Q_OBJECT
public:
    struct Stats {
        quint64 totalComputations = 0;
        double avgProcessingTimeMs = 0.0;
    };
    explicit RenyiEntropy(QObject* parent = nullptr);
    double renyi(const QVector<double>& probabilities, double alpha);
    double tsallis(const QVector<double>& probabilities, double alpha);
    double shannon(const QVector<double>& probabilities);
    double collision(const QVector<double>& probabilities);
    QVector<double> estimateProbabilities(const QVector<double>& data, int bins) const;
    const Stats& stats() const { return m_stats; }
    void resetStatistics();
signals:
    void computationCompleted(double entropy);
private:
    mutable double m_timeSum;
    mutable Stats m_stats;
};
#endif

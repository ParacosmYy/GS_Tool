/**
 * @file GaussianMixture7.h
 * @brief GMM7 - semi-supervised EM with label constraints
 */
#pragma once
#include <QObject>
#include <QVector>
#include <QPair>
class GaussianMixture7 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalFits = 0; int totalPointsProcessed = 0; double avgProcessingTimeMs = 0.0; };
    explicit GaussianMixture7(QObject* parent = nullptr);
    void setComponents(int k);
    void setMaxIterations(int maxIter);
    void setConstraints(const QVector<QPair<int,int>>& mustLink, const QVector<QPair<int,int>>& cannotLink);
    QVector<int> fit(const QVector<QVector<double>>& data);
    QVector<int> fitSemiSupervised(const QVector<QVector<double>>& data, const QVector<int>& partialLabels);
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void fitComplete(int components, double logLikelihood);
private:
    int m_k = 3; int m_maxIter = 200;
    QVector<QPair<int,int>> m_mustLink, m_cannotLink;
    Stats m_stats; double m_timeSum = 0.0;
};

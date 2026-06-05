/**
 * @file MeanShift4.h
 * @brief 均值漂移4 — 自适应带宽+均值漂移分割
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

class MeanShift4 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalClusterings = 0;
        int totalPointsProcessed = 0;
        int totalShiftIterations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MeanShift4(QObject* parent = nullptr);

    void setBandwidth(double bandwidth);
    void setKernel(const QString& kernel);
    void setMaxIterations(int maxIter);
    void setConvergenceThreshold(double tol);
    QVector<int> fit(const QVector<QVector<double>>& data);
    QVector<QVector<double>> modes() const { return m_modes; }

    int numClusters() const { return m_modes.size(); }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int points, int clusters);

private:
    double m_bandwidth = 1.0;
    QString m_kernel = "gaussian";
    int m_maxIterations = 300;
    double m_tol = 1e-4;
    QVector<QVector<double>> m_modes;

    double kernelWeight(const QVector<double>& x,
                         const QVector<double>& center) const;
    QVector<double> shiftPoint(const QVector<double>& point,
                                const QVector<QVector<double>>& data) const;

    Stats m_stats;
    double m_timeSum = 0.0;
};

/**
 * @file MeanShift3.h
 * @brief 均值漂移增强 — 多核/自适应带宽/层次模式/密度估计
 */
#pragma once
#include <QObject>
#include <QVector>
#include <QPair>
class MeanShift3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalShifts = 0; int totalPointsProcessed = 0; double avgProcessingTimeMs = 0.0; };
    explicit MeanShift3(QObject* parent = nullptr);
    void setBandwidth(double h);
    void setKernelType(int type);
    void setMaxIterations(int maxIter);
    void setConvergenceThreshold(double tol);
    QVector<int> fit(const QVector<QVector<double>>& data);
    QVector<QVector<double>> clusterCenters() const;
    int clusterCount() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void clusteringComplete(int numClusters);
private:
    QVector<double> shiftPoint(const QVector<double>& point, const QVector<QVector<double>>& data) const;
    double gaussianKernel(double dist) const;
    double adaptiveBandwidth(const QVector<double>& point, const QVector<QVector<double>>& data) const;
    double estimateDensity(const QVector<double>& point, const QVector<QVector<double>>& data) const;
    double m_bandwidth = 1.0; int m_kernelType = 0;
    int m_maxIter = 300; double m_tol = 1e-4;
    QVector<QVector<double>> m_centers; QVector<int> m_labels;
    Stats m_stats; double m_timeSum = 0.0;
};

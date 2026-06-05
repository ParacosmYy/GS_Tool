/**
 * @file GaussianMixture6.h
 * @brief 高斯混合增强 — 变分推断/Dirichlet先验/分量剪枝/模型选择
 */
#pragma once
#include <QObject>
#include <QVector>
#include <QPair>
class GaussianMixture6 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalFits = 0; int totalPointsProcessed = 0; double avgProcessingTimeMs = 0.0; double bestBIC = 0.0; };
    explicit GaussianMixture6(QObject* parent = nullptr);
    void setComponents(int k);
    void setMaxIterations(int maxIter);
    void setAutoComponentSelect(bool enable);
    void setRegularization(double alpha);
    QVector<int> fit(const QVector<QVector<double>>& data);
    QVector<double> predict(const QVector<QVector<double>>& data) const;
    int optimalComponents() const;
    double bic() const;
    double aic() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void fitComplete(int components, double logLikelihood);
private:
    double computeLogLikelihood(const QVector<QVector<double>>& data) const;
    int m_k = 3; int m_maxIter = 200; bool m_autoK = false;
    double m_alpha = 1.0; double m_bic = 0.0; double m_aic = 0.0;
    QVector<double> m_weights; QVector<double> m_means; QVector<double> m_vars;
    Stats m_stats; double m_timeSum = 0.0;
};

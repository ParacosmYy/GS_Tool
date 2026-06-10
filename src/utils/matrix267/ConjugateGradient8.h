/**
 * @file ConjugateGradient8.h
 * @brief 共轭梯度法(Fletcher-Reeves重启强Wolfe线搜索非二次优化) — Conjugate Gradient with Fletcher-Reeves Restart and Strong Wolfe Line Search for Non-quadratic Optimization
 *
 * 功能: 实现共轭梯度法(Conjugate Gradient)，采用Fletcher-Reeves重启策略
 *       (Fletcher-Reeves restart)和强Wolfe线搜索(strong Wolfe line search)
 *       求解非二次优化问题(non-quadratic optimization)。
 *
 * 协作: GradientDescent7(梯度下降) / NewtonMethod6(牛顿法) / LBFGS7(L-BFGS)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 共轭梯度法(Fletcher-Reeves重启强Wolfe线搜索非二次优化)
 */
class ConjugateGradient8 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numDimensions = 0;
        int numIterations = 0;
        double finalValue = 0.0;
        double finalGradientNorm = 0.0;
        bool converged = false;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Objective function type */
    using ObjectiveFunc = std::function<double(const QVector<double>&)>;

    /** @brief Gradient function type */
    using GradientFunc = std::function<QVector<double>(const QVector<double>&)>;

    explicit ConjugateGradient8(QObject *parent = nullptr);
    ~ConjugateGradient8() override;

    /** @brief Set objective and gradient functions */
    void setObjective(ObjectiveFunc obj, GradientFunc grad);

    /** @brief Set optimization parameters */
    void setParameters(int maxIterations, double tolerance, int restartPeriod = 10);

    /** @brief Minimize starting from initial point */
    QVector<double> minimize(const QVector<double>& x0);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void iterationCompleted(int iter, double value, double gradNorm, double timeMs);

private:
    ObjectiveFunc m_obj;
    GradientFunc m_grad;
    int m_maxIter = 1000;
    double m_tol = 1e-6;
    int m_restartPeriod = 10;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Strong Wolfe line search */
    double strongWolfeSearch(const QVector<double>& x, const QVector<double>& p,
                             double fx, const QVector<double>& gx) const;

    /** @brief Compute vector dot product */
    double dotProduct(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief Compute vector norm */
    double vecNorm(const QVector<double>& v) const;

    /** @brief Compute Fletcher-Reeves beta */
    double fletcherReevesBeta(const QVector<double>& gNew,
                              const QVector<double>& gOld) const;
};

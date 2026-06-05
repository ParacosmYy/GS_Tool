/**
 * @file BfgsOptimizer.cpp
 * @brief BFGS优化器实现
 */

#include "utils/bfgs/BfgsOptimizer.h"

#include <QElapsedTimer>
#include <QtMath>

/** @brief 构造函数 @param parent 父对象 */
BfgsOptimizer::BfgsOptimizer(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief BFGS优化 */
BfgsOptimizer::Result BfgsOptimizer::optimize(
    const ObjFunc& objective,
    const GradFunc& gradient,
    const QVector<double>& initialPoint,
    int maxIter, double tol)
{
    QElapsedTimer timer;
    timer.start();

    Result result;
    int n = initialPoint.size();
    if (n == 0) return result;

    QVector<double> x = initialPoint;

    /* 初始Hessian近似 = I */
    QVector<QVector<double>> H(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) H[i][i] = 1.0;

    /* 梯度函数(若为空则数值差分) */
    auto computeGrad = [&](const QVector<double>& p) -> QVector<double> {
        if (gradient) return gradient(p);
        return numericalGradient(objective, p);
    };

    QVector<double> grad = computeGrad(x);
    double fVal = objective(x);

    for (int iter = 0; iter < maxIter; ++iter) {
        /* 搜索方向: d = -H * grad */
        QVector<double> d(n, 0.0);
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                d[i] -= H[i][j] * grad[j];

        /* 梯度范数检查 */
        double gradNorm = 0.0;
        for (double g : grad) gradNorm += g * g;
        gradNorm = qSqrt(gradNorm);

        if (gradNorm < tol) {
            result.converged = true;
            result.optimalPoint = x;
            result.optimalValue = fVal;
            result.iterations = iter;
            goto done;
        }

        /* 线搜索 */
        double alpha = lineSearch(objective, x, d, grad);
        if (alpha < 1e-15) {
            result.optimalPoint = x;
            result.optimalValue = fVal;
            result.iterations = iter;
            break;
        }

        /* 更新 */
        QVector<double> xNew(n);
        for (int i = 0; i < n; ++i) xNew[i] = x[i] + alpha * d[i];

        QVector<double> gradNew = computeGrad(xNew);

        /* BFGS更新 */
        QVector<double> s(n);
        QVector<double> y(n);
        for (int i = 0; i < n; ++i) {
            s[i] = xNew[i] - x[i];
            y[i] = gradNew[i] - grad[i];
        }

        double sy = 0.0;
        for (int i = 0; i < n; ++i) sy += s[i] * y[i];

        if (sy > 1e-15) {
            QVector<double> Hs(n, 0.0);
            for (int i = 0; i < n; ++i)
                for (int j = 0; j < n; ++j)
                    Hs[i] += H[i][j] * s[j];

            double sHs = 0.0;
            for (int i = 0; i < n; ++i) sHs += s[i] * Hs[i];

            for (int i = 0; i < n; ++i) {
                for (int j = 0; j < n; ++j) {
                    H[i][j] += (y[i] * y[j]) / sy - (Hs[i] * Hs[j]) / sHs;
                }
            }
        }

        x = xNew;
        grad = gradNew;
        fVal = objective(x);

        if (iter % 10 == 0)
            emit iterationCompleted(iter, fVal);
    }

    result.optimalPoint = x;
    result.optimalValue = fVal;
    result.iterations = maxIter;

done:
    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    m_stats.totalIterations += static_cast<quint64>(result.iterations);
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalIterations);

    emit optimizationCompleted(result.optimalValue);
    return result;
}

/** @brief 数值梯度 */
QVector<double> BfgsOptimizer::numericalGradient(
    const ObjFunc& obj, const QVector<double>& x, double eps)
{
    int n = x.size();
    QVector<double> grad(n);
    double f0 = obj(x);

    for (int i = 0; i < n; ++i) {
        QVector<double> xp = x;
        xp[i] += eps;
        grad[i] = (obj(xp) - f0) / eps;
    }
    return grad;
}

/** @brief 回溯线搜索(Armijo条件) */
double BfgsOptimizer::lineSearch(const ObjFunc& obj,
                                  const QVector<double>& x,
                                  const QVector<double>& dir,
                                  const QVector<double>& grad)
{
    double alpha = 1.0;
    double c1 = 1e-4;
    double rho = 0.5;
    double f0 = obj(x);
    double dg0 = 0.0;
    int n = x.size();
    for (int i = 0; i < n; ++i) dg0 += grad[i] * dir[i];

    for (int iter = 0; iter < 30; ++iter) {
        QVector<double> xNew(n);
        for (int i = 0; i < n; ++i) xNew[i] = x[i] + alpha * dir[i];

        if (obj(xNew) <= f0 + c1 * alpha * dg0)
            return alpha;

        alpha *= rho;
    }
    return alpha;
}

/** @brief 重置统计 */
void BfgsOptimizer::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

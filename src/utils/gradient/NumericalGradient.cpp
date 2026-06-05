/**
 * @file NumericalGradient.cpp
 * @brief 数值梯度计算器实现 — 有限差分法
 */

#include "utils/gradient/NumericalGradient.h"

#include <QElapsedTimer>

/** @brief 构造函数 @param parent 父对象 */
NumericalGradient::NumericalGradient(QObject* parent)
    : QObject(parent)
    , m_stepSize(1e-6)
    , m_method(Method::Central)
    , m_timeSum(0.0) {}

/** @brief 设置步长 @param h 差分步长 */
void NumericalGradient::setStepSize(double h)
{
    m_stepSize = (h > 0) ? h : 1e-6;
}

/** @brief 设置差分方法 @param method 方法 */
void NumericalGradient::setMethod(Method method)
{
    m_method = method;
}

/** @brief 计算标量场梯度 @param func 标量函数 @param x 评估点 @return 梯度向量 */
QVector<double> NumericalGradient::gradient(
    const ScalarFunc& func, const QVector<double>& x)
{
    QElapsedTimer timer;
    timer.start();

    int n = x.size();
    QVector<double> grad(n, 0.0);

    for (int i = 0; i < n; ++i) {
        grad[i] = partialDerivative(func, x, i);
    }

    m_stats.totalGradients++;
    m_timeSum += timer.elapsed();
    quint64 total = m_stats.totalGradients + m_stats.totalJacobians;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit gradientComputed(n);
    return grad;
}

/** @brief 计算单变量偏导数 @param func 标量函数 @param x 评估点 @param dim 维度 @return 偏导数 */
double NumericalGradient::partialDerivative(
    const ScalarFunc& func, const QVector<double>& x, int dim)
{
    if (dim < 0 || dim >= x.size()) return 0.0;

    double h = m_stepSize;
    QVector<double> xp = x;
    QVector<double> xm = x;

    switch (m_method) {
    case Method::Forward:
        xp[dim] = x[dim] + h;
        m_stats.totalEvaluations += 2;
        return (func(xp) - func(x)) / h;

    case Method::Backward:
        xm[dim] = x[dim] - h;
        m_stats.totalEvaluations += 2;
        return (func(x) - func(xm)) / h;

    case Method::Central:
    default:
        xp[dim] = x[dim] + h;
        xm[dim] = x[dim] - h;
        m_stats.totalEvaluations += 2;
        return (func(xp) - func(xm)) / (2.0 * h);
    }
}

/** @brief 计算雅可比矩阵 @param func 向量函数 @param x 评估点 @return 雅可比矩阵 */
QVector<QVector<double>> NumericalGradient::jacobian(
    const VectorFunc& func, const QVector<double>& x)
{
    QElapsedTimer timer;
    timer.start();

    int n = x.size();
    QVector<double> f0 = func(x);
    int m = f0.size();

    /* 雅可比矩阵: m行n列 */
    QVector<QVector<double>> J(m, QVector<double>(n, 0.0));

    double h = m_stepSize;

    for (int j = 0; j < n; ++j) {
        QVector<double> xp = x;
        QVector<double> xm = x;

        switch (m_method) {
        case Method::Forward:
            xp[j] = x[j] + h;
            {
                QVector<double> fp = func(xp);
                for (int i = 0; i < m; ++i) {
                    J[i][j] = (fp[i] - f0[i]) / h;
                }
                m_stats.totalEvaluations += 1;
            }
            break;

        case Method::Backward:
            xm[j] = x[j] - h;
            {
                QVector<double> fm = func(xm);
                for (int i = 0; i < m; ++i) {
                    J[i][j] = (f0[i] - fm[i]) / h;
                }
                m_stats.totalEvaluations += 1;
            }
            break;

        case Method::Central:
        default:
            xp[j] = x[j] + h;
            xm[j] = x[j] - h;
            {
                QVector<double> fp = func(xp);
                QVector<double> fm = func(xm);
                for (int i = 0; i < m; ++i) {
                    J[i][j] = (fp[i] - fm[i]) / (2.0 * h);
                }
                m_stats.totalEvaluations += 2;
            }
            break;
        }
    }

    m_stats.totalJacobians++;
    m_timeSum += timer.elapsed();
    quint64 total = m_stats.totalGradients + m_stats.totalJacobians;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit jacobianComputed(m, n);
    return J;
}

/** @brief 计算Hessian矩阵 @param func 标量函数 @param x 评估点 @return Hessian矩阵 */
QVector<QVector<double>> NumericalGradient::hessian(
    const ScalarFunc& func, const QVector<double>& x)
{
    QElapsedTimer timer;
    timer.start();

    int n = x.size();
    QVector<QVector<double>> H(n, QVector<double>(n, 0.0));
    double h = m_stepSize;
    double h2 = h * h;

    double f0 = func(x);

    for (int i = 0; i < n; ++i) {
        /* 对角线: d^2f/dxi^2 */
        QVector<double> xpp = x; xpp[i] = x[i] + h;
        QVector<double> xmm = x; xmm[i] = x[i] - h;
        H[i][i] = (func(xpp) - 2.0 * f0 + func(xmm)) / h2;

        for (int j = i + 1; j < n; ++j) {
            /* 非对角线: d^2f/dxidxj */
            QVector<double> xpp2 = x; xpp2[i] = x[i] + h; xpp2[j] = x[j] + h;
            QVector<double> xmm2 = x; xmm2[i] = x[i] - h; xmm2[j] = x[j] - h;
            QVector<double> xpm = x; xpm[i] = x[i] + h; xpm[j] = x[j] - h;
            QVector<double> xmp = x; xmp[i] = x[i] - h; xmp[j] = x[j] + h;

            H[i][j] = (func(xpp2) - func(xpm) - func(xmp) + func(xmm2)) / (4.0 * h2);
            H[j][i] = H[i][j];
        }
    }

    m_stats.totalGradients++;
    m_timeSum += timer.elapsed();
    quint64 total = m_stats.totalGradients + m_stats.totalJacobians;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    return H;
}

/** @brief 重置统计 */
void NumericalGradient::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

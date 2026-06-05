/**
 * @file ToeplitzSolver.cpp
 * @brief Toeplitz 矩阵求解器实现 — Levinson-Durbin 递归
 */

#include "utils/matrix15/ToeplitzSolver.h"

#include <QElapsedTimer>
#include <QtMath>

/** @brief 构造函数 @param parent 父对象 */
ToeplitzSolver::ToeplitzSolver(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 求解 Toeplitz 方程组 T·x = b
 * @param firstRow Toeplitz 矩阵第一行 [t0, t1, ..., tn-1]
 * @param rhs 右端向量 b
 * @return 求解结果
 *
 * 使用 Levinson 递归求解一般 Toeplitz 方程组。
 * 要求 t0 ≠ 0，且矩阵非奇异。
 */
ToeplitzSolver::SolveResult ToeplitzSolver::solve(
    const QVector<double>& firstRow, const QVector<double>& rhs)
{
    QElapsedTimer timer;
    timer.start();

    SolveResult result;
    int n = firstRow.size();
    if (n < 1 || n != rhs.size() || qFuzzyIsNull(firstRow[0])) {
        ++m_stats.totalFailures;
        return result;
    }

    if (n == 1) {
        result.solution = {rhs[0] / firstRow[0]};
        result.success = true;
        double elapsed = static_cast<double>(timer.elapsed());
        ++m_stats.totalSolves;
        m_stats.totalDimensions += n;
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs = m_timeSum
            / static_cast<double>(m_stats.totalSolves);
        return result;
    }

    /* Levinson 递归: 同时维护前向和后向预测器 */
    QVector<double> forward(n, 0.0);   /* 前向预测系数 */
    QVector<double> backward(n, 0.0);  /* 后向预测系数 */
    double error = firstRow[0];         /* 预测误差 */

    forward[0] = 1.0 / firstRow[0];
    backward[0] = 1.0 / firstRow[0];

    for (int m = 1; m < n; ++m) {
        /* 计算前向/后向内积 */
        double fDot = 0.0, bDot = 0.0;
        for (int i = 0; i < m; ++i) {
            fDot += firstRow[m - i] * forward[i];
            bDot += firstRow[m - i] * backward[i];
        }

        double refCoeff = -fDot * error;
        double newError = error * (1.0 - refCoeff * refCoeff);
        if (qFuzzyIsNull(newError)) {
            ++m_stats.totalFailures;
            return result;
        }

        /* 更新前向和后向预测器 */
        QVector<double> newForward(m + 1), newBackward(m + 1);
        for (int i = 0; i < m; ++i) {
            newForward[i] = forward[i] + refCoeff * backward[m - 1 - i];
            newBackward[i + 1] = backward[i] + refCoeff * forward[m - 1 - i];
        }
        newForward[m] = refCoeff * backward[0];
        newBackward[0] = refCoeff * forward[0];

        /* 归一化 */
        for (int i = 0; i <= m; ++i) {
            newForward[i] /= newError;
            newBackward[i] /= newError;
        }

        forward = newForward;
        backward = newBackward;
        error = 1.0 / newError;  /* 存储 1/error 以便下轮使用 */
    }

    /* 使用前向预测器求解 T·x = b */
    QVector<double> x(n, 0.0);
    x[0] = rhs[0] * forward[0];
    for (int m = 1; m < n; ++m) {
        double residual = rhs[m];
        for (int i = 0; i < m; ++i) {
            residual -= firstRow[m - i] * x[i];
        }
        for (int i = 0; i <= m; ++i) {
            x[i] += residual * backward[m - i];
        }
    }

    result.solution = x;
    result.predictionError = 1.0 / error;
    result.success = true;

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalSolves;
    m_stats.totalDimensions += n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalSolves);

    emit solveCompleted(n, result.predictionError);
    return result;
}

/**
 * @brief 求解 Yule-Walker 方程
 * @param autocorrelation 自相关序列 [r0, r1, ..., rp]
 * @return AR 系数和预测误差
 *
 * Yule-Walker 方程是 Toeplitz 方程的特例:
 * r(0)   r(1)   ... r(p-1)   a1       r(1)
 * r(1)   r(0)   ... r(p-2)   a2   =   r(2)
 * ...                               ...
 * r(p-1) r(p-2) ... r(0)     ap       r(p)
 */
ToeplitzSolver::SolveResult ToeplitzSolver::solveYuleWalker(
    const QVector<double>& autocorrelation)
{
    QElapsedTimer timer;
    timer.start();

    SolveResult result;
    int p = autocorrelation.size() - 1; /* AR 阶数 */
    if (p < 1 || qFuzzyIsNull(autocorrelation[0])) {
        ++m_stats.totalFailures;
        return result;
    }

    /* Durbin 递归 */
    QVector<double> a(p + 1, 0.0);
    a[0] = 1.0;
    double error = autocorrelation[0];

    for (int m = 1; m <= p; ++m) {
        /* 反射系数 */
        double km = 0.0;
        for (int i = 0; i < m; ++i) {
            km += a[i] * autocorrelation[m - i];
        }
        km = -km / error;

        /* 更新 AR 系数 (需要旧值，所以用临时数组) */
        QVector<double> temp = a;
        for (int i = 1; i < m; ++i) {
            a[i] = temp[i] + km * temp[m - i];
        }
        a[m] = km;

        error *= (1.0 - km * km);
        if (error <= 0) {
            ++m_stats.totalFailures;
            return result;
        }
    }

    /* 提取 AR 系数 [a1, ..., ap] */
    result.solution.resize(p);
    for (int i = 0; i < p; ++i) {
        result.solution[i] = a[i + 1];
    }
    result.predictionError = error;
    result.reflectionCoeff = a[p];
    result.success = true;

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalSolves;
    m_stats.totalDimensions += p;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalSolves);

    emit solveCompleted(p, error);
    return result;
}

/**
 * @brief 计算反射系数
 * @param autocorrelation 自相关序列
 * @return 反射系数列表
 */
QVector<double> ToeplitzSolver::reflectionCoefficients(
    const QVector<double>& autocorrelation)
{
    int p = autocorrelation.size() - 1;
    if (p < 1) return {};

    QVector<double> a(p + 1, 0.0);
    a[0] = 1.0;
    double error = autocorrelation[0];
    QVector<double> reflect(p);

    for (int m = 1; m <= p; ++m) {
        double km = 0.0;
        for (int i = 0; i < m; ++i) {
            km += a[i] * autocorrelation[m - i];
        }
        km = -km / error;
        reflect[m - 1] = km;

        QVector<double> temp = a;
        for (int i = 1; i < m; ++i) {
            a[i] = temp[i] + km * temp[m - i];
        }
        a[m] = km;
        error *= (1.0 - km * km);
    }

    return reflect;
}

/** @brief 重置统计信息 */
void ToeplitzSolver::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

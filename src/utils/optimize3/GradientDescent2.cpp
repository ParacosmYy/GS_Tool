/**
 * @file GradientDescent2.cpp
 * @brief L-BFGS拟牛顿优化器实现 — 双循环递归 + Wolfe线搜索
 */

#include "utils/optimize3/GradientDescent2.h"

#include <QElapsedTimer>

#include <algorithm>
#include <cmath>
#include <numeric>

/** @brief 构造函数 @param memorySize 记忆大小 @param parent 父对象 */
GradientDescent2::GradientDescent2(int memorySize, QObject* parent)
    : QObject(parent)
    , m_memorySize(std::max(1, memorySize))
{
}

/** @brief 设置Wolfe参数 @param c1 充分下降 @param c2 曲率条件 */
void GradientDescent2::setWolfeParams(double c1, double c2)
{
    m_wolfeC1 = std::max(1e-8, c1);
    m_wolfeC2 = std::max(m_wolfeC1 + 1e-8, std::min(c2, 1.0));
}

/** @brief L-BFGS双循环递归 @param grad 梯度 @param sList 历史s @param yList 历史y @param rhoList 历史rho @return 搜索方向 */
std::vector<double> GradientDescent2::twoLoopRecursion(
    const std::vector<double>& grad,
    const QVector<std::vector<double>>& sList,
    const QVector<std::vector<double>>& yList,
    const QVector<double>& rhoList) const
{
    /* 双循环递归算法计算 H_k * grad */
    int n = static_cast<int>(grad.size());
    int m = sList.size();
    std::vector<double> q = grad;
    std::vector<double> alpha(m, 0.0);

    /* 第一个循环: 从最新到最旧 */
    for (int i = m - 1; i >= 0; --i) {
        double dotSY = 0.0;
        for (int j = 0; j < n; ++j) dotSY += sList[i][j] * q[j];
        alpha[i] = rhoList[i] * dotSY;
        for (int j = 0; j < n; ++j) q[j] -= alpha[i] * yList[i][j];
    }

    /* 初始Hessian近似: 使用最近一次s'y缩放单位矩阵 */
    if (m > 0) {
        double sy = 0.0, yy = 0.0;
        int last = m - 1;
        for (int j = 0; j < n; ++j) {
            sy += sList[last][j] * yList[last][j];
            yy += yList[last][j] * yList[last][j];
        }
        double gamma = (yy > 1e-20) ? (sy / yy) : 1.0;
        for (int j = 0; j < n; ++j) q[j] *= gamma;
    }

    /* 第二个循环: 从最旧到最新 */
    for (int i = 0; i < m; ++i) {
        double dotYQ = 0.0;
        for (int j = 0; j < n; ++j) dotYQ += yList[i][j] * q[j];
        double beta = rhoList[i] * dotYQ;
        for (int j = 0; j < n; ++j) q[j] += sList[i][j] * (alpha[i] - beta);
    }

    /* 取负方向(因为我们要最小化) */
    for (int j = 0; j < n; ++j) q[j] = -q[j];
    return q;
}

/** @brief Wolfe线搜索 @param func 目标函数 @param x 当前点 @param dir 搜索方向 @param grad 梯度 @param fVal 函数值 @return 步长 */
double GradientDescent2::wolfeLineSearch(
    const ObjectiveFunc& func,
    const std::vector<double>& x,
    const std::vector<double>& dir,
    const std::vector<double>& grad,
    double fVal)
{
    int n = static_cast<int>(x.size());
    double dg0 = 0.0;
    for (int j = 0; j < n; ++j) dg0 += grad[j] * dir[j];
    if (dg0 >= 0.0) return 0.0; /* 不是下降方向 */

    double alpha = m_maxStep;
    double alphaPrev = 0.0;
    double fPrev = fVal;
    int maxLS = 40;

    for (int ls = 0; ls < maxLS; ++ls) {
        /* 计算试探点 x + alpha * dir */
        std::vector<double> xNew(n);
        for (int j = 0; j < n; ++j) xNew[j] = x[j] + alpha * dir[j];

        std::vector<double> gradNew;
        double fNew = func(xNew, gradNew);

        /* Armijo条件(充分下降) */
        if (fNew > fVal + m_wolfeC1 * alpha * dg0) {
            /* 区间缩小(插值) */
            double denom = alpha * alpha - alphaPrev * alphaPrev;
            if (std::abs(denom) > 1e-30) {
                double alphaNew = -((alphaPrev * alphaPrev) * (fNew - fVal - dg0 * alpha)
                    - (alpha * alpha) * (fPrev - fVal - dg0 * alphaPrev)) / denom;
                alphaPrev = alpha;
                fPrev = fNew;
                alpha = std::max(1e-10, std::min(alphaNew, 0.5 * alpha));
            } else {
                alpha = 0.5 * alpha;
            }
            continue;
        }

        /* 强Wolfe条件(曲率) */
        double dgNew = 0.0;
        for (int j = 0; j < n; ++j) dgNew += gradNew[j] * dir[j];
        if (std::abs(dgNew) <= -m_wolfeC2 * dg0) return alpha;

        if (dgNew >= 0.0) {
            /* 二分法回溯 */
            return 0.5 * (alpha + alphaPrev);
        }
        alphaPrev = alpha;
        fPrev = fNew;
        alpha = std::min(2.0 * alpha, 1e10);
    }
    return alpha;
}

/** @brief 执行L-BFGS优化 @param func 目标函数 @param x0 初始点 @param maxIter 最大迭代 @param gradTol 梯度容差 @return 结果 */
GradientDescent2::Result GradientDescent2::optimize(
    const ObjectiveFunc& func,
    const std::vector<double>& x0,
    int maxIter,
    double gradTol)
{
    QElapsedTimer timer;
    timer.start();

    Result result;
    int n = static_cast<int>(x0.size());
    if (n == 0) {
        result.converged = false;
        return result;
    }

    std::vector<double> x = x0;
    std::vector<double> grad(n);
    double fVal = func(x, grad);

    result.functionEvals = 1;

    /* L-BFGS历史存储 */
    QVector<std::vector<double>> sList;
    QVector<std::vector<double>> yList;
    QVector<double> rhoList;

    for (int iter = 0; iter < maxIter; ++iter) {
        /* 计算梯度范数 */
        double gNorm = 0.0;
        for (int j = 0; j < n; ++j) gNorm += grad[j] * grad[j];
        gNorm = std::sqrt(gNorm);

        emit iterationProgress(iter, fVal, gNorm);

        /* 收敛判据 */
        if (gNorm < gradTol) {
            result.converged = true;
            result.iterations = iter;
            break;
        }

        /* 双循环递归计算搜索方向 */
        std::vector<double> dir = twoLoopRecursion(grad, sList, yList, rhoList);

        /* 检查方向有效性 */
        double dNorm = 0.0;
        for (int j = 0; j < n; ++j) dNorm += dir[j] * dir[j];
        if (dNorm < 1e-30) {
            /* 使用负梯度方向作为后备 */
            for (int j = 0; j < n; ++j) dir[j] = -grad[j];
            sList.clear();
            yList.clear();
            rhoList.clear();
        }

        /* 线搜索 */
        double alpha = wolfeLineSearch(func, x, dir, grad, fVal);
        result.functionEvals += 2; /* 线搜索中的评估 */

        /* 更新位置 */
        std::vector<double> xNew(n);
        for (int j = 0; j < n; ++j) xNew[j] = x[j] + alpha * dir[j];

        /* 计算新梯度 */
        std::vector<double> gradNew(n);
        double fNew = func(xNew, gradNew);
        result.functionEvals++;

        /* 记录s = x_new - x, y = grad_new - grad */
        std::vector<double> s(n), y(n);
        double sy = 0.0;
        for (int j = 0; j < n; ++j) {
            s[j] = xNew[j] - x[j];
            y[j] = gradNew[j] - grad[j];
            sy += s[j] * y[j];
        }

        /* 只有s'y > 0时才更新(保证正定性) */
        if (sy > 1e-20) {
            double rho = 1.0 / sy;
            if (sList.size() >= m_memorySize) {
                sList.removeFirst();
                yList.removeFirst();
                rhoList.removeFirst();
            }
            sList.append(std::move(s));
            yList.append(std::move(y));
            rhoList.append(rho);
        }

        x = xNew;
        grad = gradNew;
        fVal = fNew;
        result.iterations = iter + 1;
    }

    result.x = x;
    result.functionValue = fVal;
    result.gradientNorm = 0.0;
    for (int j = 0; j < n; ++j) result.gradientNorm += grad[j] * grad[j];
    result.gradientNorm = std::sqrt(result.gradientNorm);

    /* 更新统计 */
    m_stats.totalOptimizations++;
    m_stats.totalIterations += result.iterations;
    m_stats.totalFunctionEvals += result.functionEvals;
    if (result.converged) m_stats.totalConverged++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOptimizations;

    emit optimizationFinished(result);
    return result;
}

/** @brief 重置统计 */
void GradientDescent2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

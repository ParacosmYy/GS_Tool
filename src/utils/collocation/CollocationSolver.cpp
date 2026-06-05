/**
 * @file CollocationSolver.cpp
 * @brief 配置法求解器实现 — 边值问题数值求解
 */

#include "utils/collocation/CollocationSolver.h"

#include <QElapsedTimer>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
CollocationSolver::CollocationSolver(QObject* parent)
    : QObject(parent)
    , m_timeSumMs(0.0)
{
}

/**
 * @brief 求解二阶边值问题 y'' = f(t, y, y')
 * @param f 右端函数
 * @param a 区间左端点
 * @param b 区间右端点
 * @param ya 左边界条件
 * @param yb 右边界条件
 * @param n 网格点数
 * @return 解的轨迹
 *
 * 对二阶BVP使用有限差分离散化:
 *   y'' ≈ (y_{i+1} - 2y_i + y_{i-1}) / h²
 *   y'  ≈ (y_{i+1} - y_{i-1}) / (2h)
 * 对线性项直接构造三对角方程组，非线性项使用Newton迭代线性化。
 */
QVector<QPair<double, double>> CollocationSolver::solve(
    std::function<double(double, double, double)> f,
    double a, double b, double ya, double yb, int n)
{
    QElapsedTimer timer;
    timer.start();

    /* 确保网格点数有效 */
    if (n < 4) n = 4;

    double h = (b - a) / static_cast<double>(n - 1);
    int interior = n - 2;  /* 内部节点数 */

    /* 初始化近似解为线性插值 */
    QVector<double> y(n, 0.0);
    for (int i = 0; i < n; ++i) {
        double t = a + i * h;
        y[i] = ya + (yb - ya) * (t - a) / (b - a);
    }
    y[0] = ya;
    y[n - 1] = yb;

    /* Newton迭代求解非线性方程组 */
    const int maxNewtonIter = 50;
    const double newtonTol = 1e-10;

    for (int iter = 0; iter < maxNewtonIter; ++iter) {
        /* 构造三对角方程组 */
        QVector<double> lower(interior, 0.0);
        QVector<double> mainDiag(interior, 0.0);
        QVector<double> upper(interior, 0.0);
        QVector<double> rhs(interior, 0.0);

        for (int i = 0; i < interior; ++i) {
            int idx = i + 1;  /* 全局索引 */
            double t = a + idx * h;

            double yi = y[idx];
            double yim1 = y[idx - 1];
            double yip1 = y[idx + 1];

            double fVal = f(t, yi,
                (yip1 - yim1) / (2.0 * h));

            /* 残差 */
            double residual = (yip1 - 2.0 * yi + yim1) / (h * h) - fVal;

            /* 数值Jacobian: 对y[idx-1], y[idx], y[idx+1]差分 */
            double eps = 1e-7;
            double dfdy = ((yip1 - 2.0 * (yi + eps) + yim1) / (h * h)
                - f(t, yi + eps, (yip1 - yim1) / (2.0 * h))
                - residual) / eps;

            double dfdyP1 = (1.0 / (h * h)
                - (f(t, yi,
                    (yip1 + eps - yim1) / (2.0 * h)) - fVal) / eps);

            double dfdyM1 = (1.0 / (h * h)
                - (fVal - f(t, yi,
                    (yip1 - yim1 - eps) / (2.0 * h))) / eps);

            mainDiag[i] = dfdy;
            upper[i] = dfdyP1;
            lower[i] = dfdyM1;
            rhs[i] = -residual;
        }

        /* 边界条件对右端项的贡献 */
        rhs[0] -= lower[0] * ya;
        rhs[interior - 1] -= upper[interior - 1] * yb;

        /* 求解三对角方程组 */
        QVector<double> dy = thomasSolve(lower, mainDiag, upper, rhs);

        /* 更新解 */
        double maxUpdate = 0.0;
        for (int i = 0; i < interior; ++i) {
            y[i + 1] += dy[i];
            maxUpdate = std::max(maxUpdate, std::abs(dy[i]));
        }

        if (maxUpdate < newtonTol) {
            break;
        }
    }

    /* 构建输出 */
    QVector<QPair<double, double>> result;
    result.reserve(n);
    for (int i = 0; i < n; ++i) {
        result.append({a + i * h, y[i]});
    }

    /* 更新统计 */
    double elapsed = timer.nsecsElapsed() / 1e6;
    m_timeSumMs += elapsed;
    ++m_stats.totalSolves;
    m_stats.avgProcessingTimeMs = m_timeSumMs / m_stats.totalSolves;

    emit solveCompleted(n);
    return result;
}

/**
 * @brief Thomas算法求解三对角线性方程组
 * @param lower 下对角线
 * @param mainDiag 主对角线
 * @param upper 上对角线
 * @param rhs 右端向量
 * @return 解向量
 *
 * 时间复杂度O(n)，空间复杂度O(n)。
 * 要求矩阵对角占优或对称正定以保证数值稳定性。
 */
QVector<double> CollocationSolver::thomasSolve(
    const QVector<double>& lower,
    const QVector<double>& mainDiag,
    const QVector<double>& upper,
    const QVector<double>& rhs)
{
    int n = mainDiag.size();
    if (n == 0) return {};

    QVector<double> c(upper);
    QVector<double> d(rhs);
    QVector<double> x(n, 0.0);

    /* 前向消元 */
    c[0] /= mainDiag[0];
    d[0] /= mainDiag[0];
    for (int i = 1; i < n; ++i) {
        double denom = mainDiag[i] - lower[i] * c[i - 1];
        if (std::abs(denom) < 1e-30) {
            denom = 1e-30;  /* 避免除零 */
        }
        if (i < n - 1) {
            c[i] /= denom;
        }
        d[i] = (d[i] - lower[i] * d[i - 1]) / denom;
    }

    /* 回代 */
    x[n - 1] = d[n - 1];
    for (int i = n - 2; i >= 0; --i) {
        x[i] = d[i] - c[i] * x[i + 1];
    }

    return x;
}

/** @brief 重置统计信息 */
void CollocationSolver::resetStatistics()
{
    m_stats = Stats{};
    m_timeSumMs = 0.0;
}

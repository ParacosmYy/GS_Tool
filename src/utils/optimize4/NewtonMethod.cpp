/**
 * @file NewtonMethod.cpp
 * @brief 牛顿法优化器实现 — Hessian修正 + 线搜索
 */

#include "utils/optimize4/NewtonMethod.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
NewtonMethod::NewtonMethod(QObject* parent)
    : QObject(parent)
    , m_hasAnalyticGradient(false)
    , m_hasAnalyticHessian(false)
    , m_timeSum(0.0)
{
}

/** @brief 设置目标函数 @param func 目标函数 */
void NewtonMethod::setObjective(ObjectiveFunc func)
{
    m_objective = std::move(func);
}

/** @brief 设置梯度函数 @param func 梯度函数 */
void NewtonMethod::setGradient(GradientFunc func)
{
    m_gradient = std::move(func);
    m_hasAnalyticGradient = true;
}

/** @brief 设置Hessian函数 @param func Hessian函数 */
void NewtonMethod::setHessian(HessianFunc func)
{
    m_hessian = std::move(func);
    m_hasAnalyticHessian = true;
}

/** @brief 执行牛顿法优化 @param x0 初始点 @param maxIter 最大迭代 @param tolerance 容差 @return 结果 */
NewtonMethod::Result NewtonMethod::optimize(const QVector<double>& x0,
                                             int maxIter,
                                             double tolerance)
{
    QElapsedTimer timer;
    timer.start();

    Result result;
    result.x = x0;
    result.converged = false;

    int n = x0.size();
    if (n <= 0 || !m_objective) {
        result.message = QStringLiteral("无效输入或未设置目标函数");
        return result;
    }

    result.functionValue = m_objective(result.x);

    for (int iter = 0; iter < maxIter; ++iter) {
        /* 计算梯度 */
        QVector<double> g = m_hasAnalyticGradient
            ? m_gradient(result.x)
            : numericalGradient(result.x);

        /* 检查梯度收敛 */
        double gradNorm = 0.0;
        for (const auto& gi : g) gradNorm += gi * gi;
        gradNorm = qSqrt(gradNorm);

        if (gradNorm < tolerance) {
            result.converged = true;
            result.message = QStringLiteral("梯度收敛于迭代 %1").arg(iter);
            result.iterations = iter;
            break;
        }

        /* 计算Hessian */
        QVector<QVector<double>> H = m_hasAnalyticHessian
            ? m_hessian(result.x)
            : numericalHessian(result.x);

        /* 修正Hessian确保正定 */
        QVector<QVector<double>> Hreg = regularizeHessian(H);

        /* 求解搜索方向 d: H * d = -g */
        QVector<double> d = solveLinearSystem(Hreg, g);
        for (auto& di : d) di = -di;

        /* 线搜索求步长 */
        double alpha = lineSearch(result.x, d);

        /* 更新迭代点 */
        for (int i = 0; i < n; ++i) {
            result.x[i] += alpha * d[i];
        }

        result.functionValue = m_objective(result.x);
        result.iterations = iter + 1;

        emit iterationProgress(iter + 1, result.functionValue);
    }

    if (!result.converged && result.iterations >= maxIter) {
        result.message = QStringLiteral("达到最大迭代次数 %1").arg(maxIter);
    }

    /* 更新统计 */
    m_stats.totalOptimizations++;
    m_stats.totalIterations += result.iterations;
    double elapsed = static_cast<double>(timer.nsecsElapsed()) / 1e6;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalOptimizations);

    return result;
}

/** @brief 数值梯度(中心差分) @param x 当前点 @param eps 步长 @return 梯度 */
QVector<double> NewtonMethod::numericalGradient(const QVector<double>& x,
                                                 double eps) const
{
    int n = x.size();
    QVector<double> grad(n, 0.0);

    for (int i = 0; i < n; ++i) {
        QVector<double> xPlus = x, xMinus = x;
        xPlus[i] += eps;
        xMinus[i] -= eps;
        grad[i] = (m_objective(xPlus) - m_objective(xMinus)) / (2.0 * eps);
    }
    return grad;
}

/** @brief 数值Hessian(中心差分) @param x 当前点 @param eps 步长 @return Hessian矩阵 */
QVector<QVector<double>> NewtonMethod::numericalHessian(
    const QVector<double>& x, double eps) const
{
    int n = x.size();
    QVector<QVector<double>> H(n, QVector<double>(n, 0.0));
    double f0 = m_objective(x);

    for (int i = 0; i < n; ++i) {
        /* 对角元素 */
        QVector<double> xPlus = x, xMinus = x;
        xPlus[i] += eps;
        xMinus[i] -= eps;
        H[i][i] = (m_objective(xPlus) - 2.0 * f0 + m_objective(xMinus))
                   / (eps * eps);

        /* 非对角元素 */
        for (int j = i + 1; j < n; ++j) {
            QVector<double> xpp = x, xpm = x, xmp = x, xmm = x;
            xpp[i] += eps; xpp[j] += eps;
            xpm[i] += eps; xpm[j] -= eps;
            xmp[i] -= eps; xmp[j] += eps;
            xmm[i] -= eps; xmm[j] -= eps;

            H[i][j] = H[j][i] = (m_objective(xpp) - m_objective(xpm)
                - m_objective(xmp) + m_objective(xmm)) / (4.0 * eps * eps);
        }
    }
    return H;
}

/** @brief 修正Hessian为正定(特征值截断) @param H Hessian @return 修正矩阵 */
QVector<QVector<double>> NewtonMethod::regularizeHessian(
    const QVector<QVector<double>>& H) const
{
    int n = H.size();
    if (n <= 0) return {};

    /* 简单对角正则化: H + lambda * I
     * 先尝试小lambda, 若不够则加倍 */
    double lambda = 1e-6;
    const double maxLambda = 1e6;

    while (lambda < maxLambda) {
        QVector<QVector<double>> Hreg(n, QVector<double>(n, 0.0));
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                Hreg[i][j] = H[i][j] + ((i == j) ? lambda : 0.0);
            }
        }

        /* 简单正定性检查: 所有对角元素 > 0 */
        bool allPositive = true;
        for (int i = 0; i < n; ++i) {
            if (Hreg[i][i] <= 0.0) {
                allPositive = false;
                break;
            }
        }

        if (allPositive) {
            return Hreg;
        }
        lambda *= 10.0;
    }

    /* 回退: 返回单位矩阵的倍数 */
    QVector<QVector<double>> fallback(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        fallback[i][i] = 1.0;
    }
    return fallback;
}

/** @brief 求解线性系统(Cholesky简化版: LDL^T分解) @param H 矩阵 @param g 右端 @return 解 */
QVector<double> NewtonMethod::solveLinearSystem(
    const QVector<QVector<double>>& H,
    const QVector<double>& g) const
{
    int n = H.size();
    if (n <= 0) return {};

    /* Gauss消元求解 H * x = g */
    QVector<QVector<double>> aug(n, QVector<double>(n + 1, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            aug[i][j] = H[i][j];
        }
        aug[i][n] = g[i];
    }

    /* 前向消元 */
    for (int col = 0; col < n; ++col) {
        int maxRow = col;
        double maxVal = qAbs(aug[col][col]);
        for (int row = col + 1; row < n; ++row) {
            if (qAbs(aug[row][col]) > maxVal) {
                maxVal = qAbs(aug[row][col]);
                maxRow = row;
            }
        }
        if (maxRow != col) std::swap(aug[col], aug[maxRow]);

        double pivot = aug[col][col];
        if (qAbs(pivot) < 1e-14) continue;

        for (int row = col + 1; row < n; ++row) {
            double factor = aug[row][col] / pivot;
            for (int j = col; j <= n; ++j) {
                aug[row][j] -= factor * aug[col][j];
            }
        }
    }

    /* 回代 */
    QVector<double> x(n, 0.0);
    for (int i = n - 1; i >= 0; --i) {
        x[i] = aug[i][n];
        for (int j = i + 1; j < n; ++j) {
            x[i] -= aug[i][j] * x[j];
        }
        if (qAbs(aug[i][i]) > 1e-14) {
            x[i] /= aug[i][i];
        }
    }
    return x;
}

/** @brief 回溯线搜索(Armijo条件) @param x 当前点 @param d 搜索方向 @param alpha0 初始步长 @return 步长 */
double NewtonMethod::lineSearch(const QVector<double>& x,
                                 const QVector<double>& d,
                                 double alpha0) const
{
    double alpha = alpha0;
    double f0 = m_objective(x);
    const double c1 = 1e-4;   /* Armijo参数 */
    const double rho = 0.5;   /* 步长缩减因子 */
    const int maxLS = 30;     /* 最大线搜索步数 */

    /* 计算初始方向导数 */
    double dirDeriv = 0.0;
    int n = x.size();
    for (int i = 0; i < n; ++i) {
        dirDeriv += d[i] * d[i]; /* g^T*d 近似(用d本身) */
    }

    for (int ls = 0; ls < maxLS; ++ls) {
        QVector<double> xNew(n);
        for (int i = 0; i < n; ++i) {
            xNew[i] = x[i] + alpha * d[i];
        }
        double fNew = m_objective(xNew);

        /* Armijo条件: f(x + alpha*d) <= f(x) + c1*alpha*dirDeriv */
        if (fNew <= f0 + c1 * alpha * (-dirDeriv)) {
            return alpha;
        }
        alpha *= rho;
    }
    return alpha;
}

/** @brief 重置统计信息 */
void NewtonMethod::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

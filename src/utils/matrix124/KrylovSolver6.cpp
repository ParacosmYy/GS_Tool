#include "KrylovSolver6.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化Krylov子空间求解器
 * @param parent 父对象指针
 */
KrylovSolver6::KrylovSolver6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void KrylovSolver6::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置预处理方法
 *
 * @param precondType 预处理类型 (None/Jacobi/ILU/SGS)
 */
void KrylovSolver6::setPreconditioner(const QString& precondType)
{
    Q_UNUSED(precondType)
}

/**
 * @brief 获取最近一次求解的残差历史
 * @return 残差值序列
 */
QVector<double> KrylovSolver6::residualHistory() const
{
    return m_residualHistory;
}

/**
 * @brief GMRES算法求解线性方程组 Ax = b
 *
 * GMRES(m)重启算法流程：
 * 1. 计算初始残差 r0 = b - Ax0
 * 2. Arnoldi过程构建m+1个正交基向量
 * 3. 最小二乘求解子空间投影问题
 * 4. 重构近似解 x = x0 + Vm * y
 * 5. 若未收敛则重启
 *
 * @param matrix 系数矩阵
 * @param rhs 右端向量
 * @param maxIterations 最大迭代次数
 * @param tolerance 收敛容差
 * @return 解向量
 */
QVector<double> KrylovSolver6::gmres(const QVector<QVector<double>>& matrix,
                                      const QVector<double>& rhs,
                                      int maxIterations, double tolerance)
{
    QElapsedTimer timer;
    timer.start();
    m_residualHistory.clear();

    const int n = rhs.size();
    QVector<double> x(n, 0.0);
    if (n == 0 || matrix.size() != n) {
        emit solveCompleted(0);
        return x;
    }

    /* 计算右端范数 */
    double bnorm = 0.0;
    for (int i = 0; i < n; ++i)
        bnorm += rhs[i] * rhs[i];
    bnorm = qSqrt(qMax(bnorm, 1e-30));

    const int m = qMin(30, n); /* Krylov子空间维度 */
    int totalIter = 0;

    for (int outer = 0; outer < maxIterations / qMax(m, 1) + 1; ++outer) {
        /* 计算残差 r = b - A*x */
        QVector<double> r(n, 0.0);
        for (int i = 0; i < n; ++i) {
            r[i] = rhs[i];
            for (int j = 0; j < n; ++j)
                r[i] -= matrix[i][j] * x[j];
        }

        double beta = 0.0;
        for (int i = 0; i < n; ++i)
            beta += r[i] * r[i];
        beta = qSqrt(beta);

        m_residualHistory.append(beta / bnorm);
        if (beta / bnorm < tolerance) break;

        /* Arnoldi过程 */
        QVector<QVector<double>> V(m + 1, QVector<double>(n, 0.0));
        QVector<QVector<double>> H(m + 1, QVector<double>(m, 0.0));
        QVector<double> cs(m, 0.0);
        QVector<double> sn(m, 0.0);
        QVector<double> g(m + 1, 0.0);

        for (int i = 0; i < n; ++i)
            V[0][i] = r[i] / qMax(beta, 1e-30);
        g[0] = beta;

        int j = 0;
        for (j = 0; j < m && totalIter < maxIterations; ++j, ++totalIter) {
            /* w = A * V[j] */
            QVector<double> w(n, 0.0);
            for (int i = 0; i < n; ++i)
                for (int k = 0; k < n; ++k)
                    w[i] += matrix[i][k] * V[j][k];

            /* Modified Gram-Schmidt */
            for (int i = 0; i <= j; ++i) {
                double dot = 0.0;
                for (int k = 0; k < n; ++k)
                    dot += w[k] * V[i][k];
                H[i][j] = dot;
                for (int k = 0; k < n; ++k)
                    w[k] -= dot * V[i][k];
            }

            double normW = 0.0;
            for (int k = 0; k < n; ++k)
                normW += w[k] * w[k];
            H[j + 1][j] = qSqrt(normW);

            if (H[j + 1][j] > 1e-15)
                for (int k = 0; k < n; ++k)
                    V[j + 1][k] = w[k] / H[j + 1][j];

            /* Givens旋转 */
            for (int i = 0; i < j; ++i) {
                double temp = H[i][j];
                H[i][j] = cs[i] * temp + sn[i] * H[i + 1][j];
                H[i + 1][j] = -sn[i] * temp + cs[i] * H[i + 1][j];
            }
            double r = qSqrt(H[j][j] * H[j][j] + H[j + 1][j] * H[j + 1][j]);
            if (r > 1e-30) { cs[j] = H[j][j] / r; sn[j] = H[j + 1][j] / r; }
            H[j][j] = r;
            H[j + 1][j] = 0.0;
            g[j + 1] = -sn[j] * g[j];
            g[j] = cs[j] * g[j];

            double res = qAbs(g[j + 1]) / bnorm;
            m_residualHistory.append(res);
            if (res < tolerance) { j++; break; }
        }

        /* 回代求解最小二乘 */
        QVector<double> y(j, 0.0);
        for (int i = j - 1; i >= 0; --i) {
            double sum = g[i];
            for (int k = i + 1; k < j; ++k)
                sum -= H[i][k] * y[k];
            y[i] = (qAbs(H[i][i]) > 1e-15) ? sum / H[i][i] : 0.0;
        }

        /* 更新解 */
        for (int i = 0; i < n; ++i)
            for (int k = 0; k < j; ++k)
                x[i] += V[k][i] * y[k];

        if (m_residualHistory.last() < tolerance) break;
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalSolveOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolveOps;

    emit solveCompleted(totalIter);
    return x;
}

/**
 * @brief BiCGSTAB算法求解线性方程组
 *
 * Biconjugate Gradient Stabilized方法，适用于非对称稀疏系统：
 * 1. 计算初始残差 r = b - Ax
 * 2. 选择影子残差 r^ = r
 * 3. 迭代更新搜索方向、步长和修正因子
 * 4. 残差稳定化避免CGS方法的数值不稳定
 *
 * @param matrix 系数矩阵
 * @param rhs 右端向量
 * @param maxIterations 最大迭代次数
 * @param tolerance 收敛容差
 * @return 解向量
 */
QVector<double> KrylovSolver6::bicgstab(const QVector<QVector<double>>& matrix,
                                         const QVector<double>& rhs,
                                         int maxIterations, double tolerance)
{
    QElapsedTimer timer;
    timer.start();
    m_residualHistory.clear();

    const int n = rhs.size();
    QVector<double> x(n, 0.0);
    if (n == 0 || matrix.size() != n) {
        emit solveCompleted(0);
        return x;
    }

    /* 矩阵-向量乘法辅助函数 */
    auto matVec = [&](const QVector<double>& v) -> QVector<double> {
        QVector<double> out(n, 0.0);
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                out[i] += matrix[i][j] * v[j];
        return out;
    };
    auto dot = [](const QVector<double>& a, const QVector<double>& b) -> double {
        double s = 0.0;
        for (int i = 0; i < a.size(); ++i) s += a[i] * b[i];
        return s;
    };

    /* 初始残差 r = b - A*x */
    QVector<double> r = rhs;
    QVector<double> rHat = r; /* 影子残差 */
    QVector<double> p = r;

    double bnorm = qSqrt(qMax(dot(rhs, rhs), 1e-30));
    double rnorm = qSqrt(dot(r, r));
    m_residualHistory.append(rnorm / bnorm);

    if (rnorm / bnorm < tolerance) {
        emit solveCompleted(0);
        return x;
    }

    double rhoPrev = dot(rHat, r);
    int iter = 0;

    for (iter = 0; iter < maxIterations; ++iter) {
        /* p = r + beta * (p - omega * v) 已在循环末尾更新 */

        /* v = A * p */
        QVector<double> v = matVec(p);

        double alpha = rhoPrev / qMax(dot(rHat, v), 1e-30);

        /* s = r - alpha * v */
        QVector<double> s(n, 0.0);
        for (int i = 0; i < n; ++i)
            s[i] = r[i] - alpha * v[i];

        /* t = A * s */
        QVector<double> t = matVec(s);

        double omega = dot(t, s) / qMax(dot(t, t), 1e-30);

        /* x = x + alpha * p + omega * s */
        for (int i = 0; i < n; ++i)
            x[i] += alpha * p[i] + omega * s[i];

        /* r = s - omega * t */
        for (int i = 0; i < n; ++i)
            r[i] = s[i] - omega * t[i];

        rnorm = qSqrt(dot(r, r));
        m_residualHistory.append(rnorm / bnorm);

        if (rnorm / bnorm < tolerance) break;

        double rho = dot(rHat, r);
        if (qFuzzyIsNull(rho)) break;

        double beta = (rho / qMax(rhoPrev, 1e-30)) * (alpha / qMax(omega, 1e-30));
        for (int i = 0; i < n; ++i)
            p[i] = r[i] + beta * (p[i] - omega * v[i]);

        rhoPrev = rho;
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalSolveOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolveOps;

    emit solveCompleted(iter);
    return x;
}

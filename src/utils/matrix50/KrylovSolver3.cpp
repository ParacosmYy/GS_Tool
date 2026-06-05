/**
 * @file KrylovSolver3.cpp
 * @brief Krylov求解器3 — MINRES+对称不定系统 实现
 *
 * 实现三种 Krylov 子空间迭代求解器：
 * 1. CG — 共轭梯度法（对称正定系统）
 * 2. MINRES — 最小残差法（对称不定系统）
 * 3. BiCGSTAB — 双共轭梯度稳定法（一般非对称系统）
 *
 * 所有方法使用 CSR 稀疏矩阵存储格式。
 */

#include "utils/matrix50/KrylovSolver3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

namespace {

/** @brief 向量点积 */
double dotProduct(const QVector<double>& a, const QVector<double>& b)
{
    double sum = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i) {
        sum += a[i] * b[i];
    }
    return sum;
}

} /* anonymous namespace */

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
KrylovSolver3::KrylovSolver3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置稀疏矩阵（CSR格式）
 * @param n 矩阵维度
 * @param rowPtr 行指针数组，长度 n+1
 * @param colIdx 列索引数组
 * @param values 非零值数组
 */
void KrylovSolver3::setMatrix(int n, const QVector<int>& rowPtr,
                               const QVector<int>& colIdx,
                               const QVector<double>& values)
{
    m_n = n;
    m_rowPtr = rowPtr;
    m_colIdx = colIdx;
    m_values = values;
    m_stats.matrixSize = n;
}

/**
 * @brief 设置收敛容差
 * @param tol 残差范数阈值
 */
void KrylovSolver3::setTolerance(double tol)
{
    m_tol = qMax(1e-15, tol);
}

/**
 * @brief 设置最大迭代次数
 * @param maxIter 最大迭代数
 */
void KrylovSolver3::setMaxIterations(int maxIter)
{
    m_maxIter = qMax(1, maxIter);
}

/**
 * @brief 共轭梯度法求解 Ax = b
 *
 * 适用于对称正定矩阵。每次迭代在 Krylov 子空间中寻找最优步长。
 * 收敛速度取决于条件数。
 *
 * @param rhs 右端向量 b
 * @return 解向量 x
 */
QVector<double> KrylovSolver3::solveCG(const QVector<double>& rhs)
{
    QElapsedTimer timer;
    timer.start();

    if (m_n == 0 || rhs.size() != m_n) {
        return {};
    }

    QVector<double> x(m_n, 0.0);
    QVector<double> r = rhs; /* r = b - A*x = b (初始 x=0) */
    QVector<double> p = r;
    double rsOld = dotProduct(r, r);

    m_iterUsed = 0;
    for (int i = 0; i < m_maxIter; ++i) {
        m_iterUsed = i + 1;

        QVector<double> ap = spmv(p);
        double pAp = dotProduct(p, ap);

        if (qFabs(pAp) < 1e-30) break;

        double alpha = rsOld / pAp;

        for (int j = 0; j < m_n; ++j) {
            x[j] += alpha * p[j];
            r[j] -= alpha * ap[j];
        }

        double rsNew = dotProduct(r, r);
        m_resNorm = qSqrt(rsNew);

        if (m_resNorm < m_tol) break;

        double beta = rsNew / rsOld;
        for (int j = 0; j < m_n; ++j) {
            p[j] = r[j] + beta * p[j];
        }
        rsOld = rsNew;
    }

    /* 统计更新 */
    m_timeSum += timer.elapsed();
    m_stats.totalSolves++;
    m_stats.totalIterations += m_iterUsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;
    emit solveCompleted("CG", m_iterUsed, m_resNorm);
    return x;
}

/**
 * @brief MINRES 法求解 Ax = b
 *
 * 适用于对称（可能不定）矩阵。通过 Lanczos 三对角化
 * 在最小残差意义下求解。
 *
 * @param rhs 右端向量 b
 * @return 解向量 x
 */
QVector<double> KrylovSolver3::solveMINRES(const QVector<double>& rhs)
{
    QElapsedTimer timer;
    timer.start();

    if (m_n == 0 || rhs.size() != m_n) {
        return {};
    }

    QVector<double> x(m_n, 0.0);
    QVector<double> r = rhs;
    double beta1 = qSqrt(dotProduct(r, r));

    if (beta1 < m_tol) {
        m_resNorm = beta1;
        m_iterUsed = 0;
        /* 统计更新 */
        m_timeSum += timer.elapsed();
        m_stats.totalSolves++;
        m_stats.totalIterations += m_iterUsed;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;
        emit solveCompleted("MINRES", m_iterUsed, m_resNorm);
        return x;
    }

    QVector<double> vOld(m_n, 0.0);
    QVector<double> vCur = r;
    for (int j = 0; j < m_n; ++j) vCur[j] /= beta1;
    QVector<double> vNew(m_n, 0.0);

    /* 三对角矩阵的旋转参数 */
    double cPrev = 1.0, sPrev = 0.0;
    double cCur = 1.0, sCur = 0.0;
    double dPrev = 0.0, dCur = 0.0;

    /* 搜索方向 */
    QVector<double> w(m_n, 0.0);
    QVector<double> wPrev(m_n, 0.0);
    QVector<double> wPrev2(m_n, 0.0);

    double resNorm = beta1;
    QVector<double> rhsVec = {beta1, 0.0, 0.0};

    m_iterUsed = 0;
    for (int iter = 0; iter < m_maxIter; ++iter) {
        m_iterUsed = iter + 1;

        /* Lanczos 步骤 */
        QVector<double> Av = spmv(vCur);
        double alpha = dotProduct(vCur, Av);

        /* 正交化 */
        for (int j = 0; j < m_n; ++j) {
            vNew[j] = Av[j] - alpha * vCur[j];
            if (iter > 0) {
                vNew[j] -= beta1 * vOld[j]; /* 这里用 beta1 存储 beta_{k-1} */
            }
        }

        double betaNew = qSqrt(dotProduct(vNew, vNew));
        if (betaNew > 1e-30) {
            for (int j = 0; j < m_n; ++j) {
                vNew[j] /= betaNew;
            }
        }

        /* 最小残差旋转更新 */
        double dAlpha = cCur * alpha - sCur * dPrev;
        double dBeta = sCur * betaNew;
        dPrev = dAlpha;

        double gamma = qSqrt(dAlpha * dAlpha + betaNew * betaNew);
        if (gamma > 1e-30) {
            cCur = dAlpha / gamma;
            sCur = betaNew / gamma;
        }

        /* 更新解 */
        for (int j = 0; j < m_n; ++j) {
            w[j] = (vCur[j] - dBeta * wPrev[j] - dPrev * wPrev2[j]) / gamma;
            x[j] += cCur * beta1 * w[j];
        }

        beta1 = sCur * betaNew;
        resNorm = qFabs(beta1);
        m_resNorm = resNorm;

        if (resNorm < m_tol) break;

        /* 移位 */
        wPrev2 = wPrev;
        wPrev = w;
        vOld = vCur;
        vCur = vNew;
    }

    /* 统计更新 */
    m_timeSum += timer.elapsed();
    m_stats.totalSolves++;
    m_stats.totalIterations += m_iterUsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;
    emit solveCompleted("MINRES", m_iterUsed, m_resNorm);
    return x;
}

/**
 * @brief BiCGSTAB 法求解 Ax = b
 *
 * 适用于一般非对称系统。结合双共轭梯度和稳定化步骤，
 * 避免了 CGS 的不规则收敛行为。
 *
 * @param rhs 右端向量 b
 * @return 解向量 x
 */
QVector<double> KrylovSolver3::solveBiCGSTAB(const QVector<double>& rhs)
{
    QElapsedTimer timer;
    timer.start();

    if (m_n == 0 || rhs.size() != m_n) {
        return {};
    }

    QVector<double> x(m_n, 0.0);
    QVector<double> r = rhs; /* r = b - A*0 = b */
    QVector<double> rHat = r; /* 影子残差 */

    double rho1 = 1.0, omega = 1.0, alpha = 1.0;
    QVector<double> p(m_n, 0.0);
    QVector<double> v(m_n, 0.0);

    double bNorm = qSqrt(dotProduct(rhs, rhs));
    if (bNorm < 1e-30) bNorm = 1.0;

    m_iterUsed = 0;
    for (int iter = 0; iter < m_maxIter; ++iter) {
        m_iterUsed = iter + 1;

        double rho = dotProduct(rHat, r);
        if (qFabs(rho) < 1e-30) break;

        double beta = (rho / rho1) * (alpha / omega);
        rho1 = rho;

        /* p = r + beta * (p - omega * v) */
        for (int j = 0; j < m_n; ++j) {
            p[j] = r[j] + beta * (p[j] - omega * v[j]);
        }

        v = spmv(p);
        double rHatV = dotProduct(rHat, v);
        if (qFabs(rHatV) < 1e-30) break;
        alpha = rho / rHatV;

        /* s = r - alpha * v */
        QVector<double> s(m_n);
        for (int j = 0; j < m_n; ++j) {
            s[j] = r[j] - alpha * v[j];
        }

        QVector<double> t = spmv(s);
        omega = dotProduct(t, s) / dotProduct(t, t);
        if (qFabs(omega) < 1e-30) omega = 1.0;

        /* 更新解和残差 */
        for (int j = 0; j < m_n; ++j) {
            x[j] += alpha * p[j] + omega * s[j];
            r[j] = s[j] - omega * t[j];
        }

        m_resNorm = qSqrt(dotProduct(r, r)) / bNorm;
        if (m_resNorm < m_tol) break;
    }

    /* 统计更新 */
    m_resNorm = qSqrt(dotProduct(r, r));
    m_timeSum += timer.elapsed();
    m_stats.totalSolves++;
    m_stats.totalIterations += m_iterUsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;
    emit solveCompleted("BiCGSTAB", m_iterUsed, m_resNorm);
    return x;
}

/**
 * @brief 重置统计信息
 */
void KrylovSolver3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 稀疏矩阵向量乘法 y = A * x
 * @param x 输入向量
 * @return 输出向量 y
 */
QVector<double> KrylovSolver3::spmv(const QVector<double>& x) const
{
    QVector<double> y(m_n, 0.0);
    for (int i = 0; i < m_n; ++i) {
        double sum = 0.0;
        for (int jj = m_rowPtr[i]; jj < m_rowPtr[i + 1]; ++jj) {
            sum += m_values[jj] * x[m_colIdx[jj]];
        }
        y[i] = sum;
    }
    return y;
}

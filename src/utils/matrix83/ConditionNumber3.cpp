/**
 * @file ConditionNumber3.cpp
 * @brief 矩阵条件数估计器实现 — 幂迭代法估计范数+稳定性评级
 */

#include "utils/matrix83/ConditionNumber3.h"

#include <QElapsedTimer>

#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
ConditionNumber3::ConditionNumber3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 计算1-范数条件数 kappa_1(A) = ||A||_1 * ||A^{-1}||_1
 * @param matrix 输入方阵
 * @return 1-范数条件数(>=1.0)
 */
double ConditionNumber3::conditionNumber1(const QVector<QVector<double>>& matrix) const
{
    int n = matrix.size();
    if (n == 0) return 0.0;

    /* 1-范数: 列绝对值和的最大值 */
    double normA = 0.0;
    for (int j = 0; j < n; ++j) {
        double colSum = 0.0;
        for (int i = 0; i < n; ++i) {
            colSum += std::abs(matrix[i][j]);
        }
        normA = std::max(normA, colSum);
    }

    if (normA < 1e-15) return 1e18; /* 奇异矩阵 */

    /* 估计||A^{-1}||_1使用Hager-Higham算法(幂迭代) */
    double normInvA = estimateNorm1Inverse(matrix);

    return normA * normInvA;
}

/**
 * @brief 计算2-范数条件数 kappa_2(A) = sigma_max / sigma_min
 * 使用幂迭代法估计最大和最小奇异值
 * @param matrix 输入方阵
 * @return 2-范数条件数
 */
double ConditionNumber3::conditionNumber2(const QVector<QVector<double>>& matrix)
{
    QElapsedTimer timer;
    timer.start();

    int n = matrix.size();
    if (n == 0) return 0.0;

    /* 构造A^T*A用于奇异值估计 */
    auto matVecMul = [&](const QVector<double>& x, bool transpose) -> QVector<double> {
        QVector<double> y(n, 0.0);
        if (transpose) {
            for (int i = 0; i < n; ++i) {
                for (int j = 0; j < n; ++j) {
                    if (j < matrix[i].size())
                        y[j] += matrix[i][j] * x[i];
                }
            }
        } else {
            for (int i = 0; i < n; ++i) {
                for (int j = 0; j < std::min(n, (int)matrix[i].size()); ++j) {
                    y[i] += matrix[i][j] * x[j];
                }
            }
        }
        return y;
    };

    /* --- 幂迭代求最大奇异值 sigma_max --- */
    QVector<double> v(n, 1.0 / std::sqrt(static_cast<double>(n)));
    double sigmaMax = 0.0;

    for (int iter = 0; iter < 200; ++iter) {
        QVector<double> u = matVecMul(v, false);
        double normU = 0.0;
        for (double val : u) normU += val * val;
        normU = std::sqrt(normU);
        if (normU < 1e-30) break;
        for (int i = 0; i < n; ++i) u[i] /= normU;

        QVector<double> vNew = matVecMul(u, true);
        sigmaMax = 0.0;
        for (double val : vNew) sigmaMax += val * val;
        sigmaMax = std::sqrt(sigmaMax);
        if (sigmaMax < 1e-30) break;
        for (int i = 0; i < n; ++i) vNew[i] /= sigmaMax;

        /* 检查收敛 */
        double diff = 0.0;
        for (int i = 0; i < n; ++i) diff += (vNew[i] - v[i]) * (vNew[i] - v[i]);
        v = vNew;
        if (std::sqrt(diff) < 1e-12) break;
    }

    /* --- 逆迭代求最小奇异值 sigma_min --- *
     * 使用 (sigma_max^2 * I - A^T*A) 的幂迭代, 避免显式求逆 */
    QVector<double> w(n, 1.0 / std::sqrt(static_cast<double>(n)));
    double sigmaMin = 0.0;
    double sigmaMaxSq = sigmaMax * sigmaMax;

    /* 构造 M = sigma_max^2 * I - A^T*A 的隐式矩阵-向量乘 */
    auto shiftedMul = [&](const QVector<double>& x) -> QVector<double> {
        QVector<double> ax = matVecMul(x, false);
        QVector<double> atax = matVecMul(ax, true);
        QVector<double> result(n);
        for (int i = 0; i < n; ++i) {
            result[i] = sigmaMaxSq * x[i] - atax[i];
        }
        return result;
    };

    double lambdaShift = 0.0;
    for (int iter = 0; iter < 200; ++iter) {
        QVector<double> y = shiftedMul(w);
        double normY = 0.0;
        for (double val : y) normY += val * val;
        normY = std::sqrt(normY);
        if (normY < 1e-30) break;

        lambdaShift = normY;
        for (int i = 0; i < n; ++i) y[i] /= normY;

        double diff = 0.0;
        for (int i = 0; i < n; ++i) diff += (y[i] - w[i]) * (y[i] - w[i]);
        w = y;
        if (std::sqrt(diff) < 1e-12) break;
    }

    /* sigma_min^2 = sigma_max^2 - lambda_max(M) */
    double sigmaMinSq = sigmaMaxSq - lambdaShift;
    sigmaMin = (sigmaMinSq > 0) ? std::sqrt(sigmaMinSq) : 0.0;

    double kappa = (sigmaMin > 1e-15) ? sigmaMax / sigmaMin : 1e18;

    /* --- 更新统计 --- */
    m_stats.totalEstimates++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalEstimates;

    emit conditionEstimated(kappa, stabilityRating(kappa));
    return kappa;
}

/**
 * @brief 计算无穷范数条件数 kappa_inf(A) = ||A||_inf * ||A^{-1}||_inf
 * @param matrix 输入方阵
 * @return 无穷范数条件数
 */
double ConditionNumber3::conditionNumberInf(const QVector<QVector<double>>& matrix) const
{
    int n = matrix.size();
    if (n == 0) return 0.0;

    /* 无穷范数: 行绝对值和的最大值 */
    double normA = 0.0;
    for (int i = 0; i < n; ++i) {
        double rowSum = 0.0;
        for (int j = 0; j < matrix[i].size(); ++j) {
            rowSum += std::abs(matrix[i][j]);
        }
        normA = std::max(normA, rowSum);
    }

    if (normA < 1e-15) return 1e18;

    /* kappa_inf(A) = kappa_1(A^T) */
    /* 转置矩阵 */
    QVector<QVector<double>> at(n);
    for (int i = 0; i < n; ++i) {
        at[i].resize(n);
        for (int j = 0; j < n; ++j) {
            at[i][j] = (j < matrix.size() && i < matrix[j].size()) ? matrix[j][i] : 0.0;
        }
    }

    double normInvAT = estimateNorm1Inverse(at);
    /* ||A^{-1}||_inf = ||(A^{-1})^T||_1 = ||(A^T)^{-1}||_1 */
    return normA * normInvAT;
}

/**
 * @brief 评估数值稳定性等级
 * @param conditionNumber 计算得到的条件数
 * @return 稳定性等级: "good"/"fair"/"poor"/"singular"
 */
QString ConditionNumber3::stabilityRating(double conditionNumber) const
{
    if (conditionNumber > 1e15) return QStringLiteral("singular");
    if (conditionNumber > 1e10) return QStringLiteral("poor");
    if (conditionNumber > 1e5)  return QStringLiteral("fair");
    return QStringLiteral("good");
}

/** @brief 重置统计信息 */
void ConditionNumber3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief Hager-Higham算法估计1-范数||A^{-1}||_1 (内部方法)
 * 通过求解 A^T * x = sign(z) 的迭代来估计范数
 * @param matrix 输入方阵
 * @return ||A^{-1}||_1的估计值
 */
double ConditionNumber3::estimateNorm1Inverse(const QVector<QVector<double>>& matrix) const
{
    int n = matrix.size();
    if (n == 0) return 0.0;

    /* 构造稠密LU分解用于求解 A*x = b */
    QVector<QVector<double>> lu = matrix;
    QVector<int> piv(n);
    for (int i = 0; i < n; ++i) piv[i] = i;

    /* LU分解(部分主元) */
    for (int k = 0; k < n; ++k) {
        /* 选主元 */
        int maxRow = k;
        double maxVal = std::abs(lu[k][k]);
        for (int i = k + 1; i < n; ++i) {
            if (std::abs(lu[i][k]) > maxVal) {
                maxVal = std::abs(lu[i][k]);
                maxRow = i;
            }
        }
        if (maxRow != k) {
            std::swap(lu[k], lu[maxRow]);
            std::swap(piv[k], piv[maxRow]);
        }

        if (std::abs(lu[k][k]) < 1e-15) continue;

        for (int i = k + 1; i < n; ++i) {
            lu[i][k] /= lu[k][k];
            for (int j = k + 1; j < n; ++j) {
                lu[i][j] -= lu[i][k] * lu[k][j];
            }
        }
    }

    /* LU求解函数 */
    auto luSolve = [&](QVector<double> b) -> QVector<double> {
        /* 排列 */
        for (int i = 0; i < n; ++i) {
            if (piv[i] != i) std::swap(b[i], b[piv[i]]);
        }
        /* 前代 */
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < i; ++j) {
                b[i] -= lu[i][j] * b[j];
            }
        }
        /* 回代 */
        for (int i = n - 1; i >= 0; --i) {
            for (int j = i + 1; j < n; ++j) {
                b[i] -= lu[i][j] * b[j];
            }
            if (std::abs(lu[i][i]) > 1e-15)
                b[i] /= lu[i][i];
        }
        return b;
    };

    /* Hager迭代: 估计||A^{-1}||_1 */
    QVector<double> x(n, 1.0 / static_cast<double>(n));
    double estOld = 0.0;
    double est = 0.0;

    for (int iter = 0; iter < 10; ++iter) {
        QVector<double> z = luSolve(x);

        /* 计算1-范数 */
        est = 0.0;
        for (double val : z) est += std::abs(val);

        /* 构造sign向量 */
        QVector<double> xNew(n);
        for (int i = 0; i < n; ++i) {
            xNew[i] = (z[i] >= 0) ? 1.0 : -1.0;
        }

        /* 检查收敛 */
        if (est <= estOld) break;
        estOld = est;

        /* 求解 A^T * x = sign(z) */
        /* 转置LU求解: U^T * y = b, L^T * x = y */
        QVector<double> y = xNew;
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < i; ++j) {
                y[i] -= lu[j][i] * y[j]; /* U^T */
            }
            if (std::abs(lu[i][i]) > 1e-15)
                y[i] /= lu[i][i];
        }
        for (int i = n - 1; i >= 0; --i) {
            for (int j = i + 1; j < n; ++j) {
                y[i] -= lu[j][i] * y[j]; /* L^T */
            }
        }
        x = y;
    }

    return est;
}

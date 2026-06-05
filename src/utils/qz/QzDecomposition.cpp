/**
 * @file QzDecomposition.cpp
 * @brief QZ分解实现 — 广义Schur分解求广义特征值
 *
 * 算法: 先用Givens旋转将B约化为上三角，同时变换A；
 *        再通过QZ迭代将S约化为上三角，从对角线提取特征值。
 */

#include "utils/qz/QzDecomposition.h"

#include <QElapsedTimer>
#include <cmath>

QzDecomposition::QzDecomposition(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief 构造Givens旋转参数 */
static void givens(double a, double b, double& c, double& s)
{
    if (std::abs(b) < 1e-30) { c = 1.0; s = 0.0; }
    else if (std::abs(b) > std::abs(a)) {
        double t = -a / b;
        s = 1.0 / std::sqrt(1.0 + t * t); c = s * t;
    } else {
        double t = -b / a;
        c = 1.0 / std::sqrt(1.0 + t * t); s = c * t;
    }
}

/**
 * @brief 第一步: 将B约化为上三角R，同时变换A
 */
static void reduceBtoUpperTri(QVector<QVector<double>>& S,
                              QVector<QVector<double>>& T, int n)
{
    for (int j = 0; j < n; ++j) {
        for (int i = n - 1; i > j; --i) {
            double c, s;
            givens(T[i - 1][j], T[i][j], c, s);

            for (int k = j; k < n; ++k) {
                double t1 =  c * T[i - 1][k] + s * T[i][k];
                double t2 = -s * T[i - 1][k] + c * T[i][k];
                T[i - 1][k] = t1; T[i][k] = t2;
            }
            for (int k = 0; k < n; ++k) {
                double s1 =  c * S[i - 1][k] + s * S[i][k];
                double s2 = -s * S[i - 1][k] + c * S[i][k];
                S[i - 1][k] = s1; S[i][k] = s2;
            }
        }
    }
}

/**
 * @brief 单步QZ扫描: 对S的次对角线做Givens旋转
 */
static void qzSweep(QVector<QVector<double>>& S,
                    QVector<QVector<double>>& T, int n)
{
    for (int i = 0; i < n - 1; ++i) {
        double c, s;
        givens(S[i][i], S[i + 1][i], c, s);

        /* 行旋转S和T */
        for (int k = i; k < n; ++k) {
            double t1 =  c * S[i][k] + s * S[i + 1][k];
            double t2 = -s * S[i][k] + c * S[i + 1][k];
            S[i][k] = t1; S[i + 1][k] = t2;
        }
        for (int k = 0; k < n; ++k) {
            double t1 =  c * T[i][k] + s * T[i + 1][k];
            double t2 = -s * T[i][k] + c * T[i + 1][k];
            T[i][k] = t1; T[i + 1][k] = t2;
        }

        /* 恢复T的上三角性 */
        if (i + 2 < n) {
            double c2, s2;
            givens(T[i][i + 1], T[i][i + 2], c2, s2);
            for (int k = 0; k < n; ++k) {
                double u = S[k][i + 1], v = S[k][i + 2];
                S[k][i + 1] =  c2 * u + s2 * v;
                S[k][i + 2] = -s2 * u + c2 * v;
            }
            for (int k = 0; k < n; ++k) {
                double u = T[k][i + 1], v = T[k][i + 2];
                T[k][i + 1] =  c2 * u + s2 * v;
                T[k][i + 2] = -s2 * u + c2 * v;
            }
        }
    }
}

/** @brief 检查S是否已约化为上三角 */
static bool isUpperTri(const QVector<QVector<double>>& S, int n)
{
    for (int i = 1; i < n; ++i) {
        if (std::abs(S[i][i - 1]) > 1e-10) return false;
    }
    return true;
}

QVector<double> QzDecomposition::decompose(
    const QVector<QVector<double>>& matA,
    const QVector<QVector<double>>& matB,
    int maxIter)
{
    QElapsedTimer timer;
    timer.start();

    int n = matA.size();
    if (n == 0 || matB.size() != n) {
        emit decompositionCompleted(0);
        return {};
    }

    QVector<QVector<double>> S = matA;
    QVector<QVector<double>> T = matB;

    /* B约化为上三角 */
    reduceBtoUpperTri(S, T, n);

    /* QZ迭代 */
    for (int iter = 0; iter < maxIter; ++iter) {
        if (isUpperTri(S, n)) break;
        qzSweep(S, T, n);
    }

    /* 提取广义特征值 */
    QVector<double> eigenvalues(n, 0.0);
    for (int i = 0; i < n; ++i) {
        double alpha = S[i][i], beta = T[i][i];
        eigenvalues[i] = (std::abs(beta) > 1e-30)
            ? alpha / beta
            : ((alpha >= 0) ? 1e15 : -1e15);
    }

    ++m_stats.totalDecompositions;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecompositions;

    emit decompositionCompleted(n);
    return eigenvalues;
}

void QzDecomposition::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

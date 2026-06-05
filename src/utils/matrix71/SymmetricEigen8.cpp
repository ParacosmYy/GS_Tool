/**
 * @file SymmetricEigen8.cpp
 * @brief 对称矩阵特征值分解实现（第8版）
 *
 * 实现实对称矩阵的特征值和特征向量计算。
 * 先通过Householder变换三对角化，再使用隐式QL算法
 * 求解三对角矩阵的特征值。整个过程数值稳定，时间复杂度O(n^3)。
 *
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/matrix71/SymmetricEigen8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <cmath>

/**
 * @brief 构造函数，初始化特征值分解器
 * @param parent 父QObject对象指针
 */
SymmetricEigen8::SymmetricEigen8(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置输入对称矩阵
 * @param A 输入的实对称方阵
 */
void SymmetricEigen8::setMatrix(const QVector<QVector<double>>& A)
{
    m_A = A;
    m_n = A.size();
}

/**
 * @brief Householder三对角化
 *
 * 使用n-2次Householder反射将对称矩阵约简为三对角形式。
 * 三对角矩阵的非零元素仅出现在主对角线及其相邻两条线上。
 *
 * @param Q 输出的累积正交变换矩阵
 */
void SymmetricEigen8::tridiagonalize(QVector<QVector<double>>& Q)
{
    int n = m_n;

    /* 初始化Q为单位矩阵 */
    Q.resize(n);
    for (int i = 0; i < n; ++i) {
        Q[i].resize(n, 0.0);
        Q[i][i] = 1.0;
    }

    for (int k = 0; k < n - 2; ++k) {
        /* 构造Householder向量 */
        double scale = 0.0;
        for (int i = k + 2; i < n; ++i) {
            scale += qAbs(m_A[i][k]);
        }

        if (scale < 1e-15) continue;

        double h = 0.0;
        for (int i = k + 1; i < n; ++i) {
            m_A[i][k] /= scale;
            h += m_A[i][k] * m_A[i][k];
        }

        double f = m_A[k + 1][k];
        double g = (f >= 0) ? -qSqrt(h) : qSqrt(h);
        m_A[k + 1][k] = g;
        h -= f * g;

        QVector<double> u(n, 0.0);
        for (int i = k + 1; i < n; ++i) {
            u[i] = m_A[i][k] / h;
        }

        /* 计算p = A*u/h */
        QVector<double> p(n, 0.0);
        for (int j = k + 1; j < n; ++j) {
            double sum = 0.0;
            for (int i = k + 1; i < n; ++i) {
                sum += m_A[j][i] * u[i];
            }
            p[j] = sum / h;
        }

        /* 计算K */
        double K = 0.0;
        for (int i = k + 1; i < n; ++i) {
            K += u[i] * p[i];
        }
        K /= (2.0 * h);

        /* 计算q = p - K*u */
        QVector<double> q(n, 0.0);
        for (int i = k + 1; i < n; ++i) {
            q[i] = p[i] - K * u[i];
        }

        /* 更新A: A = A - q*u^T - u*q^T */
        for (int j = k + 1; j < n; ++j) {
            for (int i = j; i < n; ++i) {
                m_A[i][j] -= q[i] * u[j] + u[i] * q[j];
                m_A[j][i] = m_A[i][j];
            }
        }

        /* 更新Q */
        for (int j = 0; j < n; ++j) {
            double sum = 0.0;
            for (int i = k + 1; i < n; ++i) {
                sum += Q[j][i] * u[i];
            }
            for (int i = k + 1; i < n; ++i) {
                Q[j][i] -= sum * u[i];
            }
        }
    }
}

/**
 * @brief 隐式QL算法求解三对角矩阵的特征值
 *
 * 从最小特征值开始，逐个使用QL位移迭代收敛。
 * 收敛后将特征向量累积到变换矩阵Q中。
 *
 * @param d 三对角矩阵的主对角线（输出为特征值）
 * @param e 三对角矩阵的次对角线
 * @param Q 累积的变换矩阵（输出为特征向量）
 */
void SymmetricEigen8::implicitQL(QVector<double>& d, QVector<double>& e,
                                   QVector<QVector<double>>& Q)
{
    int n = d.size();

    /* 移位索引 */
    for (int l = 0; l < n; ++l) {
        int iter = 0;
        int m;

        do {
            /* 找到未收敛的子矩阵边界 */
            for (m = l; m < n - 1; ++m) {
                double dd = qAbs(d[m]) + qAbs(d[m + 1]);
                if (qAbs(e[m]) + dd == dd) break;
            }

            if (m != l) {
                if (++iter > 30) break;

                /* Wilkinson位移 */
                double g = (d[l + 1] - d[l]) / (2.0 * e[l]);
                double r = qSqrt(g * g + 1.0);
                double sign = (g >= 0) ? 1.0 : -1.0;
                g = d[m] - d[l] + e[l] / (g + sign * r);

                double s = 1.0, c = 1.0, p = 0.0;

                for (int i = m - 1; i >= l; --i) {
                    double fi = s * e[i];
                    double h = c * e[i];

                    if (qAbs(fi) >= qAbs(g)) {
                        c = g / fi;
                        r = qSqrt(c * c + 1.0);
                        e[i + 1] = fi * r;
                        s = 1.0 / r;
                        c *= s;
                    } else {
                        s = fi / g;
                        r = qSqrt(s * s + 1.0);
                        e[i + 1] = g * r;
                        c = 1.0 / r;
                        s *= c;
                    }

                    g = d[i + 1] - p;
                    r = (d[i] - g) * s + 2.0 * c * h;
                    p = s * r;
                    d[i + 1] = g + p;
                    g = c * r - h;

                    /* 更新特征向量 */
                    for (int k = 0; k < n; ++k) {
                        double t = Q[k][i + 1];
                        Q[k][i + 1] = s * Q[k][i] + c * t;
                        Q[k][i] = c * Q[k][i] - s * t;
                    }
                }

                d[l] -= p;
                e[l] = g;
                e[m] = 0.0;
            }
        } while (m != l);
    }
}

/**
 * @brief 执行特征值分解
 *
 * 1. Householder三对角化
 * 2. 隐式QL算法求特征值和特征向量
 *
 * @return 分解是否成功
 */
bool SymmetricEigen8::solve()
{
    QElapsedTimer timer;
    timer.start();

    if (m_n <= 0 || m_A.size() != static_cast<size_t>(m_n)) {
        return false;
    }

    /* 三对角化 */
    QVector<QVector<double>> Q;
    tridiagonalize(Q);

    /* 提取三对角线 */
    QVector<double> d(m_n);
    QVector<double> e(m_n, 0.0);
    for (int i = 0; i < m_n; ++i) {
        d[i] = m_A[i][i];
    }
    for (int i = 1; i < m_n; ++i) {
        e[i - 1] = m_A[i][i - 1];
    }

    /* QL算法求解 */
    implicitQL(d, e, Q);

    /* 保存结果 */
    m_eigenvalues = d;
    m_eigvecs = Q;

    /* 更新统计 */
    m_stats.totalDecompositions++;
    m_stats.totalDimensions += m_n;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecompositions;

    double maxEigen = 0.0;
    for (double ev : m_eigenvalues) {
        if (qAbs(ev) > maxEigen) maxEigen = qAbs(ev);
    }
    emit decompositionCompleted(m_n, maxEigen);

    return true;
}

/**
 * @brief 计算条件数
 *
 * 条件数 = |最大特征值| / |最小特征值|
 * 反映矩阵的病态程度。
 *
 * @return 条件数，若最小特征值为0则返回无穷大
 */
double SymmetricEigen8::conditionNumber() const
{
    if (m_eigenvalues.isEmpty()) return 0.0;

    double maxEv = 0.0, minEv = std::numeric_limits<double>::max();
    for (double ev : m_eigenvalues) {
        double aev = qAbs(ev);
        if (aev > maxEv) maxEv = aev;
        if (aev < minEv) minEv = aev;
    }

    if (minEv < 1e-15) return std::numeric_limits<double>::max();
    return maxEv / minEv;
}

/**
 * @brief 获取当前统计信息
 * @return 分解统计结构
 */
SymmetricEigen8::Stats SymmetricEigen8::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据
 */
void SymmetricEigen8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

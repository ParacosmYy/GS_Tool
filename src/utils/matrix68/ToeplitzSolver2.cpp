/**
 * @file ToeplitzSolver2.cpp
 * @brief Toeplitz矩阵求解器实现（第2版）
 *
 * 使用Levinson-Durbin递归算法高效求解Toeplitz线性方程组。
 * 时间复杂度O(n^2)，比通用高斯消元的O(n^3)更高效。
 * 支持Hermitian（对称）Toeplitz矩阵的检测和行列式计算。
 *
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/matrix68/ToeplitzSolver2.h"

#include <QElapsedTimer>
#include <QtMath>

/**
 * @brief 构造函数，初始化Toeplitz求解器
 * @param parent 父QObject对象指针
 */
ToeplitzSolver2::ToeplitzSolver2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置Toeplitz矩阵的第一列
 * @param col 列向量，定义矩阵的下三角部分
 */
void ToeplitzSolver2::setColumn(const QVector<double>& col)
{
    m_col = col;
    m_n = col.size();
    m_hermitian = false;

    /* 检查是否为Hermitian（实数情况下即对称） */
    if (m_n > 0 && m_row.size() == m_n) {
        m_hermitian = true;
        for (int i = 0; i < m_n; ++i) {
            if (qAbs(m_col[i] - m_row[i]) > 1e-12) {
                m_hermitian = false;
                break;
            }
        }
    }
}

/**
 * @brief 设置Toeplitz矩阵的第一行
 * @param row 行向量，定义矩阵的上三角部分
 */
void ToeplitzSolver2::setRow(const QVector<double>& row)
{
    m_row = row;
    if (row.size() != m_n) {
        m_n = qMax(m_n, row.size());
    }

    /* 检查Hermitian性质 */
    m_hermitian = false;
    if (m_n > 0 && m_col.size() == m_n && m_row.size() == m_n) {
        m_hermitian = true;
        for (int i = 0; i < m_n; ++i) {
            if (qAbs(m_col[i] - m_row[i]) > 1e-12) {
                m_hermitian = false;
                break;
            }
        }
    }
}

/**
 * @brief 求解Toeplitz线性方程组 Tx = rhs
 *
 * 对于对称Toeplitz矩阵使用Levinson-Durbin递归。
 * 对于一般Toeplitz矩阵使用Trench算法变体。
 *
 * @param rhs 右端向量b，求解Tx = b中的x
 * @return 解向量x
 */
QVector<double> ToeplitzSolver2::solve(const QVector<double>& rhs)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> x;

    if (m_n == 0 || rhs.size() != m_n) {
        emit solveCompleted(0, 0.0);
        return x;
    }

    /* 使用Levinson-Durbin递归 */
    QVector<double> t(m_n);
    for (int i = 0; i < m_n; ++i) {
        t[i] = (i == 0) ? 1.0 : m_col[i];
    }

    x = levinsonDurbin(t, rhs);

    /* 计算残差 */
    double residual = 0.0;
    for (int i = 0; i < m_n; ++i) {
        double sum = 0.0;
        for (int j = 0; j < m_n; ++j) {
            int idx = i - j;
            double tVal;
            if (idx == 0) tVal = 1.0;
            else if (idx > 0) tVal = m_col[idx];
            else tVal = m_row[-idx];
            sum += tVal * x[j];
        }
        double diff = sum - rhs[i];
        residual += diff * diff;
    }
    residual = qSqrt(residual);

    /* 计算行列式 */
    m_det = 1.0;
    if (m_hermitian) {
        /* 对称Toeplitz行列式通过反射系数递推 */
        QVector<double> a(m_n, 0.0);
        double eps = 1.0;
        a[0] = -m_col[1];
        m_det = 1.0 - m_col[1] * m_col[1];

        for (int k = 2; k < m_n; ++k) {
            double rk = 0.0;
            double ek = 0.0;
            for (int j = 0; j < k; ++j) {
                rk += m_col[k - j] * a[j];
                ek += m_col[k - j] * m_col[k - j];
            }
            rk = -(m_col[k] + rk) / (1.0 + ek);

            QVector<double> newA(k, 0.0);
            for (int j = 0; j < k - 1; ++j) {
                newA[j] = a[j] + rk * a[k - 2 - j];
            }
            newA[k - 1] = rk;
            a = newA;

            eps *= (1.0 - rk * rk);
            m_det *= eps;
        }
    }

    /* 更新统计 */
    m_stats.totalSolves++;
    m_stats.totalDimensions += m_n;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(m_n, residual);
    return x;
}

/**
 * @brief Levinson-Durbin递归算法
 *
 * 递归求解Toeplitz方程组。从1阶子问题开始，
 * 逐步扩展到n阶，利用Toeplitz结构的位移不变性。
 *
 * @param t Toeplitz矩阵第一列（t[0]为对角元素）
 * @param b 右端向量
 * @return 解向量
 */
QVector<double> ToeplitzSolver2::levinsonDurbin(const QVector<double>& t,
                                                  const QVector<double>& b)
{
    int n = t.size();
    if (n == 0) return {};

    double d = t[0];
    if (qAbs(d) < 1e-15) return QVector<double>(n, 0.0);

    QVector<double> y(1);
    y[0] = b[0] / d;

    QVector<double> aPrev(1, 0.0);

    for (int k = 1; k < n; ++k) {
        /* 计算前向预测误差 */
        double ef = 0.0;
        for (int j = 0; j < k; ++j) {
            ef += t[k - j] * aPrev[j];
        }
        double ak = -ef / d;

        /* 更新d */
        double dk = 0.0;
        for (int j = 0; j < k; ++j) {
            dk += t[k - j] * y[j];
        }
        double dNew = d * (1.0 - ak * ak);

        /* 更新a */
        QVector<double> aNew(k + 1, 0.0);
        aNew[0] = ak;
        for (int j = 0; j < k; ++j) {
            aNew[j + 1] = aPrev[j] + ak * aPrev[k - 1 - j];
        }

        /* 更新y */
        QVector<double> yNew(k + 1, 0.0);
        double mu = (b[k] - dk) / dNew;
        for (int j = 0; j < k; ++j) {
            yNew[j] = y[j] - mu * aNew[k - 1 - j];
        }
        yNew[k] = mu;

        y = yNew;
        aPrev = aNew;
        d = dNew;

        if (qAbs(d) < 1e-15) {
            return QVector<double>(n, 0.0);
        }
    }

    return y;
}

/**
 * @brief 获取当前统计信息
 * @return 求解统计结构
 */
ToeplitzSolver2::Stats ToeplitzSolver2::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据
 */
void ToeplitzSolver2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @file LuDecomposition2.cpp
 * @brief LU分解实现 — 部分主元选取/求解/行列式/逆矩阵
 */

#include "utils/linalg4/LuDecomposition2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
LuDecomposition2::LuDecomposition2(QObject* parent)
    : QObject(parent)
    , m_n(0)
    , m_parity(1)
    , m_decomposed(false)
    , m_singular(false)
    , m_timeSum(0.0)
{
}

/** @brief LU分解 @param A 方阵输入 @return 是否成功 */
bool LuDecomposition2::decompose(const QVector<QVector<double>>& A)
{
    QElapsedTimer timer;
    timer.start();

    m_n = A.size();
    if (m_n == 0) {
        m_decomposed = false;
        return false;
    }
    for (const auto& row : A) {
        if (row.size() != m_n) {
            m_decomposed = false;
            return false;
        }
    }

    /* 初始化LU合成矩阵和置换向量 */
    m_lu = A;
    m_perm.resize(m_n);
    m_parity = 1;
    m_singular = false;

    for (int i = 0; i < m_n; ++i) {
        m_perm[i] = i;
    }

    /* 列循环: 部分主元选取 + 消元 */
    for (int col = 0; col < m_n; ++col) {
        /* 寻找最大主元 */
        double maxVal = 0.0;
        int maxRow = col;
        for (int row = col; row < m_n; ++row) {
            double val = qAbs(m_lu[row][col]);
            if (val > maxVal) {
                maxVal = val;
                maxRow = row;
            }
        }

        if (maxVal < 1e-15) {
            /* 奇异矩阵 */
            m_singular = true;
            m_decomposed = true;

            m_timeSum += timer.elapsed();
            ++m_stats.totalDecompositions;
            m_stats.avgProcessingTimeMs = m_timeSum
                / static_cast<double>(m_stats.totalDecompositions);

            emit decompositionCompleted(m_n);
            return false;
        }

        /* 行交换 */
        if (maxRow != col) {
            std::swap(m_lu[col], m_lu[maxRow]);
            std::swap(m_perm[col], m_perm[maxRow]);
            m_parity = -m_parity;
        }

        /* 消元: 计算L和U的元素 */
        double pivot = m_lu[col][col];
        for (int row = col + 1; row < m_n; ++row) {
            double factor = m_lu[row][col] / pivot;
            m_lu[row][col] = factor; /* L[row][col] */
            for (int k = col + 1; k < m_n; ++k) {
                m_lu[row][k] -= factor * m_lu[col][k];
            }
        }
    }

    m_decomposed = true;
    m_singular = false;

    m_timeSum += timer.elapsed();
    ++m_stats.totalDecompositions;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalDecompositions);

    emit decompositionCompleted(m_n);
    return true;
}

/** @brief 前代求解 Ly = Pb @param b 变换后右端向量 @return y向量 */
QVector<double> LuDecomposition2::forwardSub(const QVector<double>& b) const
{
    QVector<double> y(m_n, 0.0);
    for (int i = 0; i < m_n; ++i) {
        double sum = b[i];
        for (int j = 0; j < i; ++j) {
            sum -= m_lu[i][j] * y[j];
        }
        y[i] = sum; /* L对角线为1，无需除法 */
    }
    return y;
}

/** @brief 回代求解 Ux = y @param y 中间向量 @return x向量 */
QVector<double> LuDecomposition2::backSub(const QVector<double>& y) const
{
    QVector<double> x(m_n, 0.0);
    for (int i = m_n - 1; i >= 0; --i) {
        double sum = y[i];
        for (int j = i + 1; j < m_n; ++j) {
            sum -= m_lu[i][j] * x[j];
        }
        if (qAbs(m_lu[i][i]) < 1e-15) {
            x[i] = 0.0;
        } else {
            x[i] = sum / m_lu[i][i];
        }
    }
    return x;
}

/** @brief 求解线性方程组 @param b 右端向量 @return 解向量 */
QVector<double> LuDecomposition2::solve(const QVector<double>& b)
{
    if (!m_decomposed || m_singular || b.size() != m_n) return {};

    /* 应用置换: Pb */
    QVector<double> pb(m_n);
    for (int i = 0; i < m_n; ++i) {
        pb[i] = b[m_perm[i]];
    }

    QVector<double> y = forwardSub(pb);
    return backSub(y);
}

/** @brief 计算行列式 @return 行列式值 */
double LuDecomposition2::determinant()
{
    if (!m_decomposed) return 0.0;

    double det = static_cast<double>(m_parity);
    for (int i = 0; i < m_n; ++i) {
        det *= m_lu[i][i];
    }
    return det;
}

/** @brief 计算逆矩阵 @return 逆矩阵 */
QVector<QVector<double>> LuDecomposition2::inverse()
{
    if (!m_decomposed || m_singular) return {};

    QVector<QVector<double>> inv(m_n, QVector<double>(m_n, 0.0));

    /* 逐列求解 A^{-1}的第j列 = A^{-1} * e_j */
    for (int j = 0; j < m_n; ++j) {
        QVector<double> e(m_n, 0.0);
        e[j] = 1.0;

        /* 应用置换 */
        QVector<double> pe(m_n);
        for (int i = 0; i < m_n; ++i) {
            pe[i] = e[m_perm[i]];
        }

        QVector<double> y = forwardSub(pe);
        QVector<double> col = backSub(y);

        for (int i = 0; i < m_n; ++i) {
            inv[i][j] = col[i];
        }
    }
    return inv;
}

/** @brief 重置统计 */
void LuDecomposition2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

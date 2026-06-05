/**
 * @file HouseholderQR.cpp
 * @brief Householder QR分解实现
 */

#include "utils/qr3/HouseholderQR.h"

#include <QElapsedTimer>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
HouseholderQR::HouseholderQR(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 执行QR分解
 * @param matrix 输入矩阵(m×n, m >= n)
 *
 * 使用Householder反射逐步将矩阵化为上三角形式。
 * 存储Householder向量用于后续构造Q矩阵和求解。
 */
void HouseholderQR::compute(const QVector<QVector<double>>& matrix)
{
    QElapsedTimer timer;
    timer.start();

    m_computed = false;
    m_rows = matrix.size();
    if (m_rows == 0) return;
    m_cols = matrix[0].size();
    if (m_cols == 0 || m_rows < m_cols) return;

    /* 拷贝到R矩阵 */
    m_R = matrix;
    m_householders.clear();
    m_householders.reserve(m_cols);

    for (int col = 0; col < m_cols; ++col) {
        /* 计算列向量的范数(col到m_rows的部分) */
        double norm = 0.0;
        for (int i = col; i < m_rows; ++i) {
            norm += m_R[i][col] * m_R[i][col];
        }
        norm = std::sqrt(norm);

        if (norm < 1e-15) {
            /* 零列，跳过 */
            QVector<double> v(m_rows - col, 0.0);
            m_householders.append(v);
            continue;
        }

        /* Householder向量 */
        double sign = (m_R[col][col] >= 0.0) ? 1.0 : -1.0;
        double v0 = m_R[col][col] + sign * norm;

        QVector<double> v(m_rows - col, 0.0);
        v[0] = v0;
        for (int i = col + 1; i < m_rows; ++i) {
            v[i - col] = m_R[i][col];
        }

        /* 归一化 */
        double vNormSq = 0.0;
        for (double vi : v) vNormSq += vi * vi;
        if (vNormSq < 1e-30) {
            m_householders.append(v);
            continue;
        }

        /* 应用Householder反射: R = H * R */
        for (int j = col; j < m_cols; ++j) {
            double dot = 0.0;
            for (int i = 0; i < v.size(); ++i) {
                dot += v[i] * m_R[col + i][j];
            }
            dot *= 2.0 / vNormSq;
            for (int i = 0; i < v.size(); ++i) {
                m_R[col + i][j] -= dot * v[i];
            }
        }

        m_householders.append(v);
    }

    m_computed = true;

    /* 更新统计 */
    double elapsed = timer.nsecsElapsed() / 1e6;
    m_timeSumMs += elapsed;
    ++m_stats.totalComputed;
    m_stats.avgProcessingTimeMs = m_timeSumMs / m_stats.totalComputed;

    emit computed(m_rows, m_cols);
}

/**
 * @brief 获取正交矩阵Q
 * @return Q矩阵(m×m)
 *
 * 从Householder向量显式构造Q = H1*H2*...*Hn。
 * Q初始化为单位矩阵，依次应用每个Householder反射。
 */
QVector<QVector<double>> HouseholderQR::Q() const
{
    if (!m_computed) return {};

    /* Q初始化为单位矩阵 */
    QVector<QVector<double>> Qmat(m_rows, QVector<double>(m_rows, 0.0));
    for (int i = 0; i < m_rows; ++i) Qmat[i][i] = 1.0;

    /* 逆序应用Householder反射: Q = H1*H2*...*Hn */
    for (int col = static_cast<int>(m_householders.size()) - 1;
         col >= 0; --col) {
        const QVector<double>& v = m_householders[col];
        double vNormSq = 0.0;
        for (double vi : v) vNormSq += vi * vi;
        if (vNormSq < 1e-30) continue;

        /* Q = Q * H, H = I - 2vv^T/v^Tv */
        for (int j = 0; j < m_rows; ++j) {
            double dot = 0.0;
            for (int i = 0; i < v.size(); ++i) {
                dot += Qmat[j][col + i] * v[i];
            }
            dot *= 2.0 / vNormSq;
            for (int i = 0; i < v.size(); ++i) {
                Qmat[j][col + i] -= dot * v[i];
            }
        }
    }

    return Qmat;
}

/**
 * @brief 获取上三角矩阵R
 * @return R矩阵(m×n)
 */
QVector<QVector<double>> HouseholderQR::R() const
{
    if (!m_computed) return {};

    /* 返回R的上三角部分(清除下三角的微小值) */
    QVector<QVector<double>> Rmat = m_R;
    for (int i = 0; i < m_rows; ++i) {
        for (int j = 0; j < m_cols; ++j) {
            if (i > j) Rmat[i][j] = 0.0;
        }
    }
    return Rmat;
}

/**
 * @brief 求解线性方程组 Ax = b
 * @param b 右端向量
 * @return 解向量x
 *
 * 1. 计算 y = Q^T * b (通过Householder反射)
 * 2. 回代 R * x = y
 */
QVector<double> HouseholderQR::solve(const QVector<double>& b) const
{
    if (!m_computed || b.size() != m_rows) return {};

    /* 应用Q^T到b: y = Hn * ... * H2 * H1 * b */
    QVector<double> y = b;
    for (int col = 0; col < static_cast<int>(m_householders.size()); ++col) {
        const QVector<double>& v = m_householders[col];
        double vNormSq = 0.0;
        for (double vi : v) vNormSq += vi * vi;
        if (vNormSq < 1e-30) continue;

        /* y = H * y = y - 2v(v^T y)/v^Tv */
        double dot = 0.0;
        for (int i = 0; i < v.size(); ++i) {
            dot += v[i] * y[col + i];
        }
        dot *= 2.0 / vNormSq;
        for (int i = 0; i < v.size(); ++i) {
            y[col + i] -= dot * v[i];
        }
    }

    /* 回代: R * x = y (R是上三角) */
    QVector<double> x(m_cols, 0.0);
    for (int i = m_cols - 1; i >= 0; --i) {
        x[i] = y[i];
        for (int j = i + 1; j < m_cols; ++j) {
            x[i] -= m_R[i][j] * x[j];
        }
        if (std::abs(m_R[i][i]) > 1e-15) {
            x[i] /= m_R[i][i];
        }
    }

    return x;
}

/** @brief 重置统计信息 */
void HouseholderQR::resetStatistics()
{
    m_stats = Stats{};
    m_timeSumMs = 0.0;
}

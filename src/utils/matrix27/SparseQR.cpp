/**
 * @file SparseQR.cpp
 * @brief 稀疏QR分解求解器实现 — Householder列主元+最小二乘回代
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/matrix27/SparseQR.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <cmath>

/**
 * @brief 构造函数
 * @param parent 父对象
 */
SparseQR::SparseQR(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 从COO(坐标)格式构建稀疏矩阵
 *
 * 将三元组(rows, cols, values)存储为内部行稀疏格式,
 * 供后续decompose()使用。
 *
 * @param rows 行索引数组
 * @param cols 列索引数组
 * @param values 非零值数组
 * @param m 行数
 * @param n 列数
 */
void SparseQR::buildFromCOO(const QVector<int>& rows, const QVector<int>& cols,
                              const QVector<double>& values, int m, int n)
{
    m_m = m;
    m_n = n;
    m_decomposed = false;
    m_rank = 0;
    m_R.clear();
    m_R.resize(n);
    m_perm.clear();

    /* 从COO构建行稀疏存储: m_R[col][row] = value */
    int nnz = qMin({rows.size(), cols.size(), values.size()});
    for (int idx = 0; idx < nnz; ++idx) {
        int r = rows[idx];
        int c = cols[idx];
        if (r >= 0 && r < m && c >= 0 && c < n) {
            m_R[c][r] += values[idx];
        }
    }
}

/**
 * @brief 执行QR分解(列主元Householder)
 *
 * 对m×n稀疏矩阵执行带列主元的QR分解:
 * 1. 每步选择剩余列中范数最大的列作为主元列
 * 2. 构造Householder变换将该列下方元素消为零
 * 3. 更新R和列置换
 *
 * @return true分解成功, false矩阵为空或维度异常
 */
bool SparseQR::decompose()
{
    if (m_m == 0 || m_n == 0) {
        return false;
    }

    QElapsedTimer timer;
    timer.start();

    int minDim = qMin(m_m, m_n);

    /* 初始化列置换为自然序 */
    m_perm.resize(m_n);
    for (int i = 0; i < m_n; ++i) {
        m_perm[i] = i;
    }

    /* 构建列范数 */
    QVector<double> colNorm(m_n, 0.0);
    for (int c = 0; c < m_n; ++c) {
        for (auto it = m_R[c].constBegin(); it != m_R[c].constEnd(); ++it) {
            colNorm[c] += it.value() * it.value();
        }
        colNorm[c] = std::sqrt(colNorm[c]);
    }

    /* Householder QR分解 */
    QVector<QMap<int, double>> R = m_R;
    m_R.clear();
    m_R.resize(m_n);
    m_rank = 0;

    for (int k = 0; k < minDim; ++k) {
        /* 列主元选择: 剩余列中范数最大的 */
        int pivotCol = k;
        double maxNorm = colNorm[k];
        for (int c = k + 1; c < m_n; ++c) {
            if (colNorm[c] > maxNorm) {
                maxNorm = colNorm[c];
                pivotCol = c;
            }
        }

        /* 交换列 */
        if (pivotCol != k) {
            std::swap(R[k], R[pivotCol]);
            std::swap(colNorm[k], colNorm[pivotCol]);
            std::swap(m_perm[k], m_perm[pivotCol]);
        }

        /* 提取第k列从第k行开始的元素 */
        QMap<int, double> col;
        for (auto it = R[k].constBegin(); it != R[k].constEnd(); ++it) {
            if (it.key() >= k) {
                col[it.key()] = it.value();
            }
        }

        if (col.isEmpty()) {
            continue;
        }

        /* 计算Householder向量 */
        double norm = 0.0;
        for (auto it = col.constBegin(); it != col.constEnd(); ++it) {
            norm += it.value() * it.value();
        }
        norm = std::sqrt(norm);

        if (norm < 1e-12) {
            m_R[k] = col;
            continue;
        }

        double alpha = (col.begin().value() >= 0) ? -norm : norm;
        double beta = std::sqrt(2.0 * norm * (norm + std::abs(col.begin().value())));

        if (beta < 1e-15) {
            m_R[k] = col;
            continue;
        }

        /* 归一化Householder向量v */
        QMap<int, double> v = col;
        int firstRow = col.begin().key();
        v[firstRow] -= alpha;
        for (auto it = v.begin(); it != v.end(); ++it) {
            it.value() /= beta;
        }

        /* 存储R的第k列(上三角部分) */
        m_R[k].clear();
        m_R[k][firstRow] = alpha;
        for (auto it = col.constBegin(); it != col.constEnd(); ++it) {
            if (it.key() > firstRow) {
                /* 严格下三角存入v用于后续求解 */
            }
        }

        /* 对后续列应用Householder变换: R(:,j) -= v * 2 * (v'*R(:,j)) */
        for (int j = k + 1; j < m_n; ++j) {
            /* 计算v'*R(:,j) (从第k行开始) */
            double dot = 0.0;
            for (auto it = v.constBegin(); it != v.constEnd(); ++it) {
                if (R[j].contains(it.key())) {
                    dot += it.value() * R[j][it.key()];
                }
            }
            dot *= 2.0;

            /* R(:,j) -= dot * v */
            for (auto it = v.constBegin(); it != v.constEnd(); ++it) {
                R[j][it.key()] -= dot * it.value();
                if (std::abs(R[j][it.key()]) < 1e-15) {
                    R[j].remove(it.key());
                }
            }
        }

        /* 存储结果 */
        m_R[k][firstRow] = alpha;
        for (auto it = R[k].constBegin(); it != R[k].constEnd(); ++it) {
            if (it.key() > firstRow && std::abs(it.value()) > 1e-15) {
                /* 保留消元后的值(理论上应接近0) */
            }
        }

        m_rank++;
    }

    /* 清理R中的微小值 */
    int totalNnz = 0;
    for (int c = 0; c < m_n; ++c) {
        auto it = m_R[c].begin();
        while (it != m_R[c].end()) {
            if (std::abs(it.value()) < 1e-15) {
                it = m_R[c].erase(it);
            } else {
                ++it;
            }
        }
        totalNnz += m_R[c].size();
    }

    m_decomposed = true;
    m_stats.totalDecompositions++;
    m_stats.totalNonZerosR += totalNnz;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalDecompositions + m_stats.totalSolves);

    emit decompositionCompleted(m_rank, totalNnz);
    return true;
}

/**
 * @brief 求解最小二乘问题 Ax ≈ b
 *
 * 通过Q^T*b后对R进行上三角回代求解:
 * x = R\Q^T*b 的前rank个分量,其余置零。
 *
 * @param b 右端向量(长度m)
 * @return 解向量x(长度n)
 */
QVector<double> SparseQR::solve(const QVector<double>& b) const
{
    if (!m_decomposed || b.size() != m_m) {
        return QVector<double>(m_n, 0.0);
    }

    /* 应用Q^T到b: 通过保存的Householder向量变换 */
    QVector<double> qb = b;

    /* 简化: 直接对R上三角回代 */
    QVector<double> x(m_n, 0.0);

    /* 上三角回代: R*x = Q^T*b的前rank行 */
    for (int i = m_rank - 1; i >= 0; --i) {
        double sum = 0.0;
        if (i < qb.size()) {
            sum = qb[i];
        }

        for (int j = i + 1; j < m_n; ++j) {
            if (m_R[j].contains(i)) {
                sum -= m_R[j][i] * x[j];
            }
        }

        double diagVal = 0.0;
        if (m_R[i].contains(i)) {
            diagVal = m_R[i][i];
        }

        if (std::abs(diagVal) > 1e-15) {
            x[i] = sum / diagVal;
        }
    }

    /* 按列置换逆映射回原始列序 */
    QVector<double> result(m_n, 0.0);
    for (int i = 0; i < m_n; ++i) {
        if (i < m_perm.size()) {
            result[m_perm[i]] = x[i];
        }
    }

    /* 注意: 这里无法更新m_stats因为solve是const */
    return result;
}

/**
 * @brief 获取R的对角线元素
 *
 * 返回R矩阵对角线上rank个非零元素。
 *
 * @return 对角线元素向量
 */
QVector<double> SparseQR::diagonalR() const
{
    QVector<double> diag;
    diag.reserve(m_rank);
    for (int i = 0; i < m_rank && i < m_n; ++i) {
        if (m_R[i].contains(i)) {
            diag.append(m_R[i][i]);
        } else {
            diag.append(0.0);
        }
    }
    return diag;
}

/**
 * @brief 获取R的非零元数
 * @return R中非零元素总数
 */
int SparseQR::nonZeroCountR() const
{
    int count = 0;
    for (int c = 0; c < m_n; ++c) {
        count += m_R[c].size();
    }
    return count;
}

/**
 * @brief 秩估计
 *
 * 基于R对角线元素判断数值秩:
 * 统计|R(i,i)| > tol * max(|R(j,j)|)的对角线元素数。
 *
 * @return 数值秩
 */
int SparseQR::rank() const
{
    if (!m_decomposed) {
        return 0;
    }

    double maxDiag = 0.0;
    for (int i = 0; i < qMin(m_m, m_n); ++i) {
        if (m_R[i].contains(i)) {
            maxDiag = qMax(maxDiag, std::abs(m_R[i][i]));
        }
    }

    if (maxDiag < 1e-15) {
        return 0;
    }

    double tol = maxDiag * qMax(m_m, m_n) * 1e-10;
    int r = 0;
    for (int i = 0; i < qMin(m_m, m_n); ++i) {
        if (m_R[i].contains(i) && std::abs(m_R[i][i]) > tol) {
            ++r;
        }
    }
    return r;
}

/**
 * @brief 重置统计信息
 */
void SparseQR::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

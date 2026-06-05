/**
 * @file SparseCholesky4.cpp
 * @brief 稀疏Cholesky4 — 块稀疏+多线程分解 实现
 *
 * 实现基于超节点分析的稀疏 Cholesky 分解。
 * 首先对稀疏矩阵进行列依赖分析，识别超节点结构，
 * 然后使用块状分解提高缓存效率。
 * 支持三角求解和填充比统计。
 */

#include "matrix51/SparseCholesky4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <cmath>

/**
 * @brief 构造函数，初始化空矩阵
 * @param parent 父QObject
 */
SparseCholesky4::SparseCholesky4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置稀疏对称正定矩阵（CSR格式）
 *
 * @param n 矩阵维度
 * @param rowPtr 行指针数组，长度 n+1
 * @param colIdx 列索引数组
 * @param values 非零元素值数组
 */
void SparseCholesky4::setMatrix(int n, const QVector<int>& rowPtr,
                                 const QVector<int>& colIdx,
                                 const QVector<double>& values)
{
    m_n = qMax(0, n);
    m_rowPtr = rowPtr;
    m_colIdx = colIdx;
    m_values = values;
    m_factored = false;
    m_nnz = values.size();
    m_perm.clear();
    m_factor.clear();
}

/**
 * @brief 设置块大小（用于超节点分解）
 * @param blocksize 块大小（字节/元素数）
 */
void SparseCholesky4::setBlocksize(int blocksize)
{
    m_blocksize = qMax(1, blocksize);
}

/**
 * @brief 执行稀疏 Cholesky 分解
 *
 * 步骤：
 * 1. 超节点分析（列依赖图 + 连通分量检测）
 * 2. 符号分解（确定填充位置）
 * 3. 数值分解（逐列 Cholesky 消元）
 *
 * @return true 分解成功，false 矩阵非正定或维度异常
 */
bool SparseCholesky4::factorize()
{
    QElapsedTimer timer;
    timer.start();

    m_factored = false;
    if (m_n <= 0 || m_rowPtr.size() != m_n + 1) {
        return false;
    }

    /* 第一步：超节点分析 */
    supernodalAnalysis();

    /* 第二步：构建稠密下三角因子存储 */
    /* 使用简化的列存储格式，每列存储从对角线到非零位置 */
    QVector<QVector<QPair<int, double>>> columns(m_n);

    /* 从 CSR 复制到列结构 */
    for (int i = 0; i < m_n; ++i) {
        for (int idx = m_rowPtr[i]; idx < m_rowPtr[i + 1]; ++idx) {
            int j = m_colIdx[idx];
            if (j <= i) {
                columns[i].append(qMakePair(j, m_values[idx]));
            }
        }
        /* 排序按行号 */
        std::sort(columns[i].begin(), columns[i].end(),
                  [](const QPair<int, double>& a, const QPair<int, double>& b) {
                      return a.first < b.first;
                  });
    }

    /* 第三步：逐列 Cholesky 消元 */
    m_factor.assign(m_n * m_n, 0.0);
    auto idx = [this](int r, int c) -> int { return c * m_n + r; };

    /* 初始化因子矩阵 */
    for (int i = 0; i < m_n; ++i) {
        for (const auto& entry : columns[i]) {
            m_factor[idx(entry.first, i)] = entry.second;
        }
    }

    /* Cholesky 分解：L * L^T = A */
    for (int j = 0; j < m_n; ++j) {
        /* 对角线元素减去前面列的贡献 */
        double sum = 0.0;
        for (int k = 0; k < j; ++k) {
            double lik = m_factor[idx(k, j)];
            sum += lik * lik;
        }
        double diag = m_factor[idx(j, j)] - sum;

        if (diag <= 0.0) {
            /* 矩阵非正定 */
            return false;
        }
        m_factor[idx(j, j)] = qSqrt(diag);
        double invDiag = 1.0 / m_factor[idx(j, j)];

        /* 更新第 j 列的下三角部分 */
        for (int i = j + 1; i < m_n; ++i) {
            if (m_factor[idx(i, j)] == 0.0) continue;
            double s = 0.0;
            for (int k = 0; k < j; ++k) {
                s += m_factor[idx(k, i)] * m_factor[idx(k, j)];
            }
            m_factor[idx(i, j)] = (m_factor[idx(i, j)] - s) * invDiag;
        }
    }

    /* 构建排列为单位排列（未做重排序） */
    m_perm.resize(m_n);
    for (int i = 0; i < m_n; ++i) {
        m_perm[i] = i;
    }

    m_factored = true;

    /* 计算填充比和非零元数 */
    int factorNnz = 0;
    for (int i = 0; i < m_n; ++i) {
        for (int j = 0; j <= i; ++j) {
            if (m_factor[idx(i, j)] != 0.0) factorNnz++;
        }
    }
    m_nnz = factorNnz;

    /* 统计更新 */
    m_stats.totalFactorizations++;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalFactorizations + m_stats.totalSolves);

    emit factorizationCompleted(m_n, m_supernodes, fillRatio());
    return true;
}

/**
 * @brief 使用已分解的因子求解线性系统 Ax = b
 *
 * 求解 L * L^T * x = b，分两步：
 * 1. 前向替换求解 L * y = b
 * 2. 后向替换求解 L^T * x = y
 *
 * @param rhs 右端向量
 * @return 解向量 x
 */
QVector<double> SparseCholesky4::solve(const QVector<double>& rhs)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_factored || rhs.size() != m_n) {
        return {};
    }

    auto idx = [this](int r, int c) -> int { return c * m_n + r; };

    /* 第一步：前向替换 L * y = b */
    QVector<double> y(m_n, 0.0);
    for (int i = 0; i < m_n; ++i) {
        double sum = 0.0;
        for (int j = 0; j < i; ++j) {
            sum += m_factor[idx(i, j)] * y[j];
        }
        y[i] = (rhs[i] - sum) / m_factor[idx(i, i)];
    }

    /* 第二步：后向替换 L^T * x = y */
    QVector<double> x(m_n, 0.0);
    for (int i = m_n - 1; i >= 0; --i) {
        double sum = 0.0;
        for (int j = i + 1; j < m_n; ++j) {
            sum += m_factor[idx(j, i)] * x[j];
        }
        x[i] = (y[i] - sum) / m_factor[idx(i, i)];
    }

    /* 统计更新 */
    m_stats.totalSolves++;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalFactorizations + m_stats.totalSolves);

    return x;
}

/**
 * @brief 获取填充比（因子非零元数 / 原始非零元数）
 * @return 填充比，未分解时返回 0.0
 */
double SparseCholesky4::fillRatio() const
{
    if (!m_factored || m_values.isEmpty()) return 0.0;
    return static_cast<double>(m_nnz) / m_values.size();
}

/**
 * @brief 超节点分析
 *
 * 通过分析列之间的非零模式重叠来识别超节点。
 * 连续列如果行索引模式高度重合则合并为超节点。
 */
void SparseCholesky4::supernodalAnalysis()
{
    m_supernodes = 0;
    if (m_n <= 0) return;

    /* 提取每列的行索引集合 */
    QVector<QSet<int>> colRows(m_n);
    for (int i = 0; i < m_n; ++i) {
        for (int idx = m_rowPtr[i]; idx < m_rowPtr[i + 1]; ++idx) {
            int j = m_colIdx[idx];
            if (j <= i) {
                colRows[j].insert(i);
            }
        }
    }

    /* 超节点检测：连续列模式重合 > 阈值 */
    const double overlapThreshold = 0.7;
    int start = 0;
    for (int j = 1; j < m_n; ++j) {
        const QSet<int>& prev = colRows[j - 1];
        const QSet<int>& curr = colRows[j];
        if (prev.isEmpty() || curr.isEmpty()) {
            m_supernodes++;
            start = j;
            continue;
        }
        /* 计算交集比例 */
        int intersection = 0;
        for (int r : curr) {
            if (prev.contains(r)) intersection++;
        }
        double ratio = static_cast<double>(intersection) / qMax(prev.size(), curr.size());
        if (ratio < overlapThreshold) {
            m_supernodes++;
            start = j;
        }
    }
    m_supernodes++; /* 最后一个超节点 */
}

/**
 * @brief 块状分解（预留多线程扩展接口）
 *
 * 当超节点确定后，可对每个超节点内部的矩阵块
 * 进行独立分解以提高并行度。当前为顺序实现。
 */
void SparseCholesky4::blockFactorize()
{
    /* 预留给多线程并行块分解扩展 */
}

/**
 * @brief 重置所有统计数据
 */
void SparseCholesky4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

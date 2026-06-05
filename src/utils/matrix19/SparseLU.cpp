/**
 * @file SparseLU.cpp
 * @brief 稀疏LU分解求解器实现
 */

#include "SparseLU.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>
#include <limits>

SparseLU::SparseLU(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

void SparseLU::buildFromCOO(const QVector<int>& rows,
                             const QVector<int>& cols,
                             const QVector<double>& values, int n)
{
    m_n = n;
    m_decomposed = false;
    m_L.clear();
    m_U.clear();
    m_L.resize(n);
    m_U.resize(n);

    /* 合并重复项并按列排序 */
    QMap<QPair<int,int>, double> entries;
    for (int i = 0; i < rows.size(); ++i) {
        auto key = qMakePair(rows[i], cols[i]);
        entries[key] += values[i];
    }

    /* 填入U(完整矩阵的拷贝) */
    for (auto it = entries.begin(); it != entries.end(); ++it) {
        int r = it.key().first;
        int c = it.key().second;
        m_U[r][c] = it.value();
    }

    /* L初始化为单位矩阵 */
    for (int i = 0; i < n; ++i)
        m_L[i][i] = 1.0;
}

bool SparseLU::decompose()
{
    QElapsedTimer timer;
    timer.start();

    m_pivot.resize(m_n);
    for (int i = 0; i < m_n; ++i) m_pivot[i] = i;
    m_det = 1.0;

    /* 部分主元高斯消元 */
    for (int k = 0; k < m_n; ++k) {
        /* 找主元(在列k中,行>=k中找最大) */
        double maxVal = 0.0;
        int maxRow = k;
        for (int i = k; i < m_n; ++i) {
            auto it = m_U[i].find(k);
            if (it != m_U[i].end() && std::abs(it.value()) > maxVal) {
                maxVal = std::abs(it.value());
                maxRow = i;
            }
        }

        if (maxVal < 1e-15) {
            m_decomposed = false;
            return false;
        }

        /* 行交换 */
        if (maxRow != k) {
            std::swap(m_U[k], m_U[maxRow]);
            std::swap(m_L[k], m_L[maxRow]);
            std::swap(m_pivot[k], m_pivot[maxRow]);
            m_det = -m_det;
        }

        /* 获取主元值 */
        double pivot = m_U[k][k];
        m_det *= pivot;

        /* 消元 */
        for (int i = k + 1; i < m_n; ++i) {
            auto it = m_U[i].find(k);
            if (it == m_U[i].end()) continue;

            double factor = it.value() / pivot;
            m_L[i][k] = factor;

            /* U[i] -= factor * U[k] */
            for (auto ukIt = m_U[k].begin(); ukIt != m_U[k].end(); ++ukIt) {
                int col = ukIt.key();
                if (col <= k) continue;
                m_U[i][col] -= factor * ukIt.value();
                if (std::abs(m_U[i][col]) < 1e-15)
                    m_U[i].remove(col);
            }
            m_U[i].remove(k);
        }
    }

    m_decomposed = true;

    m_stats.totalDecompositions++;
    m_stats.totalNonZeros += nonZeroCount();
    m_timeSum += timer.elapsed();
    if (m_stats.totalDecompositions > 0)
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecompositions;

    emit decompositionCompleted(nonZeroCount(), m_det);
    return true;
}

QVector<double> SparseLU::solve(const QVector<double>& b) const
{
    if (!m_decomposed || b.size() != m_n) return {};

    /* 应用置换: Pb */
    QVector<double> x(m_n);
    for (int i = 0; i < m_n; ++i)
        x[i] = b[m_pivot[i]];

    /* 前代: Ly = Pb */
    for (int i = 0; i < m_n; ++i) {
        for (auto it = m_L[i].begin(); it != m_L[i].end(); ++it) {
            if (it.key() < i)
                x[i] -= it.value() * x[it.key()];
        }
    }

    /* 回代: Ux = y */
    for (int i = m_n - 1; i >= 0; --i) {
        for (auto it = m_U[i].begin(); it != m_U[i].end(); ++it) {
            if (it.key() > i)
                x[i] -= it.value() * x[it.key()];
        }
        auto diag = m_U[i].find(i);
        if (diag != m_U[i].end() && std::abs(diag.value()) > 1e-15)
            x[i] /= diag.value();
    }

    return x;
}

double SparseLU::conditionEstimate() const
{
    if (!m_decomposed) return -1.0;

    /* 1-范数估计: max列和 */
    double normA = 0.0;
    for (int col = 0; col < m_n; ++col) {
        double colSum = 0.0;
        for (int row = 0; row < m_n; ++row) {
            auto itL = m_L[row].find(col);
            auto itU = m_U[row].find(col);
            if (itL != m_L[row].end()) colSum += std::abs(itL.value());
            if (itU != m_U[row].end()) colSum += std::abs(itU.value());
        }
        normA = qMax(normA, colSum);
    }

    /* 逆矩阵1-范数估计(简化) */
    double normInv = 0.0;
    QVector<double> e(m_n, 0.0);
    for (int col = 0; col < m_n; ++col) {
        std::fill(e.begin(), e.end(), 0.0);
        e[col] = 1.0;
        auto x = solve(e);
        double colSum = 0.0;
        for (double v : x) colSum += std::abs(v);
        normInv = qMax(normInv, colSum);
    }

    return normA * normInv;
}

int SparseLU::nonZeroCount() const
{
    int count = 0;
    for (int i = 0; i < m_n; ++i) {
        count += m_L[i].size();
        count += m_U[i].size();
    }
    return count;
}

void SparseLU::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

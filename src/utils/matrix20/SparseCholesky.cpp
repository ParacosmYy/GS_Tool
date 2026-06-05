/**
 * @file SparseCholesky.cpp
 * @brief 稀疏Cholesky分解实现 — 符号分解 + AMD排序 + 超节点消元
 */

#include "utils/matrix20/SparseCholesky.h"

#include <QElapsedTimer>
#include <QSet>
#include <QtMath>

#include <algorithm>
#include <numeric>
#include <queue>

/* ──────────────────── 构造/析构 ──────────────────── */

/** @brief 构造函数 @param parent 父对象 */
SparseCholesky::SparseCholesky(QObject* parent)
    : QObject(parent)
{
}

/** @brief 析构函数 */
SparseCholesky::~SparseCholesky() = default;

/* ──────────────────── 配置 ──────────────────── */

/** @brief 设置分解参数 @param params 参数 */
void SparseCholesky::setParameters(const Parameters& params)
{
    m_params = params;
}

/** @brief 获取当前参数 @return 参数 */
SparseCholesky::Parameters SparseCholesky::parameters() const
{
    return m_params;
}

/* ──────────────────── 矩阵构建 ──────────────────── */

/** @brief 从COO格式构建CSC矩阵 @param n 维度 @param rows 行索引 @param cols 列索引 @param vals 值 @return CSC矩阵 */
SparseCholesky::CSCMatrix SparseCholesky::buildFromCOO(
    int n, const QVector<int>& rows,
    const QVector<int>& cols,
    const QVector<double>& vals) const
{
    CSCMatrix mat;
    mat.n = n;
    int nnz = qMin(rows.size(), qMin(cols.size(), vals.size()));

    /* 按列排序的COO条目 */
    QVector<QPair<QPair<int, int>, double>> entries;
    entries.reserve(nnz);
    for (int i = 0; i < nnz; ++i) {
        int r = qBound(0, rows[i], n - 1);
        int c = qBound(0, cols[i], n - 1);
        /* 对称: 仅存下三角 */
        if (r >= c) {
            entries.append({{r, c}, vals[i]});
        } else {
            entries.append({{c, r}, vals[i]});
        }
    }

    std::sort(entries.begin(), entries.end(),
              [](const auto& a, const auto& b) {
                  return a.first.second < b.first.second ||
                         (a.first.second == b.first.second &&
                          a.first.first < b.first.first);
              });

    /* 去重合并 */
    QVector<QPair<QPair<int, int>, double>> unique;
    for (const auto& e : entries) {
        if (!unique.isEmpty() && unique.last().first == e.first) {
            unique.last().second += e.second;
        } else {
            unique.append(e);
        }
    }

    mat.nnz = unique.size();
    mat.colPtr.resize(n + 1, 0);
    mat.rowIdx.resize(mat.nnz);
    mat.values.resize(mat.nnz);

    int colStart = 0;
    for (int i = 0; i < unique.size(); ++i) {
        mat.rowIdx[i] = unique[i].first.first;
        mat.values[i] = unique[i].second;
        int col = unique[i].first.second;
        while (colStart <= col) {
            mat.colPtr[++colStart] = i;
        }
    }
    while (colStart <= n) {
        mat.colPtr[++colStart] = mat.nnz;
    }

    return mat;
}

/** @brief 创建单位矩阵 @param n 维度 @return CSC矩阵 */
SparseCholesky::CSCMatrix SparseCholesky::identity(int n) const
{
    CSCMatrix mat;
    mat.n = n;
    mat.nnz = n;
    mat.colPtr.resize(n + 1);
    mat.rowIdx.resize(n);
    mat.values.resize(n, 1.0);
    for (int i = 0; i < n; ++i) {
        mat.colPtr[i] = i;
        mat.rowIdx[i] = i;
    }
    mat.colPtr[n] = n;
    return mat;
}

/* ──────────────────── 分解 ──────────────────── */

/** @brief 执行稀疏Cholesky分解 @param A 输入SPD矩阵 @return 分解结果 */
SparseCholesky::FactorResult SparseCholesky::factorize(const CSCMatrix& A)
{
    QElapsedTimer timer;
    timer.start();

    FactorResult result;
    if (A.n == 0) return result;

    /* 步骤1: AMD排序 */
    QVector<int> perm;
    if (m_params.enableAMD) {
        perm = amdOrdering(A);
        emit orderingCompleted(tr("AMD"), A.nnz);
    } else {
        perm.resize(A.n);
        std::iota(perm.begin(), perm.end(), 0);
    }
    result.permutation = perm;

    /* 逆排列 */
    result.invPermutation.resize(A.n);
    for (int i = 0; i < A.n; ++i) {
        result.invPermutation[perm[i]] = i;
    }

    /* 步骤2: 符号分解 */
    CSCMatrix symbol = symbolicFactorization(A, perm);
    result.L = symbol;

    /* 步骤3: 数值分解 */
    auto [lValues, dValues] = numericFactorization(A, symbol, perm);
    result.L.values = lValues;
    result.D = dValues;

    /* 计算fill-in */
    result.fillIn = result.L.nnz - A.nnz;

    /* 计算log(det) */
    result.logDeterminant = 0.0;
    result.isPositiveDefinite = true;
    for (int i = 0; i < A.n; ++i) {
        if (dValues[i] <= 0) {
            result.isPositiveDefinite = false;
            break;
        }
        result.logDeterminant += qLn(dValues[i]);
    }

    /* 超节点检测 */
    if (m_params.enableSupernodal) {
        auto supernodes = detectSupernodes(result.L.colPtr, result.L.rowIdx);
        result.supernodes = supernodes.size();
    }

    result.success = true;

    /* 统计 */
    m_stats.totalFactorizations++;
    m_stats.totalNonzerosProcessed += static_cast<quint64>(result.L.nnz);
    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(m_stats.totalFactorizations, 1ULL));

    emit factorizationCompleted(result.L.nnz, result.fillIn);
    return result;
}

/* ──────────────────── 求解 ──────────────────── */

/** @brief 求解Ax = b @param factor 分解结果 @param b 右端向量 @return 解向量 */
QVector<double> SparseCholesky::solve(const FactorResult& factor,
                                      const QVector<double>& b) const
{
    if (!factor.success || b.size() != factor.permutation.size()) return {};

    int n = b.size();

    /* Pb = P * b */
    QVector<double> pb(n);
    for (int i = 0; i < n; ++i) {
        pb[i] = b[factor.permutation[i]];
    }

    /* 前推 Lx = Pb */
    QVector<double> y = forwardSolve(factor.L, pb);

    /* 对角 Dy = D^{-1} y */
    QVector<double> z = diagonalSolve(factor.D, y);

    /* 回代 L^T x = z */
    QVector<double> x = backwardSolve(factor.L, z);

    /* P^T x (逆排列) */
    QVector<double> result(n);
    for (int i = 0; i < n; ++i) {
        result[factor.permutation[i]] = x[i];
    }

    return result;
}

/** @brief 批量求解 @param factor 分解结果 @param B 右端矩阵 @return 解矩阵 */
QVector<QVector<double>> SparseCholesky::solveBatch(
    const FactorResult& factor,
    const QVector<QVector<double>>& B) const
{
    QVector<QVector<double>> X;
    X.reserve(B.size());
    for (const auto& b : B) {
        X.append(solve(factor, b));
    }
    return X;
}

/* ──────────────────── AMD排序 ──────────────────── */

/** @brief AMD近似最小度排序 @param A 输入矩阵 @return 排列向量 */
QVector<int> SparseCholesky::amdOrdering(const CSCMatrix& A) const
{
    int n = A.n;
    QVector<int> perm(n);
    std::iota(perm.begin(), perm.end(), 0);

    if (n <= 3) return perm;

    /* 简化的近似最小度: 按度数升序排列 */
    QVector<int> degree(n);
    for (int i = 0; i < n; ++i) {
        degree[i] = 0;
        for (int p = A.colPtr[i]; p < A.colPtr[i + 1]; ++p) {
            if (A.rowIdx[p] != i) degree[i]++;
        }
    }

    /* 贪心最小度消元排序 */
    QVector<bool> eliminated(n, false);

    for (int step = 0; step < n; ++step) {
        int best = -1;
        int bestDeg = n + 1;

        for (int i = 0; i < n; ++i) {
            if (!eliminated[i] && degree[i] < bestDeg) {
                bestDeg = degree[i];
                best = i;
            }
        }

        if (best < 0) best = step;
        perm[step] = best;
        eliminated[best] = true;

        /* 模拟消元: 更新邻居度数 */
        for (int p = A.colPtr[best]; p < A.colPtr[best + 1]; ++p) {
            int neighbor = A.rowIdx[p];
            if (!eliminated[neighbor]) {
                degree[neighbor] = qMax(0, degree[neighbor] - 1);
            }
        }
    }

    return perm;
}

/* ──────────────────── 符号分解 ──────────────────── */

/** @brief 符号分解 @param A 输入矩阵 @param perm 排列 @return L的结构 */
SparseCholesky::CSCMatrix SparseCholesky::symbolicFactorization(
    const CSCMatrix& A, const QVector<int>& perm) const
{
    int n = A.n;
    CSCMatrix L;
    L.n = n;

    /* 计算消元树和非零模式(简化: 使用每列的可达集) */
    QVector<QVector<int>> pattern(n);

    for (int j = 0; j < n; ++j) {
        int pj = perm[j];

        /* 收集A的第pj列的行索引(排列后) */
        QSet<int> reachSet;
        for (int p = A.colPtr[pj]; p < A.colPtr[pj + 1]; ++p) {
            int row = A.rowIdx[p];
            int pi = perm.indexOf(row);
            if (pi > j) {
                reachSet.insert(pi);
            }
        }

        /* 合并消元树中子节点的模式 */
        for (int parent : reachSet) {
            for (int idx : pattern[parent]) {
                if (idx > j) reachSet.insert(idx);
            }
        }

        pattern[j].append(j); /* 对角线 */
        for (int r : reachSet) {
            pattern[j].append(r);
        }
        std::sort(pattern[j].begin(), pattern[j].end());
    }

    /* 构建CSC结构 */
    L.colPtr.resize(n + 1);
    L.colPtr[0] = 0;

    int nnz = 0;
    for (int j = 0; j < n; ++j) {
        nnz += pattern[j].size();
        L.colPtr[j + 1] = nnz;
    }

    L.nnz = nnz;
    L.rowIdx.resize(nnz);
    L.values.resize(nnz, 0.0);

    int pos = 0;
    for (int j = 0; j < n; ++j) {
        for (int row : pattern[j]) {
            L.rowIdx[pos++] = row;
        }
    }

    return L;
}

/* ──────────────────── 数值分解 ──────────────────── */

/** @brief 数值分解 @param A 输入矩阵 @param symbol 符号结构 @param perm 排列 @return (L值, D值) */
QPair<QVector<double>, QVector<double>> SparseCholesky::numericFactorization(
    const CSCMatrix& A,
    const CSCMatrix& symbol,
    const QVector<int>& perm)
{
    int n = A.n;
    QVector<double> lValues = symbol.values;
    QVector<double> D(n, 0.0);

    /* 密集工作数组 */
    QVector<double> work(n, 0.0);
    QVector<int> marker(n, -1);

    /* 从A中提取排列后的值 */
    QVector<double> diagA(n, 0.0);
    for (int j = 0; j < n; ++j) {
        int pj = perm[j];
        for (int p = A.colPtr[pj]; p < A.colPtr[pj + 1]; ++p) {
            if (A.rowIdx[p] == pj) {
                diagA[j] = A.values[p];
                break;
            }
        }
    }

    /* 列消元 */
    for (int j = 0; j < n; ++j) {
        /* scatter A的第pj列到work */
        int pj = perm[j];
        work.fill(0.0);
        marker.fill(-1);

        for (int p = A.colPtr[pj]; p < A.colPtr[pj + 1]; ++p) {
            int pi = perm.indexOf(A.rowIdx[p]);
            if (pi >= j && pi < n) {
                work[pi] = A.values[p];
                marker[pi] = j;
            }
        }

        /* 减去已完成的L列的贡献 */
        for (int k = 0; k < j; ++k) {
            double lkj = 0.0;
            /* 在L的第k列找L[j][k] */
            for (int p = symbol.colPtr[k]; p < symbol.colPtr[k + 1]; ++p) {
                if (symbol.rowIdx[p] == j) {
                    lkj = lValues[p];
                    break;
                }
            }

            if (qAbs(lkj) < 1e-30) continue;

            /* work -= L[j][k] * L[i][k] * D[k] */
            for (int p = symbol.colPtr[k]; p < symbol.colPtr[k + 1]; ++p) {
                int i = symbol.rowIdx[p];
                if (i >= j) {
                    work[i] -= lkj * lValues[p] * D[k];
                }
            }
        }

        /* D[j] = work[j] */
        D[j] = work[j];
        if (D[j] <= m_params.pivotTolerance) {
            D[j] = m_params.pivotTolerance; /* 正定性修正 */
        }

        /* L[j:n, j] = work[j:n] / D[j] */
        for (int p = symbol.colPtr[j]; p < symbol.colPtr[j + 1]; ++p) {
            int i = symbol.rowIdx[p];
            if (i == j) {
                lValues[p] = 1.0; /* 对角线为1 */
            } else {
                lValues[p] = work[i] / D[j];
            }
        }
    }

    return {lValues, D};
}

/* ──────────────────── 超节点检测 ──────────────────── */

/** @brief 超节点检测 @param colPtr 列指针 @param rowIdx 行索引 @return 超节点列表 */
QVector<QPair<int, int>> SparseCholesky::detectSupernodes(
    const QVector<int>& colPtr,
    const QVector<int>& rowIdx) const
{
    int n = colPtr.size() - 1;
    QVector<QPair<int, int>> supernodes;

    int i = 0;
    while (i < n) {
        int start = i;
        int colLen = colPtr[i + 1] - colPtr[i];

        /* 检查后续列是否有相同的行模式 */
        while (i + 1 < n) {
            int nextLen = colPtr[i + 2] - colPtr[i + 1];
            if (nextLen != colLen - 1) break;

            /* 验证行模式匹配(跳过对角线) */
            bool match = true;
            for (int p1 = colPtr[i] + 1, p2 = colPtr[i + 1];
                 p1 < colPtr[i + 1] && p2 < colPtr[i + 2]; ++p1, ++p2) {
                if (rowIdx[p1] != rowIdx[p2]) {
                    match = false;
                    break;
                }
            }

            if (!match) break;
            ++i;
        }

        supernodes.append({start, i - start + 1});
        ++i;
    }

    return supernodes;
}

/* ──────────────────── 三角求解 ──────────────────── */

/** @brief 前推(Lx = b) @param L 下三角因子 @param b 右端 @return 解 */
QVector<double> SparseCholesky::forwardSolve(const CSCMatrix& L,
                                             const QVector<double>& b) const
{
    int n = L.n;
    QVector<double> x = b;

    for (int j = 0; j < n; ++j) {
        double xj = x[j];
        for (int p = L.colPtr[j] + 1; p < L.colPtr[j + 1]; ++p) {
            int i = L.rowIdx[p];
            x[i] -= L.values[p] * xj;
        }
    }

    return x;
}

/** @brief 对角求解(Dx = b) @param D 对角元素 @param b 右端 @return 解 */
QVector<double> SparseCholesky::diagonalSolve(const QVector<double>& D,
                                              const QVector<double>& b) const
{
    int n = D.size();
    QVector<double> x(n);
    for (int i = 0; i < n; ++i) {
        x[i] = (qAbs(D[i]) > 1e-30) ? b[i] / D[i] : 0.0;
    }
    return x;
}

/** @brief 回代(L^T x = b) @param L 下三角因子 @param b 右端 @return 解 */
QVector<double> SparseCholesky::backwardSolve(const CSCMatrix& L,
                                              const QVector<double>& b) const
{
    int n = L.n;
    QVector<double> x = b;

    for (int j = n - 1; j >= 0; --j) {
        for (int p = L.colPtr[j] + 1; p < L.colPtr[j + 1]; ++p) {
            x[j] -= L.values[p] * x[L.rowIdx[p]];
        }
    }

    return x;
}

/* ──────────────────── 统计 ──────────────────── */

/** @brief 获取统计 @return 统计 */
SparseCholesky::Stats SparseCholesky::stats() const { return m_stats; }

/** @brief 重置统计 */
void SparseCholesky::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

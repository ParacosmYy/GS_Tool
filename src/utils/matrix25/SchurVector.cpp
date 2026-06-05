/**
 * @file SchurVector.cpp
 * @brief Schur向量提取实现 — QR迭代+有序Schur分解+不变子空间
 */

#include "utils/matrix25/SchurVector.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
SchurVector::SchurVector(QObject* parent)
    : QObject(parent)
    , m_maxIterations(200)
    , m_tolerance(1e-10)
{
}

void SchurVector::setMaxIterations(int iterations) { m_maxIterations = qMax(10, iterations); }
void SchurVector::setTolerance(double tolerance) { m_tolerance = qMax(1e-15, tolerance); }

/**
 * @brief 执行Schur分解
 * @param matrix 输入方阵
 * @return Schur分解结果
 */
SchurVector::SchurResult SchurVector::decompose(
    const QVector<QVector<double>>& matrix)
{
    QElapsedTimer timer;
    timer.start();

    SchurResult result;
    int n = matrix.size();
    if (n == 0) return result;

    /* 复制矩阵 */
    QVector<QVector<double>> A = matrix;
    result.Q = identity(n);

    /* QR迭代 */
    int iterations = 0;
    qrIteration(A, result.Q, iterations);
    result.T = A;
    result.iterations = iterations;

    /* 提取特征值 */
    extractEigenvalues(A, result.eigenvalues, result.eigenvaluesImag);

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalDecompositions;
    m_stats.totalEigenvaluesExtracted += static_cast<quint64>(result.eigenvalues.size());
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalDecompositions);

    emit decompositionCompleted(n, iterations);
    return result;
}

/**
 * @brief 提取不变子空间基(Schur向量)
 * @param result Schur分解结果
 * @param selector 特征值选择条件
 * @return 不变子空间基向量(每列一个向量)
 */
QVector<QVector<double>> SchurVector::extractSubspace(
    const SchurResult& result,
    const EigenvalueSelector& selector) const
{
    int n = result.eigenvalues.size();
    if (n == 0) return {};

    /* 找到满足条件的特征值索引 */
    QList<int> selected;
    for (int i = 0; i < n; ++i) {
        double re = result.eigenvalues[i];
        double im = result.eigenvaluesImag[i];
        double mag = qSqrt(re * re + im * im);

        if (re >= selector.minReal && re <= selector.maxReal
            && mag >= selector.minAbs && mag <= selector.maxAbs) {
            selected.append(i);
        }
    }

    /* 按要求排序: 选取最大特征值 */
    if (selector.selectLargest) {
        std::sort(selected.begin(), selected.end(), [&](int a, int b) {
            double ma = qSqrt(result.eigenvalues[a] * result.eigenvalues[a]
                + result.eigenvaluesImag[a] * result.eigenvaluesImag[a]);
            double mb = qSqrt(result.eigenvalues[b] * result.eigenvalues[b]
                + result.eigenvaluesImag[b] * result.eigenvaluesImag[b]);
            return ma > mb;
        });
    }

    /* 限制个数 */
    if (selector.count > 0 && selected.size() > selector.count) {
        selected = selected.mid(0, selector.count);
    }

    /* 提取对应的Schur向量(Q的列) */
    int numVecs = selected.size();
    QVector<QVector<double>> subspace(n, QVector<double>(numVecs, 0.0));
    for (int col = 0; col < numVecs; ++col) {
        int idx = selected[col];
        for (int row = 0; row < n; ++row) {
            subspace[row][col] = result.Q[row][idx];
        }
    }
    return subspace;
}

/**
 * @brief 重排Schur形式(按特征值排序)
 * @param result [in/out] Schur分解结果
 * @param selector 排序条件
 * @return 重排后的Schur向量矩阵
 */
QVector<QVector<double>> SchurVector::reorderSchur(
    SchurResult& result,
    const EigenvalueSelector& selector) const
{
    int n = result.eigenvalues.size();
    if (n <= 1) return result.Q;

    /* 构建排序索引: 按特征值绝对值降序 */
    QList<int> indices;
    for (int i = 0; i < n; ++i) indices.append(i);

    if (selector.selectLargest) {
        std::sort(indices.begin(), indices.end(), [&](int a, int b) {
            double ma = qSqrt(result.eigenvalues[a] * result.eigenvalues[a]
                + result.eigenvaluesImag[a] * result.eigenvaluesImag[a]);
            double mb = qSqrt(result.eigenvalues[b] * result.eigenvalues[b]
                + result.eigenvaluesImag[b] * result.eigenvaluesImag[b]);
            return ma > mb;
        });
    }

    /* 对Schur形式和Q矩阵应用置换 */
    QVector<int> perm(n);
    for (int i = 0; i < n; ++i) perm[i] = indices[i];

    /* 重排T的行和列 */
    QVector<QVector<double>> newT(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            newT[i][j] = result.T[perm[i]][perm[j]];
        }
    }
    result.T = newT;

    /* 重排Q的列 */
    QVector<QVector<double>> newQ(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            newQ[i][j] = result.Q[i][perm[j]];
        }
    }
    result.Q = newQ;

    /* 重排特征值 */
    QVector<double> newEig(n), newEigIm(n);
    for (int i = 0; i < n; ++i) {
        newEig[i] = result.eigenvalues[perm[i]];
        newEigIm[i] = result.eigenvaluesImag[perm[i]];
    }
    result.eigenvalues = newEig;
    result.eigenvaluesImag = newEigIm;

    return result.Q;
}

/** @brief 重置统计信息 */
void SchurVector::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief QR迭代求Schur形式
 * @param A [in/out] 输入矩阵，输出Schur形式
 * @param Q [in/out] 累积正交变换
 * @param iterations [out] 迭代次数
 */
void SchurVector::qrIteration(QVector<QVector<double>>& A,
                              QVector<QVector<double>>& Q,
                              int& iterations)
{
    int n = A.size();
    double norm = frobeniusNorm(A);
    if (norm < 1e-15) { iterations = 0; return; }

    for (iterations = 0; iterations < m_maxIterations; ++iterations) {
        /* Wilkinson位移: 使用右下角2x2块的特征值 */
        double shift = 0.0;
        if (n >= 2) {
            double a = A[n-2][n-2], b = A[n-2][n-1];
            double c = A[n-1][n-2], d = A[n-1][n-1];
            double tr = a + d;
            double det = a * d - b * c;
            double disc = qSqrt(qAbs(tr * tr / 4.0 - det));
            double e1 = tr / 2.0 + disc;
            double e2 = tr / 2.0 - disc;
            shift = (qAbs(e1 - d) < qAbs(e2 - d)) ? e1 : e2;
        }

        /* 应用位移: A - shift*I */
        for (int i = 0; i < n; ++i) A[i][i] -= shift;

        /* QR分解 */
        QVector<QVector<double>> qk, rk;
        householderQR(A, qk, rk);

        /* A = R * Q + shift * I */
        A = matMul(rk, qk);
        for (int i = 0; i < n; ++i) A[i][i] += shift;

        /* 累积正交变换 Q = Q * qk */
        Q = matMul(Q, qk);

        /* 收敛判定: 次对角线元素足够小 */
        bool converged = true;
        for (int i = 1; i < n; ++i) {
            if (qAbs(A[i][i-1]) > m_tolerance * norm) {
                converged = false;
                break;
            }
        }
        if (converged) break;
    }
}

/**
 * @brief Householder QR分解
 * @param A 输入矩阵
 * @param Q [out] 正交矩阵
 * @param R [out] 上三角矩阵
 */
void SchurVector::householderQR(const QVector<QVector<double>>& A,
                                QVector<QVector<double>>& Q,
                                QVector<QVector<double>>& R) const
{
    int m = A.size();
    if (m == 0) return;
    int n = A[0].size();

    R = A;
    Q = identity(m);

    for (int k = 0; k < qMin(m - 1, n); ++k) {
        /* 计算Householder向量 */
        double norm = 0.0;
        for (int i = k; i < m; ++i) norm += R[i][k] * R[i][k];
        norm = qSqrt(norm);
        if (norm < 1e-15) continue;

        double sign = (R[k][k] >= 0) ? 1.0 : -1.0;
        double alpha = sign * norm;
        R[k][k] += alpha;

        /* 归一化 */
        double beta = 0.0;
        for (int i = k; i < m; ++i) beta += R[i][k] * R[i][k];
        beta = qSqrt(beta);
        if (beta < 1e-15) continue;
        for (int i = k; i < m; ++i) R[i][k] /= beta;

        /* 变换R */
        for (int j = k; j < n; ++j) {
            double dot = 0.0;
            for (int i = k; i < m; ++i) dot += R[i][k] * R[i][j];
            for (int i = k; i < m; ++i) R[i][j] -= 2.0 * dot * R[i][k];
        }

        /* 变换Q */
        for (int j = 0; j < m; ++j) {
            double dot = 0.0;
            for (int i = k; i < m; ++i) dot += R[i][k] * Q[i][j];
            for (int i = k; i < m; ++i) Q[i][j] -= 2.0 * dot * R[i][k];
        }

        /* 恢复R的对角元素 */
        R[k][k] = -alpha;
        for (int i = k + 1; i < m; ++i) R[i][k] = 0.0;
    }

    /* Q需要转置(存储为Q^T) */
    QVector<QVector<double>> Qt = Q;
    for (int i = 0; i < m; ++i)
        for (int j = 0; j < m; ++j)
            Q[i][j] = Qt[j][i];
}

/**
 * @brief 从准上三角Schur形式提取特征值
 * @param T Schur形式矩阵
 * @param real [out] 特征值实部
 * @param imag [out] 特征值虚部
 */
void SchurVector::extractEigenvalues(const QVector<QVector<double>>& T,
                                     QVector<double>& real,
                                     QVector<double>& imag) const
{
    int n = T.size();
    real.resize(n);
    imag.resize(n);

    int i = 0;
    while (i < n) {
        if (i == n - 1 || qAbs(T[i+1][i]) < m_tolerance) {
            /* 1x1块: 实特征值 */
            real[i] = T[i][i];
            imag[i] = 0.0;
            ++i;
        } else {
            /* 2x2块: 复共轭特征值 */
            double a = T[i][i], b = T[i][i+1];
            double c = T[i+1][i], d = T[i+1][i+1];
            double tr = a + d;
            double det = a * d - b * c;
            double disc = tr * tr - 4.0 * det;
            if (disc >= 0) {
                real[i] = (tr + qSqrt(disc)) / 2.0;
                real[i+1] = (tr - qSqrt(disc)) / 2.0;
                imag[i] = 0.0;
                imag[i+1] = 0.0;
            } else {
                real[i] = tr / 2.0;
                real[i+1] = tr / 2.0;
                imag[i] = qSqrt(-disc) / 2.0;
                imag[i+1] = -qSqrt(-disc) / 2.0;
            }
            i += 2;
        }
    }
}

/**
 * @brief Frobenius范数
 * @param A 矩阵
 * @return 范数值
 */
double SchurVector::frobeniusNorm(const QVector<QVector<double>>& A) const
{
    double sum = 0.0;
    for (const auto& row : A)
        for (double v : row) sum += v * v;
    return qSqrt(sum);
}

/** @brief 单位矩阵 @param n 阶数 @return 单位矩阵 */
QVector<QVector<double>> SchurVector::identity(int n) const
{
    QVector<QVector<double>> I(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) I[i][i] = 1.0;
    return I;
}

/**
 * @brief 矩阵乘法
 * @param A 矩阵A
 * @param B 矩阵B
 * @return 乘积矩阵
 */
QVector<QVector<double>> SchurVector::matMul(
    const QVector<QVector<double>>& A,
    const QVector<QVector<double>>& B) const
{
    int m = A.size();
    if (m == 0) return {};
    int n = B[0].size();
    int k = B.size();
    QVector<QVector<double>> C(m, QVector<double>(n, 0.0));
    for (int i = 0; i < m; ++i)
        for (int j = 0; j < n; ++j)
            for (int p =  0; p < k; ++p)
                C[i][j] += A[i][p] * B[p][j];
    return C;
}

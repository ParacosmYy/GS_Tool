/**
 * @file TensorDecomposition.cpp
 * @brief 张量CP分解实现 — 交替最小二乘(ALS)
 */

#include "utils/tensor3/TensorDecomposition.h"

#include <QElapsedTimer>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
TensorDecomposition::TensorDecomposition(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置CP分解的秩
 * @param r 目标秩(必须 > 0)
 */
void TensorDecomposition::setRank(int r)
{
    m_rank = qMax(1, r);
}

/**
 * @brief 矩阵乘法 C = A^T * B
 * @param A 矩阵A (rowsA × colsA)
 * @param B 矩阵B (rowsB × colsB), rowsB必须等于rowsA
 * @return 乘积矩阵 (colsA × colsB)
 */
QVector<QVector<double>> TensorDecomposition::matMulATB(
    const QVector<QVector<double>>& A,
    const QVector<QVector<double>>& B)
{
    if (A.isEmpty() || B.isEmpty()) return {};

    int rowsA = A.size();
    int colsA = A[0].size();
    int colsB = B[0].size();

    QVector<QVector<double>> C(colsA, QVector<double>(colsB, 0.0));
    for (int i = 0; i < colsA; ++i) {
        for (int j = 0; j < colsB; ++j) {
            double sum = 0.0;
            for (int k = 0; k < rowsA; ++k) {
                sum += A[k][i] * B[k][j];
            }
            C[i][j] = sum;
        }
    }
    return C;
}

/**
 * @brief Khatri-Rao积 (B ⊙ C)
 * @param B 因子矩阵B (J×R)
 * @param C 因子矩阵C (K×R)
 * @return Khatri-Rao积矩阵 (J*K×R)
 *
 * B ⊙ C 的第r列 = B[:,r] ⊗ C[:,r]
 */
QVector<QVector<double>> TensorDecomposition::khatriRao(
    const QVector<QVector<double>>& B,
    const QVector<QVector<double>>& C)
{
    if (B.isEmpty() || C.isEmpty()) return {};

    int J = B.size();
    int K = C.size();
    int R = B[0].size();

    QVector<QVector<double>> result(J * K, QVector<double>(R, 0.0));
    for (int r = 0; r < R; ++r) {
        for (int j = 0; j < J; ++j) {
            for (int k = 0; k < K; ++k) {
                result[j * K + k][r] = B[j][r] * C[k][r];
            }
        }
    }
    return result;
}

/**
 * @brief 矩阵伪逆 (正规方程法)
 * @param A 输入矩阵 (m×n)
 * @return 伪逆矩阵 (n×m)
 */
QVector<QVector<double>> TensorDecomposition::pseudoInverse(
    const QVector<QVector<double>>& A)
{
    if (A.isEmpty()) return {};

    int m = A.size();
    int n = A[0].size();

    /* A^T * A (n×n) */
    QVector<QVector<double>> ATA(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            double sum = 0.0;
            for (int k = 0; k < m; ++k) {
                sum += A[k][i] * A[k][j];
            }
            ATA[i][j] = sum;
        }
    }

    /* 正则化 (加小对角项防止奇异) */
    double lambda = 1e-8;
    for (int i = 0; i < n; ++i) {
        ATA[i][i] += lambda;
    }

    /* 高斯消元求逆 (n×n) */
    int nn = n;
    QVector<QVector<double>> aug(nn, QVector<double>(2 * nn, 0.0));
    for (int i = 0; i < nn; ++i) {
        for (int j = 0; j < nn; ++j) aug[i][j] = ATA[i][j];
        aug[i][nn + i] = 1.0;
    }
    for (int col = 0; col < nn; ++col) {
        double pivot = aug[col][col];
        if (std::abs(pivot) < 1e-15) return {};
        for (int j = 0; j < 2 * nn; ++j) aug[col][j] /= pivot;
        for (int row = 0; row < nn; ++row) {
            if (row == col) continue;
            double factor = aug[row][col];
            for (int j = 0; j < 2 * nn; ++j) aug[row][j] -= factor * aug[col][j];
        }
    }
    QVector<QVector<double>> ATAInv(nn, QVector<double>(nn, 0.0));
    for (int i = 0; i < nn; ++i)
        for (int j = 0; j < nn; ++j) ATAInv[i][j] = aug[i][nn + j];

    /* pinv(A) = (A^T A)^-1 * A^T */
    QVector<QVector<double>> result(nn, QVector<double>(m, 0.0));
    for (int i = 0; i < nn; ++i) {
        for (int j = 0; j < m; ++j) {
            double sum = 0.0;
            for (int k = 0; k < nn; ++k) {
                sum += ATAInv[i][k] * A[j][k];
            }
            result[i][j] = sum;
        }
    }
    return result;
}

/**
 * @brief 执行CP分解(ALS)
 * @param tensor 三维张量 [I][J][K]
 * @param maxIter 最大ALS迭代次数(默认100)
 * @return 因子矩阵列表 {A(I×R), B(J×R), C(K×R)}
 *
 * ALS交替更新A、B、C三个因子矩阵，每次固定两个求解第三个。
 * X(I×JK) ≈ A * (C ⊙ B)^T
 */
QVector<QVector<double>> TensorDecomposition::decompose(
    const QVector<QVector<QVector<double>>>& tensor,
    int maxIter)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QVector<double>> result;

    int I = tensor.size();
    if (I == 0) return result;
    int J = tensor[0].size();
    if (J == 0) return result;
    int K = tensor[0][0].size();
    if (K == 0) return result;

    int R = m_rank;

    /* 初始化因子矩阵为小随机值 */
    QVector<QVector<double>> A(I, QVector<double>(R, 0.0));
    QVector<QVector<double>> B(J, QVector<double>(R, 0.0));
    QVector<QVector<double>> C(K, QVector<double>(R, 0.0));

    for (int i = 0; i < I; ++i)
        for (int r = 0; r < R; ++r) A[i][r] = 0.1 * (i + r + 1) / (I + R);
    for (int j = 0; j < J; ++j)
        for (int r = 0; r < R; ++r) B[j][r] = 0.1 * (j + r + 1) / (J + R);
    for (int k = 0; k < K; ++k)
        for (int r = 0; r < R; ++r) C[k][r] = 0.1 * (k + r + 1) / (K + R);

    /* 将张量展开为模式1矩阵 X(I × JK) */
    int JK = J * K;
    QVector<QVector<double>> Xmode1(I, QVector<double>(JK, 0.0));
    for (int i = 0; i < I; ++i) {
        for (int j = 0; j < J; ++j) {
            for (int k = 0; k < K; ++k) {
                Xmode1[i][j * K + k] = tensor[i][j][k];
            }
        }
    }

    /* ALS迭代 */
    for (int iter = 0; iter < maxIter; ++iter) {
        /* 更新A: A = X1 * pinv(C ⊙ B)^T = X1 * (C ⊙ B) * pinv((C⊙B)^T*(C⊙B)) */
        auto CkB = khatriRao(C, B);
        auto CkBpinv = pseudoInverse(CkB);
        /* A = X1 * CkB * pinv(CkB)^T  => 简化: A = X1 * (CkB * pinv(CkB)^T) */
        /* 更简洁: A(i,:) = X1(i,:) * CkB * (CkB^T CkB)^-1 CkB^T  */
        /* 直接: A = X1 * pinv(CkB)^T */
        for (int i = 0; i < I; ++i) {
            /* X1(i,:) is 1×JK, CkBpinv is R×JK, result is R×1 */
            for (int r = 0; r < R; ++r) {
                double sum = 0.0;
                for (int jk = 0; jk < JK; ++jk) {
                    sum += Xmode1[i][jk] * CkBpinv[r][jk];
                }
                A[i][r] = sum;
            }
        }

        /* 更新B: 展开模式2 X2(J × IK) */
        auto AkC = khatriRao(A, C);
        auto AkCpinv = pseudoInverse(AkC);
        for (int j = 0; j < J; ++j) {
            for (int r = 0; r < R; ++r) {
                double sum = 0.0;
                for (int ik = 0; ik < I * K; ++ik) {
                    int ii = ik / K;
                    int kk = ik % K;
                    sum += tensor[ii][j][kk] * AkCpinv[r][ik];
                }
                B[j][r] = sum;
            }
        }

        /* 更新C: 展开模式3 X3(K × IJ) */
        auto AkB = khatriRao(A, B);
        auto AkBpinv = pseudoInverse(AkB);
        for (int k = 0; k < K; ++k) {
            for (int r = 0; r < R; ++r) {
                double sum = 0.0;
                for (int ij = 0; ij < I * J; ++ij) {
                    int ii = ij / J;
                    int jj = ij % J;
                    sum += tensor[ii][jj][k] * AkBpinv[r][ij];
                }
                C[k][r] = sum;
            }
        }
    }

    /* 组装结果: A, B, C 拼接到一个vector中 */
    result.reserve(A.size() + B.size() + C.size());
    result.append(A);
    result.append(B);
    result.append(C);

    /* 更新统计 */
    double elapsed = timer.nsecsElapsed() / 1e6;
    m_timeSumMs += elapsed;
    ++m_stats.totalDecomposed;
    m_stats.avgProcessingTimeMs = m_timeSumMs / m_stats.totalDecomposed;

    emit decompositionCompleted(m_rank);
    return result;
}

/** @brief 重置统计信息 */
void TensorDecomposition::resetStatistics()
{
    m_stats = Stats{};
    m_timeSumMs = 0.0;
}

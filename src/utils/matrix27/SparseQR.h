/**
 * @file SparseQR.h
 * @brief 稀疏QR分解求解器 — CSparse风格
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QMap>
#include <QPair>

/**
 * @brief 稀疏QR分解求解器
 * 左看Householder QR,适用于稀疏超定系统最小二乘
 */
class SparseQR : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalDecompositions = 0;      ///< 累计分解次数
        int totalSolves = 0;              ///< 累计求解次数
        int totalNonZerosR = 0;           ///< R的非零元总数
        double avgProcessingTimeMs = 0.0;
    };

    explicit SparseQR(QObject* parent = nullptr);

    /** @brief 从COO构建矩阵 @param rows @param cols @param values @param m 行数 @param n 列数 */
    void buildFromCOO(const QVector<int>& rows, const QVector<int>& cols,
                      const QVector<double>& values, int m, int n);

    /** @brief 执行QR分解(列主元) @return 是否成功 */
    bool decompose();

    /** @brief 求解最小二乘 Ax≈b @param b 右端向量 @return 解向量x */
    QVector<double> solve(const QVector<double>& b) const;

    /** @brief 获取R的对角线元素 */
    QVector<double> diagonalR() const;

    /** @brief 获取R的非零元数 */
    int nonZeroCountR() const;

    /** @brief 获取矩阵行数 */
    int rows() const { return m_m; }
    /** @brief 获取矩阵列数 */
    int cols() const { return m_n; }

    /** @brief 获取列置换 */
    QVector<int> columnPermutation() const { return m_perm; }

    /** @brief 秩估计 */
    int rank() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 分解完成 @param rank 秩 @param nnz 非零元数 */
    void decompositionCompleted(int rank, int nnz);

private:
    int m_m = 0, m_n = 0;               ///< 矩阵维度
    QVector<QMap<int, double>> m_R;      ///< 上三角R(行稀疏)
    QVector<int> m_perm;                 ///< 列置换
    int m_rank = 0;                      ///< 数值秩
    bool m_decomposed = false;           ///< 是否已分解

    Stats m_stats;
    double m_timeSum = 0.0;
};

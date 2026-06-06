/**
 * @file LUDecomposition.h
 * @brief LU分解(部分主元选取+行列式/逆矩阵) — LU Decomposition with Partial Pivoting, Determinant and Inverse Computation
 *
 * 功能: 实现LU分解(带部分主元选取/partial pivoting)，支持行列式计算、
 *       矩阵求逆、以及基于L/U三角方程组的前推/回代求解。
 *
 * 协作: CholeskyDecomp(Cholesky分解) / QRDecomposition(QR分解) / SvdDecomposition(SVD)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief LU分解器
 */
class LUDecomposition : public QObject {
    Q_OBJECT

public:
    /** @brief 分解结果 */
    struct Result {
        QVector<QVector<double>> L;  ///< 下三角矩阵
        QVector<QVector<double>> U;  ///< 上三角矩阵
        QVector<int> P;              ///< 置换向量
        double determinant = 0.0;    ///< 行列式
        int sign = 1;                ///< 置换符号
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalDecompositions = 0; ///< 累计分解次数
        quint64 totalSolves = 0;         ///< 累计求解次数
        double avgProcessingTimeMs = 0.0;///< 平均处理耗时(ms)
        int lastMatrixSize = 0;          ///< 最近矩阵阶数
    };

    explicit LUDecomposition(QObject* parent = nullptr);
    ~LUDecomposition() override;

    /**
     * @brief 执行LU分解(部分主元选取)
     * @param matrix 方阵(n x n)
     * @return 分解结果(L,U,P,det)
     */
    Result decompose(const QVector<QVector<double>>& matrix);

    /**
     * @brief 求解线性方程组 Ax=b
     * @param result LU分解结果
     * @param b 右端向量
     * @return 解向量x
     */
    QVector<double> solve(const Result& result, const QVector<double>& b) const;

    /**
     * @brief 计算逆矩阵
     * @param result LU分解结果
     * @param n 矩阵阶数
     * @return 逆矩阵
     */
    QVector<QVector<double>> inverse(const Result& result, int n) const;

    /**
     * @brief 仅计算行列式(不存L/U)
     * @param matrix 方阵
     * @return 行列式值
     */
    double determinant(const QVector<QVector<double>>& matrix);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 分解完成 @param n 矩阵阶数 */
    void decompositionCompleted(int n);
    /** @brief 求解完成 */
    void solveCompleted();

private:
    /** @brief 前推求解 Ly=Pb */
    QVector<double> forwardSub(const Result& lu,
                               const QVector<double>& pb) const;

    /** @brief 回代求解 Ux=y */
    QVector<double> backSub(const Result& lu,
                            const QVector<double>& y) const;

    Stats m_stats;
    double m_timeSum = 0.0;
};

/**
 * @file PolynomialRoots.h
 * @brief 多项式求根 — 伴随矩阵特征值法
 *
 * 功能: 通过构造伴随矩阵并计算其特征值，求解多项式的全部复数根，
 *       统计求解次数/耗时。
 */
#ifndef POLYNOMIALROOTS_H
#define POLYNOMIALROOTS_H

#include <QObject>
#include <QVector>
#include <complex>

class PolynomialRoots : public QObject {
    Q_OBJECT
public:
    /** 操作统计 */
    struct Stats {
        quint64 totalSolves = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit PolynomialRoots(QObject* parent = nullptr);

    /**
     * @brief 求多项式的全部复数根
     * @param coeffs 多项式系数，从高次到低次 [a_n, a_{n-1}, ..., a_0]
     * @return 所有复数根(含共轭对)
     */
    QVector<std::complex<double>> solve(const QVector<double>& coeffs);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 求解完成 @param degree 多项式次数 @param rootCount 根的数量 */
    void solveCompleted(int degree, int rootCount);

private:
    /**
     * @brief QR迭代求一般矩阵的全部特征值
     * @param mat n*n矩阵(按行存储) @param n 矩阵维度 @return 特征值列表
     */
    QVector<std::complex<double>> qrEigenvalues(QVector<double>& mat, int n);

    /**
     * @brief 将一般矩阵约化为上Hessenberg形式
     * @param mat n*n矩阵(原地修改) @param n 矩阵维度
     */
    void hessenbergReduce(QVector<double>& mat, int n);

    Stats m_stats;
    double m_timeSum;
};

#endif // POLYNOMIALROOTS_H

/**
 * @file HessenbergReduction.h
 * @brief 上Hessenberg化简 — Householder变换
 *
 * 功能: 使用Householder变换将实矩阵化为上Hessenberg形式，
 *       H = Q^T A Q，为QR算法做准备。
 *
 * 协作: QrEigenSolver(QR特征值) / SchurDecomposition(Schur分解)
 */
#ifndef HESSENBERGREDUCTION_H
#define HESSENBERGREDUCTION_H

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Hessenberg化简器
 */
class HessenbergReduction : public QObject {
    Q_OBJECT

public:
    /** @brief 统计 */
    struct Stats {
        quint64 totalReductions = 0;       ///< 累计化简次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    explicit HessenbergReduction(QObject* parent = nullptr);

    /**
     * @brief 化矩阵为上Hessenberg形式
     * @param A 输入矩阵(n×n)
     * @return (正交矩阵Q, Hessenberg矩阵H)
     */
    QPair<QVector<QVector<double>>, QVector<QVector<double>>> reduce(
        const QVector<QVector<double>>& A);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 化简完成 @param size 矩阵维度 */
    void reductionCompleted(int size);

private:
    Stats m_stats;              ///< 统计信息
    double m_timeSum = 0.0;     ///< 累计耗时
};

#endif // HESSENBERGREDUCTION_H

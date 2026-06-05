/**
 * @file QrEigenSolver.h
 * @brief QR算法特征值求解 — Wilkinson位移
 *
 * 功能: 使用带Wilkinson位移的QR迭代求解实矩阵的全部特征值，
 *       先化上Hessenberg再QR迭代。
 *
 * 协作: HessenbergReduction(Hessenberg化) / SturmSequence(Sturm计数)
 */
#ifndef QREIGENSOLVER_H
#define QREIGENSOLVER_H

#include <QObject>
#include <QVector>

/**
 * @brief QR算法特征值求解器
 */
class QrEigenSolver : public QObject {
    Q_OBJECT

public:
    /** @brief 统计 */
    struct Stats {
        quint64 totalSolves = 0;            ///< 累计求解次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    explicit QrEigenSolver(QObject* parent = nullptr);

    /**
     * @brief 求解矩阵全部特征值
     * @param A 输入矩阵
     * @param maxIter 最大QR迭代次数
     * @return 特征值向量
     */
    QVector<double> solve(const QVector<QVector<double>>& A,
                           int maxIter = 1000);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 求解完成 @param size 矩阵维度 @param iterations 迭代次数 */
    void solveCompleted(int size, int iterations);

private:
    /** @brief Wilkinson位移 */
    double wilkinsonShift(double a, double b, double c) const;

    Stats m_stats;              ///< 统计信息
    double m_timeSum = 0.0;     ///< 累计耗时
};

#endif // QREIGENSOLVER_H

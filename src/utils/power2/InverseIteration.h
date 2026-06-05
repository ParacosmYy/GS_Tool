/**
 * @file InverseIteration.h
 * @brief 反幂迭代 — 求解最接近给定平移的特征值/特征向量
 *
 * 功能: 使用反幂迭代(Inverse Iteration)求解矩阵A中最接近
 *       给定平移sigma的特征值及其对应的特征向量。
 *       通过(A - sigma*I)^{-1} * x迭代实现。
 *       统计求解次数和平均耗时。
 */
#ifndef INVERSEITERATION_H
#define INVERSEITERATION_H

#include <QObject>
#include <QPair>
#include <QVector>

/**
 * @class InverseIteration
 * @brief 反幂迭代工具类，求解最接近平移的特征对
 */
class InverseIteration : public QObject {
    Q_OBJECT
public:
    /** 统计信息结构体 */
    struct Stats {
        quint64 totalSolves = 0;        /**< 总求解次数 */
        double  avgProcessingTimeMs = 0.0; /**< 平均处理耗时(ms) */
    };

    /**
     * @brief 构造函数
     * @param parent 父QObject
     */
    explicit InverseIteration(QObject* parent = nullptr);

    /**
     * @brief 反幂迭代求解最接近shift的特征值和特征向量
     * @param A 输入方阵
     * @param shift 目标平移值(接近所求特征值)
     * @param tol 收敛容差
     * @param maxIter 最大迭代次数
     * @return QPair{eigenvalue, eigenvector}
     */
    QPair<double, QVector<double>>
    solve(const QVector<QVector<double>>& A,
          double shift, double tol = 1e-10, int maxIter = 200);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 求解完成信号 @param eigenvalue 求得的特征值 @param iterations 迭代次数 */
    void solveCompleted(double eigenvalue, int iterations);

private:
    /**
     * @brief 使用部分主元Gauss消元求解线性系统 Ax = b
     * @param A 系数矩阵(会被修改)
     * @param b 右端向量(会被修改)
     * @return 解向量x，失败返回空
     */
    QVector<double> solveLinearSystem(QVector<QVector<double>> A,
                                      QVector<double> b);

    Stats  m_stats;   /**< 统计数据 */
    double m_timeSum; /**< 累计耗时(ms) */
};

#endif // INVERSEITERATION_H

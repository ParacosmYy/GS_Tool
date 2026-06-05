/**
 * @file PolarDecomposition.h
 * @brief 极分解 — 矩阵A = UP分解，基于SVD
 *
 * 功能: 对方阵A进行极分解A = UP，其中U为正交矩阵，P为正定对称矩阵。
 *       基于SVD实现: U = U_svd * V_svd^T, P = V_svd * Sigma * V_svd^T。
 *       统计分解次数和平均耗时。
 */
#ifndef POLARDECOMPOSITION_H
#define POLARDECOMPOSITION_H

#include <QObject>
#include <QPair>
#include <QVector>

/**
 * @class PolarDecomposition
 * @brief 极分解工具类，将矩阵A分解为正交矩阵U和正定对称矩阵P
 */
class PolarDecomposition : public QObject {
    Q_OBJECT
public:
    /** 统计信息结构体 */
    struct Stats {
        quint64 totalDecompositions = 0;  /**< 总分解次数 */
        double  avgProcessingTimeMs = 0.0; /**< 平均处理耗时(ms) */
    };

    /**
     * @brief 构造函数
     * @param parent 父QObject
     */
    explicit PolarDecomposition(QObject* parent = nullptr);

    /**
     * @brief 对矩阵A执行极分解 A = UP
     * @param A 输入方阵
     * @return QPair{U, P}，U为正交矩阵，P为正定对称矩阵
     */
    QPair<QVector<QVector<double>>, QVector<QVector<double>>>
    decompose(const QVector<QVector<double>>& A);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 分解完成信号 @param size 矩阵维度 */
    void decompositionCompleted(int size);

private:
    Stats  m_stats;   /**< 统计数据 */
    double m_timeSum; /**< 累计耗时(ms) */
};

#endif // POLARDECOMPOSITION_H

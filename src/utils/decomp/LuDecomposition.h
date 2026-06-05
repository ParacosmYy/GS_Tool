/**
 * @file LuDecomposition.h
 * @brief LU分解 — Doolittle算法
 *
 * 功能: 将矩阵分解为下三角L和上三角U的乘积，用于线性方程组求解、
 *       行列式计算和矩阵求逆。支持部分主元选取(pivoting)。
 *
 * 协作: ExtendedKalman(矩阵求逆) / SingularValueDecomposition
 */
#ifndef LUDECOMPOSITION_H
#define LUDECOMPOSITION_H

#include <QObject>
#include <QVector>

/**
 * @brief LU分解 — Doolittle算法+部分主元
 */
class LuDecomposition : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalDecompositions = 0;  ///< 累计分解次数
        quint64 totalSolves = 0;          ///< 累计求解次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    /** @brief 分解结果 */
    struct LuResult {
        QVector<QVector<double>> L;       ///< 下三角矩阵
        QVector<QVector<double>> U;       ///< 上三角矩阵
        QVector<int> permutation;         ///< 置换向量
        double determinant = 0.0;         ///< 行列式
        bool success = false;             ///< 是否成功
    };

    explicit LuDecomposition(QObject* parent = nullptr);

    /** @brief 执行LU分解
     *  @param matrix 输入方阵
     *  @return 分解结果 */
    LuResult decompose(const QVector<QVector<double>>& matrix);

    /** @brief 使用已有分解求解Ax=b
     *  @param lu 分解结果
     *  @param b 右端向量
     *  @return 解向量 */
    QVector<double> solve(const LuResult& lu,
                          const QVector<double>& b);

    /** @brief 计算行列式
     *  @param matrix 输入方阵
     *  @return 行列式值 */
    double determinant(const QVector<QVector<double>>& matrix);

    /** @brief 矩阵求逆
     *  @param matrix 输入方阵
     *  @return 逆矩阵，失败返回空 */
    QVector<QVector<double>> inverse(
        const QVector<QVector<double>>& matrix);

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 分解完成 @param n 矩阵维度 @param det 行列式 */
    void decompositionCompleted(int n, double det);

    /** @brief 求解完成 @param n 维度 */
    void solveCompleted(int n);

private:
    double m_timeSum;   ///< 处理时间累加器
    Stats m_stats;      ///< 统计信息
};

#endif // LUDECOMPOSITION_H

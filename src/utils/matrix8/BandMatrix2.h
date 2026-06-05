/**
 * @file BandMatrix2.h
 * @brief 带状矩阵求解器 — Thomas算法与带状LU分解
 *
 * 功能: 求解三对角和多对角带状线性方程组 Ax=b,
 *       Thomas算法 O(n) 求解三对角系统, 扩展Thomas算法支持更宽带宽,
 *       支持部分主元选取保证数值稳定性。
 *
 * 协作: FiniteDifference(有限差分PDE) / CubicSpline(样条插值) / BeamSolver(结构力学)
 */

#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 带状矩阵求解器
 *
 * 使用Thomas算法和扩展Thomas算法高效求解带状线性系统。
 * 三对角系统复杂度 O(n), 更宽带宽系统复杂度 O(n*bw^2)。
 * 支持部分主元选取和条件数估计。
 */
class BandMatrix2 : public QObject
{
    Q_OBJECT

public:
    /** @brief 求解器类型 */
    enum class SolverMethod {
        Thomas,         ///< Thomas算法(仅三对角, O(n))
        ExtendedThomas, ///< 扩展Thomas算法(带状, O(n*bw^2))
        PivotingLU      ///< 带主元选取的LU分解(最稳定)
    };
    Q_ENUM(SolverMethod)

    /** @brief 矩阵条件信息 */
    struct ConditionInfo {
        double conditionEstimate = 0.0;     ///< 条件数估计
        bool isStrictlyDiagonallyDominant = false; ///< 是否严格对角占优
        bool isSymmetric = false;           ///< 是否对称
        bool isPositiveDefinite = false;    ///< 是否正定(对称时)
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalSystemsSolved = 0;         ///< 累计求解系统数
        int totalThomasSolves = 0;          ///< 累计Thomas算法求解次数
        int totalExtendedSolves = 0;        ///< 累计扩展Thomas求解次数
        int totalPivotingSolves = 0;        ///< 累计主元LU求解次数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
    };

    explicit BandMatrix2(QObject* parent = nullptr);

    /**
     * @brief 求解三对角系统(Thomas算法)
     * @param lower 下对角线元素 a[1..n-1]
     * @param mainDiag 主对角线元素 b[0..n-1]
     * @param upper 上对角线元素 c[0..n-2]
     * @param rhs 右端向量 d[0..n-1]
     * @return 解向量 x[0..n-1], 空数组表示求解失败
     */
    QVector<double> solveTridiagonal(const QVector<double>& lower,
                                      const QVector<double>& mainDiag,
                                      const QVector<double>& upper,
                                      const QVector<double>& rhs);

    /**
     * @brief 求解带状系统(扩展Thomas算法)
     * @param bands 带状存储: bands[i] = 第 i 条对角线元素
     *        bands[0] = 主对角线, bands[k] = 上方第 k 条, bands[-k] = 下方第 k 条
     * @param rhs 右端向量
     * @param halfBandwidth 半带宽(不含主对角线)
     * @return 解向量, 空数组表示求解失败
     */
    QVector<double> solveBanded(const QVector<QVector<double>>& bands,
                                 const QVector<double>& rhs,
                                 int halfBandwidth);

    /**
     * @brief 求解带状系统(带主元选取的LU分解)
     * @param bands 带状存储格式(同solveBanded)
     * @param rhs 右端向量
     * @param halfBandwidth 半带宽
     * @return 解向量, 空数组表示求解失败
     */
    QVector<double> solvePivoting(const QVector<QVector<double>>& bands,
                                   const QVector<double>& rhs,
                                   int halfBandwidth);

    /**
     * @brief 循环三对角系统求解(周期边界条件)
     * @param lower 下对角线 a[1..n-1], a[0] 为左下角元素
     * @param mainDiag 主对角线 b[0..n-1]
     * @param upper 上对角线 c[0..n-2], c[n-1] 为右上角元素
     * @param rhs 右端向量 d[0..n-1]
     * @return 解向量, 空数组表示求解失败
     */
    QVector<double> solveCyclicTridiagonal(const QVector<double>& lower,
                                            const QVector<double>& mainDiag,
                                            const QVector<double>& upper,
                                            const QVector<double>& rhs);

    /**
     * @brief 估计矩阵条件信息
     * @param bands 带状存储格式
     * @param halfBandwidth 半带宽
     * @param n 矩阵阶数
     * @return 条件信息
     */
    ConditionInfo estimateCondition(const QVector<QVector<double>>& bands,
                                     int halfBandwidth, int n) const;

    /**
     * @brief 验证解的精度(计算残差范数)
     * @param bands 带状存储格式
     * @param halfBandwidth 半带宽
     * @param solution 解向量
     * @param rhs 右端向量
     * @return 相对残差范数 ||Ax-b|| / ||b||
     */
    double computeResidual(const QVector<QVector<double>>& bands,
                            int halfBandwidth,
                            const QVector<double>& solution,
                            const QVector<double>& rhs) const;

    Stats stats() const;
    void resetStatistics();

private:
    /**
     * @brief 从带状存储格式获取矩阵元素
     * @param bands 带状存储
     * @param halfBandwidth 半带宽
     * @param row 行号
     * @param col 列号
     * @return 矩阵元素值
     */
    double getElement(const QVector<QVector<double>>& bands,
                       int halfBandwidth, int row, int col) const;

    Stats m_stats;
    double m_timeSum = 0.0;
};

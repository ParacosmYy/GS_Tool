/**
 * @file StrassenMultiply.h
 * @brief Strassen矩阵乘法 — 动态交叉点的分治矩阵乘法
 *
 * 功能: 实现Strassen算法，当子矩阵小于阈值时切换到朴素乘法，
 *       支持动态交叉点调整、非方阵补零、块矩阵运算。
 *       适用于大规模矩阵运算、线性代数求解加速。
 *
 * 协作: SchurComplement(块矩阵) / KalmanFilter(矩阵运算)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Strassen矩阵乘法引擎 — 动态交叉点
 */
class StrassenMultiply : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalMultiplications = 0;          ///< 累计矩阵乘法次数
        int totalStrassenCalls = 0;            ///< 累计Strassen递归调用次数
        int totalNaiveFallbacks = 0;           ///< 累计朴素乘法回退次数
        double avgProcessingTimeMs = 0.0;      ///< 平均处理时间(ms)
    };

    /**
     * @brief 构造函数
     * @param crossover 交叉点阈值(子矩阵 ≤ crossover 时用朴素法)
     * @param parent 父对象
     */
    explicit StrassenMultiply(int crossover = 64, QObject* parent = nullptr);

    /**
     * @brief 矩阵乘法 A * B = C (Strassen算法)
     * @param A 左矩阵(m x k)
     * @param B 右矩阵(k x n)
     * @return 结果矩阵(m x n)
     */
    QVector<QVector<double>> multiply(
        const QVector<QVector<double>>& A,
        const QVector<QVector<double>>& B);

    /**
     * @brief 方阵乘法(优化路径)
     * @param A 左方阵(n x n)
     * @param B 右方阵(n x n)
     * @return 结果方阵(n x n)
     */
    QVector<QVector<double>> multiplySquare(
        const QVector<QVector<double>>& A,
        const QVector<QVector<double>>& B);

    /**
     * @brief 朴素O(n^3)矩阵乘法
     * @param A 左矩阵
     * @param B 右矩阵
     * @return 结果矩阵
     */
    static QVector<QVector<double>> naiveMultiply(
        const QVector<QVector<double>>& A,
        const QVector<QVector<double>>& B);

    /**
     * @brief 矩阵加法
     */
    static QVector<QVector<double>> add(
        const QVector<QVector<double>>& A,
        const QVector<QVector<double>>& B);

    /**
     * @brief 矩阵减法
     */
    static QVector<QVector<double>> subtract(
        const QVector<QVector<double>>& A,
        const QVector<QVector<double>>& B);

    /**
     * @brief 自动寻找最优交叉点(基准测试)
     * @param sizes 测试矩阵尺寸列表
     * @return 最优交叉点值
     */
    int autoTuneCrossover(const QList<int>& sizes = {32, 64, 128, 256});

    /**
     * @brief 设置交叉点
     * @param crossover 新阈值
     */
    void setCrossover(int crossover) { m_crossover = crossover; }

    /**
     * @brief 获取当前交叉点
     */
    int crossover() const { return m_crossover; }

    /**
     * @brief 获取统计信息
     */
    const Stats& stats() const { return m_stats; }

    /**
     * @brief 重置统计信息
     */
    void resetStatistics();

signals:
    /**
     * @brief 乘法计算完成
     * @param rows 结果行数
     * @param cols 结果列数
     */
    void multiplicationCompleted(int rows, int cols);

private:
    /**
     * @brief Strassen递归核心(仅方阵，2的幂次)
     * @param A 左方阵
     * @param B 右方阵
     * @param depth 当前递归深度
     * @return 结果方阵
     */
    QVector<QVector<double>> strassenRecursive(
        const QVector<QVector<double>>& A,
        const QVector<QVector<double>>& B,
        int depth);

    /**
     * @brief 补零到2的幂次方阵
     */
    QPair<QVector<QVector<double>>, QVector<QVector<double>>> padToPowerOfTwo(
        const QVector<QVector<double>>& A,
        const QVector<QVector<double>>& B) const;

    /**
     * @brief 裁剪回原始尺寸
     */
    static QVector<QVector<double>> trim(
        const QVector<QVector<double>>& M, int rows, int cols);

    int m_crossover;                           ///< 交叉点阈值
    mutable int m_trimRows = 0;                ///< 裁剪行数(padToPowerOfTwo中修改)
    mutable int m_trimCols = 0;                ///< 裁剪列数(padToPowerOfTwo中修改)

    mutable Stats m_stats;
    mutable double m_timeSum = 0.0;            ///< 处理时间累加器
};

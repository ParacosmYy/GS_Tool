/**
 * @file SchurComplement.h
 * @brief Schur补块矩阵消元引擎 — 用于块矩阵降维求解
 *
 * 功能: 实现Schur补分解，将 [A B; C D] 形式的块矩阵
 *       通过消元降维为 D - C*A^{-1}*B，适用于大规模稀疏
 *       线性系统求解、卡尔曼滤波信息矩阵融合等场景。
 *
 * 协作: DataCorrelator(相关性矩阵) / KalmanFilter(信息融合)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Schur补块矩阵消元引擎
 */
class SchurComplement : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalComplementsComputed = 0;       ///< 累计Schur补计算次数
        int totalInversionsPerformed = 0;       ///< 累计矩阵求逆次数
        double avgProcessingTimeMs = 0.0;       ///< 平均处理时间(ms)
    };

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit SchurComplement(QObject* parent = nullptr);

    /**
     * @brief 计算Schur补 S = D - C * inv(A) * B
     * @param A 左上块矩阵(n x n)
     * @param B 右上块矩阵(n x m)
     * @param C 左下块矩阵(m x n)
     * @param D 右下块矩阵(m x m)
     * @return Schur补矩阵(m x m)
     */
    QVector<QVector<double>> computeSchurComplement(
        const QVector<QVector<double>>& A,
        const QVector<QVector<double>>& B,
        const QVector<QVector<double>>& C,
        const QVector<QVector<double>>& D);

    /**
     * @brief 从合并矩阵中提取Schur补
     * @param blockMatrix 合并块矩阵[(n+m) x (n+m)]
     * @param splitRowCol 分割位置n(A块大小)
     * @return Schur补矩阵(m x m)
     */
    QVector<QVector<double>> computeFromBlock(
        const QVector<QVector<double>>& blockMatrix,
        int splitRowCol);

    /**
     * @brief 求解块线性系统 [A B; C D] [x; y] = [f; g]
     * @param A 左上块
     * @param B 右上块
     * @param C 左下块
     * @param D 右下块
     * @param f 上部右端项
     * @param g 下部右端项
     * @return 解向量对(x, y)
     */
    QPair<QVector<double>, QVector<double>> solveBlockSystem(
        const QVector<QVector<double>>& A,
        const QVector<QVector<double>>& B,
        const QVector<QVector<double>>& C,
        const QVector<QVector<double>>& D,
        const QVector<double>& f,
        const QVector<double>& g);

    /**
     * @brief 矩阵求逆(Gauss-Jordan消元)
     * @param matrix 输入方阵
     * @return 逆矩阵; 空矩阵表示奇异
     */
    QVector<QVector<double>> invertMatrix(
        const QVector<QVector<double>>& matrix);

    /**
     * @brief 矩阵乘法
     * @param A 矩阵A
     * @param B 矩阵B
     * @return A * B
     */
    static QVector<QVector<double>> multiply(
        const QVector<QVector<double>>& A,
        const QVector<QVector<double>>& B);

    /**
     * @brief 矩阵减法
     * @param A 矩阵A
     * @param B 矩阵B
     * @return A - B
     */
    static QVector<QVector<double>> subtract(
        const QVector<QVector<double>>& A,
        const QVector<QVector<double>>& B);

    /**
     * @brief 获取统计信息
     * @return 统计引用
     */
    const Stats& stats() const { return m_stats; }

    /**
     * @brief 重置统计信息
     */
    void resetStatistics();

signals:
    /**
     * @brief Schur补计算完成
     * @param inputRows 输入矩阵行数
     * @param complementRows Schur补行数
     */
    void complementComputed(int inputRows, int complementRows);

private:
    /**
     * @brief 矩阵向量乘法
     * @param M 矩阵
     * @param v 向量
     * @return M * v
     */
    static QVector<double> matVecMultiply(
        const QVector<QVector<double>>& M,
        const QVector<double>& v);

    Stats m_stats;
    double m_timeSum = 0.0;             ///< 处理时间累加器
};

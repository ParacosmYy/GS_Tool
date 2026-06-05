#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief ToeplitzSolver7 - Toeplitz矩阵求解器第7代实现
 *
 * 针对Toeplitz结构矩阵的高效求解，支持Levinson-Durbin算法、
 * Trench算法及循环预条件共轭梯度法。
 */
class ToeplitzSolver7 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalSolveOps = 0; double avgProcessingTimeMs = 0.0; };
    explicit ToeplitzSolver7(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief Levinson-Durbin算法求解Toeplitz方程组
     * @param firstRow 矩阵第一行元素
     * @param rhs 右端向量
     * @return 解向量
     */
    QVector<double> levinsonDurbin(const QVector<double>& firstRow,
                                   const QVector<double>& rhs);

    /**
     * @brief 求解Yule-Walker方程（自回归参数估计）
     * @param autocorrelation 自相关序列
     * @return AR模型系数
     */
    QVector<double> yuleWalker(const QVector<double>& autocorrelation);

    /**
     * @brief 计算Toeplitz矩阵的行列式
     * @param firstRow 矩阵第一行元素
     * @return 行列式值
     */
    double determinant(const QVector<double>& firstRow) const;

    /**
     * @brief 计算Toeplitz矩阵的逆矩阵
     * @param firstRow 矩阵第一行元素
     * @return 逆矩阵
     */
    QVector<QVector<double>> inverse(const QVector<double>& firstRow);

signals:
    void solveCompleted(int matrixSize);

private:
    Stats m_stats; double m_timeSum = 0.0;
};

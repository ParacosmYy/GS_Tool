/**
 * @file L1Regularization.h
 * @brief L1正则化(L1 Regularization / LASSO)
 */

#pragma once

#include <QObject>
#include <QVector>

/**
 * @class L1Regularization
 * @brief L1正则化 — LASSO回归和稀疏求解
 *
 * 支持坐标下降法求解LASSO、弹性网络、软阈值。
 * 适用于特征选择、稀疏建模、信号重构等场景。
 */
class L1Regularization : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalFitted = 0;       /**< 总拟合次数 */
        int totalPredictions = 0;  /**< 总预测次数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 构造函数 */
    explicit L1Regularization(QObject* parent = nullptr);

    /**
     * @brief LASSO回归(坐标下降)
     * @param X 设计矩阵(n×p)
     * @param y 响应向量
     * @param lambda 正则化强度
     * @param maxIter 最大迭代次数
     * @param tolerance 收敛阈值
     * @return 回归系数(p×1)
     */
    QVector<double> lasso(const QVector<QVector<double>>& X,
                            const QVector<double>& y,
                            double lambda, int maxIter = 1000,
                            double tolerance = 1e-6);

    /**
     * @brief 弹性网络(坐标下降)
     * @param X 设计矩阵
     * @param y 响应向量
     * @param lambda L1+L2正则化强度
     * @param alpha L1混合比例(1=LASSO, 0=Ridge)
     * @return 回归系数
     */
    QVector<double> elasticNet(const QVector<QVector<double>>& X,
                                 const QVector<double>& y,
                                 double lambda, double alpha = 0.5);

    /**
     * @brief 软阈值算子
     * @param x 输入值
     * @param threshold 阈值
     * @return 软阈值结果
     */
    static double softThreshold(double x, double threshold);

    /**
     * @brief 正则化路径(多个lambda值)
     * @param X 设计矩阵
     * @param y 响应向量
     * @param lambdas lambda值列表
     * @return 每个lambda对应的系数
     */
    QVector<QVector<double>> regularizationPath(
        const QVector<QVector<double>>& X,
        const QVector<double>& y,
        const QVector<double>& lambdas);

    /**
     * @brief 预测
     * @param coefficients 系数
     * @param x 输入特征
     * @return 预测值
     */
    static double predict(const QVector<double>& coefficients,
                           const QVector<double>& x);

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 拟合完成信号 */
    void fitted(int features, double lambda);

private:
    Stats m_stats;
    double m_timeSum;
};

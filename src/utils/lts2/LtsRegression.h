/**
 * @file LtsRegression.h
 * @brief 最小截平方(LTS)稳健回归 — 抵抗异常值的线性拟合
 *
 * 功能: 基于最小截平方(Least Trimmed Squares)算法进行稳健
 *       线性回归，通过剔除指定比例的最大残差数据点来抵抗
 *       异常值影响。适用于含噪声/离群点的传感器数据拟合。
 *
 * 协作: DataCorrelator(相关性分析) / AnomalyDetector(异常检测)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <QElapsedTimer>

/**
 * @brief 最小截平方稳健回归
 */
class LtsRegression : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalFits = 0;              ///< 累计拟合次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit LtsRegression(QObject* parent = nullptr);

    /**
     * @brief 执行LTS回归拟合
     * @param x 自变量数据
     * @param y 因变量数据
     * @param trimRatio 截断比例(0.0~0.5)，剔除最大残差的比例
     * @return QPair(斜率, 截距)
     */
    QPair<double, double> fit(const QVector<double>& x,
                              const QVector<double>& y,
                              double trimRatio = 0.25);

    /**
     * @brief 计算残差序列(基于最近一次拟合结果)
     * @param x 自变量数据
     * @param y 因变量数据
     * @return 残差向量
     */
    QVector<double> residuals(const QVector<double>& x,
                              const QVector<double>& y) const;

    /**
     * @brief 获取R²决定系数(基于最近一次拟合)
     * @return R²值(0~1)
     */
    double rSquared() const;

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 拟合完成信号 @param slope 斜率 @param intercept 截距 @param r2 R²值 */
    void fitCompleted(double slope, double intercept, double r2);

private:
    /**
     * @brief 使用指定子集进行普通最小二乘拟合
     * @param x 自变量数据
     * @param y 因变量数据
     * @param indices 使用的样本索引
     * @return QPair(斜率, 截距)
     */
    static QPair<double, double> olsSubset(
        const QVector<double>& x,
        const QVector<double>& y,
        const QVector<int>& indices);

    /**
     * @brief 计算截断残差平方和
     * @param x 自变量
     * @param y 因变量
     * @param slope 斜率
     * @param intercept 截距
     * @param h 保留样本数
     * @return 截断RSS
     */
    static double trimmedRSS(const QVector<double>& x,
                             const QVector<double>& y,
                             double slope, double intercept, int h);

    /**
     * @brief 计算完整数据的OLS拟合
     * @param x 自变量
     * @param y 因变量
     * @return QPair(斜率, 截距)
     */
    static QPair<double, double> olsFull(const QVector<double>& x,
                                         const QVector<double>& y);

    double m_slope;         ///< 最近拟合斜率
    double m_intercept;     ///< 最近拟合截距
    double m_rSquared;      ///< 最近拟合R²
    bool m_fitted;          ///< 是否已拟合

    QElapsedTimer m_timer;  ///< 计时器
    double m_timeSum;       ///< 累计耗时
    Stats m_stats;
};

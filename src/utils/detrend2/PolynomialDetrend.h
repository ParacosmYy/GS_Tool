/**
 * @file PolynomialDetrend.h
 * @brief 多项式去趋势 — 基于AIC的自动阶数选择
 *
 * 功能: 对信号进行多项式拟合去趋势，支持手动指定阶数或
 *       基于AIC准则自动选择最优阶数。适用于信号预处理、
 *       消除基线漂移。
 *
 * 协作: WaveformGenerator(信号生成) / DigitalFilter(频域滤波)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QElapsedTimer>

/**
 * @brief 多项式去趋势器 — AIC模型选择
 */
class PolynomialDetrend : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalDetrended = 0;         ///< 累计去趋势次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit PolynomialDetrend(QObject* parent = nullptr);

    /**
     * @brief 多项式去趋势 — 拟合并移除趋势分量
     * @param signal 输入信号
     * @param order 多项式阶数(0=常数, 1=线性, 2=二次...)
     * @return 去趋势后的残差信号
     */
    QVector<double> detrend(const QVector<double>& signal, int order);

    /**
     * @brief 自动去趋势 — 基于AIC选择最优阶数
     * @param signal 输入信号
     * @return 去趋势后的残差信号
     */
    QVector<double> autoDetrend(const QVector<double>& signal);

    /**
     * @brief 设置最大拟合阶数(用于autoDetrend)
     * @param maxOrder 最大阶数
     */
    void setMaxOrder(int maxOrder);

    /**
     * @brief 拟合多项式系数
     * @param y 因变量数据(自变量为均匀采样索引)
     * @param order 多项式阶数
     * @return 系数向量 [a0, a1, a2, ...] (从低阶到高阶)
     */
    QVector<double> fitPolynomial(const QVector<double>& y, int order) const;

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 去趋势完成信号 @param order 使用阶数 @param residualVariance 残差方差 */
    void detrendCompleted(int order, double residualVariance);

private:
    /**
     * @brief 计算AIC值
     * @param residualSum 残差平方和
     * @param n 样本数
     * @param k 参数个数
     * @return AIC值
     */
    double computeAIC(double residualSum, int n, int k) const;

    /**
     * @brief 使用多项式系数计算拟合值
     * @param coeffs 多项式系数
     * @param n 数据点数
     * @return 拟合值向量
     */
    QVector<double> evaluatePolynomial(const QVector<double>& coeffs,
                                        int n) const;

    /**
     * @brief 构建Vandermonde矩阵求解正规方程
     * @param y 因变量
     * @param order 阶数
     * @return 系数向量
     */
    QVector<double> solveVandermonde(const QVector<double>& y,
                                      int order) const;

    int m_maxOrder;             ///< 最大阶数限制(默认10)
    QElapsedTimer m_timer;      ///< 计时器
    double m_timeSum;            ///< 累计耗时
    Stats m_stats;
};

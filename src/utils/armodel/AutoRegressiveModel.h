/**
 * @file AutoRegressiveModel.h
 * @brief 自回归(AR)模型 — Yule-Walker估计/预测/阶数选择
 *
 * 功能: Yule-Walker方程求解AR系数，向前/向后预测，
 *       残差计算，AIC/BIC准则阶数选择，
 *       统计拟合次数/最优阶数/耗时。
 */
#ifndef AUTOREGRESSIVEMODEL_H
#define AUTOREGRESSIVEMODEL_H

#include <QObject>
#include <QVector>

/**
 * @brief 自回归(AR)模型
 */
class AutoRegressiveModel : public QObject {
    Q_OBJECT

public:
    /** @brief 统计 */
    struct Stats {
        quint64 totalFits = 0;         ///< 累计拟合次数
        quint64 totalPredictions = 0;  ///< 累计预测次数
        int     bestOrder = 0;         ///< 最优阶数
        double  bestAic = 0.0;         ///< 最优AIC
        double  bestBic = 0.0;         ///< 最优BIC
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间
    };

    explicit AutoRegressiveModel(QObject* parent = nullptr);

    /** @brief 拟合AR模型 @param data 时间序列 @param order AR阶数(0=自动选择) */
    void fit(const QVector<double>& data, int order = 0);

    /** @brief 自动选择最优阶数 @param data 时间序列 @param maxOrder 最大阶数 @return 最优阶数 */
    int selectOrder(const QVector<double>& data, int maxOrder = 20);

    /** @brief 向前预测 @param steps 预测步数 @return 预测值 */
    QVector<double> predict(int steps);

    /** @brief 计算残差 @param data 原始数据 @return 残差序列 */
    QVector<double> residuals(const QVector<double>& data);

    /** @brief 计算AIC @param data 数据 @param order 阶数 @return AIC值 */
    double computeAic(const QVector<double>& data, int order);

    /** @brief 计算BIC @param data 数据 @param order 阶数 @return BIC值 */
    double computeBic(const QVector<double>& data, int order);

    /** @brief AR系数 @return 系数向量 */
    QVector<double> coefficients() const;

    /** @brief 白噪声方差 @return 方差估计 */
    double noiseVariance() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 模型拟合完成 @param order AR阶数 @param noiseVar 噪声方差 */
    void modelFitted(int order, double noiseVar);
    /** @brief 预测完成 @param steps 预测步数 */
    void predictionCompleted(int steps);

private:
    /** @brief 计算自相关函数 @param data 数据 @param maxLag 最大滞后 @return 自相关值 */
    QVector<double> autocorrelation(const QVector<double>& data, int maxLag);
    /** @brief Levinson-Durbin递归 @param acf 自相关 @param order 阶数 @return (系数, 噪声方差) */
    QPair<QVector<double>, double> levinsonDurbin(
        const QVector<double>& acf, int order);

    QVector<double> m_coeffs;      ///< AR系数
    QVector<double> m_lastData;    ///< 最近拟合数据(用于预测)
    double m_noiseVar;             ///< 白噪声方差
    int m_order;                   ///< 当前阶数
    Stats m_stats;
    double m_timeSum;
};

#endif // AUTOREGRESSIVEMODEL_H

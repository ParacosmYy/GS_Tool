/**
 * @file HoltWinters.h
 * @brief Holt-Winters预测器 — 三次指数平滑时间序列预测
 *
 * 功能: 实现加法/乘法Holt-Winters，支持趋势+季节性分解、
 *       多步预测、自动参数优化，统计预测次数/误差/耗时。
 */
#ifndef HOLTWINTERS_H
#define HOLTWINTERS_H

#include <QObject>
#include <QVector>

class HoltWinters : public QObject {
    Q_OBJECT
public:
    /** 模型类型 */
    enum class ModelType { Additive, Multiplicative };

    /** 统计 */
    struct Stats {
        quint64 totalFits = 0;
        quint64 totalForecasts = 0;
        double  avgMape = 0.0;
        double  avgProcessingTimeMs = 0.0;
    };

    /** 拟合结果 */
    struct FitResult {
        double alpha = 0.0;     ///< 水平平滑
        double beta = 0.0;      ///< 趋势平滑
        double gamma = 0.0;     ///< 季节平滑
        double mape = 0.0;      ///< 平均绝对百分比误差
        QVector<double> level;
        QVector<double> trend;
        QVector<double> season;
    };

    explicit HoltWinters(QObject* parent = nullptr);

    /** @brief 拟合模型 @param data 时间序列 @param seasonLength 季节长度 @param type 模型类型 @return 拟合结果 */
    FitResult fit(const QVector<double>& data, int seasonLength,
                  ModelType type = ModelType::Additive);

    /** @brief 预测 @param result 拟合结果 @param steps 步数 @param seasonLength 季节长度 @return 预测值 */
    QVector<double> forecast(const FitResult& result, int steps,
                              int seasonLength);

    /** @brief 自动拟合(网格搜索最优参数) @param data 时间序列 @param seasonLength 季节长度 @return 最佳拟合结果 */
    FitResult autoFit(const QVector<double>& data, int seasonLength);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void fitCompleted(double mape);
    void forecastCompleted(int steps);

private:
    double computeMape(const QVector<double>& actual,
                       const QVector<double>& fitted) const;

    mutable Stats m_stats;
    mutable double m_timeSum;
};

#endif // HOLTWINTERS_H

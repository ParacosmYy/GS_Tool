/**
 * @file TrendPredictor.h
 * @brief 趋势预测引擎 — 线性/指数/多项式趋势外推
 *
 * 功能: 拟合历史数据的趋势模型，外推预测未来值，
 *       支持线性/指数/二次/移动平均4种预测方法。
 *
 * 协作: DataReducer(降采样后预测) / DashboardModel(仪表预测)
 */
#ifndef TRENDPREDICTOR_H
#define TRENDPREDICTOR_H

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 趋势预测引擎 — 历史数据趋势外推
 */
class TrendPredictor : public QObject {
    Q_OBJECT

public:
    /** @brief 预测方法 */
    enum class PredictionMethod {
        Linear,         ///< 线性回归预测
        Exponential,    ///< 指数趋势预测
        Quadratic,      ///< 二次多项式预测
        MovingAverage   ///< 移动平均预测
    };
    Q_ENUM(PredictionMethod)

    /** @brief 预测结果 */
    struct Prediction {
        double value = 0.0;         ///< 预测值
        double lowerBound = 0.0;    ///< 置信下界
        double upperBound = 0.0;    ///< 置信上界
        double rSquared = 0.0;      ///< R²拟合度
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalPredictions = 0;       ///< 累计预测次数
        double  averageError = 0.0;         ///< 平均预测误差
        double  peakError = 0.0;            ///< 峰值误差
        double  averageRSquared = 0.0;      ///< 平均R²
    };

    explicit TrendPredictor(QObject* parent = nullptr);

    /** @brief 设置预测方法 @param method 方法 */
    void setMethod(PredictionMethod method);

    /** @brief 设置历史数据 @param data 数据 */
    void setHistory(const QVector<double>& data);

    /** @brief 设置置信区间(默认0.95) @param level 置信水平 */
    void setConfidenceLevel(double level);

    /** @brief 预测未来第N步 @param steps 步数 @return 预测结果 */
    Prediction predict(int steps = 1);

    /** @brief 批量预测多步 @param count 预测步数 @return 预测结果列表 */
    QList<Prediction> predictMultiStep(int count);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 预测完成 @param steps 预测步数 @param value 预测值 */
    void predictionReady(int steps, double value);

private:
    Prediction predictLinear(int steps);
    Prediction predictExponential(int steps);
    Prediction predictQuadratic(int steps);
    Prediction predictMA(int steps);

    double computeRSquared(const QVector<double>& actual,
                           const QVector<double>& fitted) const;

    PredictionMethod m_method;      ///< 预测方法
    QVector<double> m_history;      ///< 历史数据
    double m_confidenceLevel;       ///< 置信水平

    Stats m_stats;
    double m_errorSum;              ///< 误差累加器
    double m_rSquaredSum;           ///< R²累加器
};

#endif // TRENDPREDICTOR_H

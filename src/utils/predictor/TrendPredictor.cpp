/**
 * @file TrendPredictor.cpp
 * @brief 趋势预测引擎实现 — 线性/指数/二次/移动平均预测
 */

#include "utils/predictor/TrendPredictor.h"

#include <QtMath>

/** @brief 构造函数 @param parent 父对象 */
TrendPredictor::TrendPredictor(QObject* parent)
    : QObject(parent)
    , m_method(PredictionMethod::Linear)
    , m_confidenceLevel(0.95)
    , m_errorSum(0.0)
    , m_rSquaredSum(0.0)
{
}

/** @brief 设置预测方法 @param method 方法 */
void TrendPredictor::setMethod(PredictionMethod method)
{
    m_method = method;
}

/** @brief 设置历史数据 @param data 数据 */
void TrendPredictor::setHistory(const QVector<double>& data)
{
    m_history = data;
}

/** @brief 设置置信水平 @param level 水平 */
void TrendPredictor::setConfidenceLevel(double level)
{
    m_confidenceLevel = qBound(0.5, level, 0.999);
}

/** @brief 预测 @param steps 步数 @return 预测结果 */
TrendPredictor::Prediction TrendPredictor::predict(int steps)
{
    if (m_history.size() < 3) {
        Prediction p;
        p.value = m_history.isEmpty() ? 0.0 : m_history.last();
        return p;
    }

    Prediction result;
    switch (m_method) {
    case PredictionMethod::Linear:
        result = predictLinear(steps);
        break;
    case PredictionMethod::Exponential:
        result = predictExponential(steps);
        break;
    case PredictionMethod::Quadratic:
        result = predictQuadratic(steps);
        break;
    case PredictionMethod::MovingAverage:
        result = predictMA(steps);
        break;
    }

    ++m_stats.totalPredictions;
    m_rSquaredSum += result.rSquared;
    m_stats.averageRSquared = m_rSquaredSum
        / static_cast<double>(m_stats.totalPredictions);

    emit predictionReady(steps, result.value);
    return result;
}

/** @brief 批量预测 @param count 步数 @return 预测列表 */
QList<TrendPredictor::Prediction> TrendPredictor::predictMultiStep(int count)
{
    QList<Prediction> results;
    for (int i = 1; i <= count; ++i) {
        results.append(predict(i));
    }
    return results;
}

/** @brief 重置统计 */
void TrendPredictor::resetStatistics()
{
    m_stats = Stats{};
    m_errorSum = 0.0;
    m_rSquaredSum = 0.0;
}

/** @brief 线性预测 @param steps 步数 @return 预测 */
TrendPredictor::Prediction TrendPredictor::predictLinear(int steps)
{
    int n = m_history.size();
    double sumX = 0.0, sumY = 0.0, sumXY = 0.0, sumX2 = 0.0;
    for (int i = 0; i < n; ++i) {
        double x = static_cast<double>(i);
        sumX += x;
        sumY += m_history[i];
        sumXY += x * m_history[i];
        sumX2 += x * x;
    }

    double denom = n * sumX2 - sumX * sumX;
    double slope = (qFuzzyIsNull(denom)) ? 0.0 : (n * sumXY - sumX * sumY) / denom;
    double intercept = (sumY - slope * sumX) / n;

    Prediction p;
    p.value = intercept + slope * (n + steps - 1);

    /* 残差标准差 */
    double ssRes = 0.0;
    QVector<double> fitted;
    fitted.reserve(n);
    for (int i = 0; i < n; ++i) {
        double f = intercept + slope * i;
        fitted.append(f);
        ssRes += (m_history[i] - f) * (m_history[i] - f);
    }
    double se = qSqrt(ssRes / qMax(1, n - 2));

    double zScore = 1.96; /* 95%置信 */
    p.lowerBound = p.value - zScore * se;
    p.upperBound = p.value + zScore * se;
    p.rSquared = computeRSquared(m_history, fitted);

    return p;
}

/** @brief 指数预测 @param steps 步数 @return 预测 */
TrendPredictor::Prediction TrendPredictor::predictExponential(int steps)
{
    int n = m_history.size();
    QVector<double> logY;
    logY.reserve(n);
    for (double v : m_history) {
        logY.append((v > 0) ? qLn(v) : 0.0);
    }

    double sumX = 0.0, sumY = 0.0, sumXY = 0.0, sumX2 = 0.0;
    for (int i = 0; i < n; ++i) {
        double x = static_cast<double>(i);
        sumX += x; sumY += logY[i];
        sumXY += x * logY[i]; sumX2 += x * x;
    }

    double denom = n * sumX2 - sumX * sumX;
    double b = (qFuzzyIsNull(denom)) ? 0.0 : (n * sumXY - sumX * sumY) / denom;
    double a = qExp((sumY - b * sumX) / n);

    Prediction p;
    p.value = a * qExp(b * (n + steps - 1));
    p.rSquared = 0.5; /* 简化 */
    p.lowerBound = p.value * 0.9;
    p.upperBound = p.value * 1.1;

    return p;
}

/** @brief 二次预测 @param steps 步数 @return 预测 */
TrendPredictor::Prediction TrendPredictor::predictQuadratic(int steps)
{
    int n = m_history.size();
    /* 最小二乘拟合 y = ax² + bx + c */
    double s0 = n, s1 = 0, s2 = 0, s3 = 0, s4 = 0;
    double t0 = 0, t1 = 0, t2 = 0;

    for (int i = 0; i < n; ++i) {
        double x = static_cast<double>(i);
        double y = m_history[i];
        s1 += x; s2 += x * x; s3 += x * x * x; s4 += x * x * x * x;
        t0 += y; t1 += x * y; t2 += x * x * y;
    }

    /* 求解 3x3 线性系统 (简化Cramer法则) */
    double det = s0 * (s2 * s4 - s3 * s3) - s1 * (s1 * s4 - s3 * s2)
               + s2 * (s1 * s3 - s2 * s2);
    double cA = 0.0, cB = 0.0, cC = 0.0;

    if (!qFuzzyIsNull(det)) {
        cC = (t0 * (s2 * s4 - s3 * s3) - t1 * (s1 * s4 - s3 * s2)
            + t2 * (s1 * s3 - s2 * s2)) / det;
        cB = (s0 * (t1 * s4 - t2 * s3) - s1 * (t0 * s4 - t2 * s2)
            + s2 * (t0 * s3 - t1 * s2)) / det;
        cA = (s0 * (s2 * t2 - s3 * t1) - s1 * (s1 * t2 - s3 * t0)
            + s2 * (s1 * t1 - s2 * t0)) / det;
    }

    double x = static_cast<double>(n + steps - 1);
    Prediction p;
    p.value = cA * x * x + cB * x + cC;
    p.lowerBound = p.value * 0.85;
    p.upperBound = p.value * 1.15;
    p.rSquared = 0.5;

    return p;
}

/** @brief 移动平均预测 @param steps 步数 @return 预测 */
TrendPredictor::Prediction TrendPredictor::predictMA(int steps)
{
    int n = m_history.size();
    int windowSize = qMax(3, n / 5);
    double sum = 0.0;
    for (int i = n - windowSize; i < n; ++i) {
        sum += m_history[i];
    }
    double avg = sum / windowSize;

    Prediction p;
    p.value = avg;
    p.lowerBound = avg * 0.9;
    p.upperBound = avg * 1.1;
    p.rSquared = 0.5;

    return p;
}

/** @brief 计算R² @param actual 实际值 @param fitted 拟合值 @return R² */
double TrendPredictor::computeRSquared(
    const QVector<double>& actual, const QVector<double>& fitted) const
{
    if (actual.size() != fitted.size() || actual.isEmpty()) {
        return 0.0;
    }

    double meanY = 0.0;
    for (double v : actual) meanY += v;
    meanY /= actual.size();

    double ssTot = 0.0, ssRes = 0.0;
    for (int i = 0; i < actual.size(); ++i) {
        ssTot += (actual[i] - meanY) * (actual[i] - meanY);
        ssRes += (actual[i] - fitted[i]) * (actual[i] - fitted[i]);
    }

    if (qFuzzyIsNull(ssTot)) return 1.0;
    return qBound(0.0, 1.0 - ssRes / ssTot, 1.0);
}

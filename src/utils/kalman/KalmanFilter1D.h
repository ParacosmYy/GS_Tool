/**
 * @file KalmanFilter1D.h
 * @brief 一维卡尔曼滤波器 — 状态估计与噪声滤波
 *
 * 功能: 标准卡尔曼滤波，支持过程噪声/测量噪声可调，
 *       适用于传感器数据融合和实时信号跟踪。
 *
 * 协作: PidController(控制环) / DigitalFilter(信号滤波)
 */
#ifndef KALMANFILTER1D_H
#define KALMANFILTER1D_H

#include <QObject>

class KalmanFilter1D : public QObject {
    Q_OBJECT

public:
    /** @brief 统计 */
    struct Stats {
        quint64 totalUpdates = 0;
        quint64 totalResets = 0;
        double  peakInnovation = 0.0;
        double  averageEstimate = 0.0;
    };

    explicit KalmanFilter1D(QObject* parent = nullptr);

    /** @brief 初始化 @param initialState 初始状态 @param initialError 初始误差 */
    void initialize(double initialState, double initialError);

    /** @brief 设置过程噪声 @param q 过程噪声方差 */
    void setProcessNoise(double q);

    /** @brief 设置测量噪声 @param r 测量噪声方差 */
    void setMeasurementNoise(double r);

    /** @brief 预测步骤(无测量) */
    void predict();

    /** @brief 更新步骤(有测量) @param measurement 测量值 @return 最优估计 */
    double update(double measurement);

    /** @brief 获取当前估计 @return 状态估计 */
    double estimate() const { return m_state; }

    /** @brief 获取当前误差 @return 误差协方差 */
    double errorCovariance() const { return m_error; }

    /** @brief 获取卡尔曼增益 @return 增益 */
    double kalmanGain() const { return m_gain; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void updated(double measurement, double estimate);

private:
    double m_state;     ///< 状态估计
    double m_error;     ///< 误差协方差
    double m_q;         ///< 过程噪声方差
    double m_r;         ///< 测量噪声方差
    double m_gain;      ///< 卡尔曼增益
    double m_innovation;///< 新息(测量残差)

    double m_estSum;
    Stats m_stats;
};

#endif // KALMANFILTER1D_H

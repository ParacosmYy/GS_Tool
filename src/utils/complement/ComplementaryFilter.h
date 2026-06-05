/**
 * @file ComplementaryFilter.h
 * @brief 互补滤波器 — 多传感器融合
 *
 * 功能: 基于可调融合系数alpha实现两个传感器的互补融合。
 *       高通滤波信任高频响应快的传感器(如陀螺仪)，
 *       低通滤波信任低频漂移小的传感器(如加速度计)。
 *       适用于IMU姿态估计、多传感器数据融合等场景。
 *
 * 协作: KalmanFilter1D(卡尔曼滤波) / ExtendedKalman(非线性)
 */
#ifndef COMPLEMENTARYFILTER_H
#define COMPLEMENTARYFILTER_H

#include <QObject>
#include <QVector>

/**
 * @brief 互补滤波器 — 多传感器融合
 */
class ComplementaryFilter : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalUpdates = 0;          ///< 累计更新次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit ComplementaryFilter(QObject* parent = nullptr);

    /** @brief 设置融合系数
     *  @param alpha 融合系数[0,1]，越大越信任sensor1(高通) */
    void setAlpha(double alpha);

    /** @brief 获取融合系数 @return alpha */
    double alpha() const { return m_alpha; }

    /** @brief 单步更新
     *  @param sensor1 高频传感器值(如陀螺仪积分)
     *  @param sensor2 低频传感器值(如加速度计)
     *  @return 融合后状态 */
    double update(double sensor1, double sensor2);

    /** @brief 向量形式更新(多维传感器融合)
     *  @param sensor1 高频传感器向量
     *  @param sensor2 低频传感器向量
     *  @return 融合后状态向量 */
    QVector<double> updateVector(const QVector<double>& sensor1,
                                 const QVector<double>& sensor2);

    /** @brief 获取当前融合状态 @return 状态值 */
    double getState() const { return m_state; }

    /** @brief 获取当前多维融合状态 @return 状态向量 */
    QVector<double> getStateVector() const { return m_stateVector; }

    /** @brief 重置滤波器状态 @param initialState 初始状态 */
    void reset(double initialState = 0.0);

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 融合更新完成 @param fusedState 融合结果 */
    void updateCompleted(double fusedState);

private:
    double m_alpha;                ///< 融合系数
    double m_state;                ///< 当前一维状态
    QVector<double> m_stateVector; ///< 当前多维状态
    double m_timeSum;              ///< 处理时间累加器
    Stats  m_stats;                ///< 统计信息
};

#endif // COMPLEMENTARYFILTER_H

/**
 * @file ParticleFilter.h
 * @brief 粒子滤波器 — 序贯蒙特卡洛(SMC)非线性状态估计
 *
 * 功能: 100粒子默认配置，支持系统重采样(低方差法)，
 *       适用于非线性非高斯系统的状态估计。
 *
 * 协作: ExtendedKalman(线性近似) / KalmanFilter1D(一维高斯)
 */
#ifndef PARTICLEFILTER_H
#define PARTICLEFILTER_H

#include <QObject>
#include <QVector>
#include <functional>

/**
 * @brief 粒子滤波器 — 序贯蒙特卡洛状态估计
 */
class ParticleFilter : public QObject {
    Q_OBJECT

public:
    /** @brief 单个粒子 */
    struct Particle {
        QVector<double> state;  ///< 粒子状态向量
        double weight = 0.0;    ///< 粒子权重
    };

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalPredictions = 0;      ///< 累计预测次数
        quint64 totalUpdates = 0;          ///< 累计更新次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    /** @brief 状态转移函数: (state) -> newState */
    using StateFunc = std::function<QVector<double>(const QVector<double>&)>;

    /** @brief 观测似然函数: (state, measurement) -> likelihood */
    using ObsFunc = std::function<double(const QVector<double>&, double)>;

    explicit ParticleFilter(QObject* parent = nullptr);

    /** @brief 初始化粒子滤波器
     *  @param stateDim 状态维度
     *  @param obsDim 观测维度 */
    void initialize(int stateDim, int obsDim);

    /** @brief 预测步骤(状态转移) */
    void predict();

    /** @brief 更新步骤(观测修正)
     *  @param measurement 观测值 */
    void update(double measurement);

    /** @brief 获取当前加权平均状态 @return 状态向量 */
    QVector<double> state() const;

    /** @brief 设置状态转移函数 @param func 转移函数 */
    void setStateFunc(const StateFunc& func);

    /** @brief 设置观测似然函数 @param func 似然函数 */
    void setObsFunc(const ObsFunc& func);

    /** @brief 设置过程噪声标准差 @param sigma 噪声标准差 */
    void setProcessNoise(double sigma);

    /** @brief 获取所有粒子 @return 粒子列表 */
    const QVector<Particle>& particles() const { return m_particles; }

    /** @brief 获取有效粒子数 @return ESS */
    double effectiveSampleSize() const;

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 预测完成 @param numParticles 粒子数量 */
    void predictionCompleted(int numParticles);

    /** @brief 更新完成 @param ess 有效粒子数 */
    void updateCompleted(double ess);

private:
    /** @brief 系统重采样(低方差法) */
    void resample();

    /** @brief 归一化粒子权重 */
    void normalizeWeights();

    QVector<Particle> m_particles;   ///< 粒子集合
    int m_stateDim;                  ///< 状态维度
    int m_obsDim;                    ///< 观测维度
    double m_processNoise;           ///< 过程噪声标准差
    double m_timeSum;                ///< 处理时间累加器
    StateFunc m_stateFunc;           ///< 状态转移函数
    ObsFunc m_obsFunc;               ///< 观测似然函数
    Stats  m_stats;                  ///< 统计信息
};

#endif // PARTICLEFILTER_H

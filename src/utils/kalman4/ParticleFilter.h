/**
 * @file ParticleFilter.h
 * @brief 粒子滤波器 — 非线性非高斯状态估计
 *
 * 功能: 基于序贯蒙特卡洛(SMC)方法的粒子滤波器，通过一组加权粒子
 *       近似后验概率分布，适用于非线性非高斯系统的状态估计。
 *       支持自定义状态转移函数和观测函数。
 *
 * 协作: ExtendedKalman(线性近似) / KalmanFilter1D(线性高斯)
 */
#ifndef PARTICLEFILTER_H
#define PARTICLEFILTER_H

#include <QObject>
#include <QVector>
#include <functional>

/**
 * @brief 粒子滤波器 — 非线性非高斯状态估计
 */
class ParticleFilter : public QObject {
    Q_OBJECT

public:
    /** @brief 单个粒子 */
    struct Particle {
        QVector<double> state;  ///< 粒子状态
        double weight = 0.0;    ///< 粒子权重
    };

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalPredictions = 0;      ///< 累计预测次数
        quint64 totalUpdates = 0;          ///< 累计更新次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    /** @brief 状态转移函数类型: (state) -> newState */
    using StateFunc = std::function<QVector<double>(const QVector<double>&)>;

    /** @brief 观测函数类型: (state, measurement) -> likelihood */
    using ObsFunc = std::function<double(const QVector<double>&, double)>;

    explicit ParticleFilter(QObject* parent = nullptr);

    /** @brief 初始化粒子滤波器
     *  @param stateDim 状态维度
     *  @param numParticles 粒子数量 */
    void initialize(int stateDim, int numParticles);

    /** @brief 预测步骤(状态转移)
     *  @param stateFunc 状态转移函数 */
    void predict(const StateFunc& stateFunc);

    /** @brief 更新步骤(观测修正)
     *  @param obsFunc 观测似然函数
     *  @param measurement 观测值 */
    void update(const ObsFunc& obsFunc, double measurement);

    /** @brief 获取加权平均状态估计 @return 状态向量 */
    QVector<double> estimate() const;

    /** @brief 获取所有粒子 @return 粒子列表 */
    const QVector<Particle>& particles() const { return m_particles; }

    /** @brief 设置过程噪声标准差 @param sigma 噪声标准差 */
    void setProcessNoise(double sigma);

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
    /** @brief 系统重采样(低方差) */
    void resample();

    /** @brief 归一化粒子权重 */
    void normalizeWeights();

    QVector<Particle> m_particles;   ///< 粒子集合
    int m_stateDim;                  ///< 状态维度
    double m_processNoise;           ///< 过程噪声标准差
    double m_timeSum;                ///< 处理时间累加器
    Stats  m_stats;                  ///< 统计信息
};

#endif // PARTICLEFILTER_H

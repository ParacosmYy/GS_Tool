/**
 * @file AdaptiveLineEnhancer2.h
 * @brief 自适应线增强器(LMS/RLS滤波+干扰消除) — Adaptive Line Enhancer with LMS/RLS Filter and Interference Cancellation
 *
 * 功能: 实现自适应线增强器(ALE)，通过LMS或RLS自适应滤波器
 *       检测并增强窄带信号(谱线)、消除宽带噪声和窄带干扰。
 *
 * 协作: BinauralProcessor(双耳处理) / DiscreteHartleyTransform(DHT) / WienerFilter(维纳滤波)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 自适应线增强器
 */
class AdaptiveLineEnhancer2 : public QObject {
    Q_OBJECT

public:
    /** @brief 自适应算法类型 */
    enum Algorithm { LMS, NormalizedLMS, RLS };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSamples = 0;         ///< 累计处理采样点数
        double avgProcessingTimeUs = 0.0; ///< 平均耗时(μs)
        double lastMse = 0.0;             ///< 最近均方误差
        int filterOrder = 0;              ///< 滤波器阶数
    };

    explicit AdaptiveLineEnhancer2(QObject *parent = nullptr);
    ~AdaptiveLineEnhancer2() override;

    /** @brief 设置自适应算法 */
    void setAlgorithm(Algorithm algo);

    /** @brief 设置滤波器阶数 */
    void setFilterOrder(int order);

    /** @brief 设置LMS步长参数 */
    void setStepSize(double mu);

    /** @brief 设置延迟线长度(解相关) */
    void setDelayLine(int delay);

    /** @brief 设置RLS遗忘因子 */
    void setForgettingFactor(double lambda);

    /**
     * @brief 处理输入信号(逐样本)
     * @param input 输入信号
     * @return 增强后的信号
     */
    QVector<double> process(const QVector<double>& input);

    /** @brief 获取自适应滤波器系数 */
    QVector<double> filterCoefficients() const;

    /** @brief 获取误差信号(噪声估计) */
    QVector<double> errorSignal() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

    /** @brief 重置滤波器状态 */
    void reset();

signals:
    /** @brief 处理完成 @param samples 采样点数 @param mse 均方误差 */
    void processingCompleted(int samples, double mse);

private:
    /** @brief LMS单步更新 */
    double lmsStep(double desired, double reference);

    /** @brief NLMS单步更新 */
    double nlmsStep(double desired, double reference);

    /** @brief RLS单步更新 */
    double rlsStep(double desired, double reference);

    Algorithm m_algo = NormalizedLMS;
    int m_order = 32;
    int m_delay = 8;
    double m_mu = 0.01;
    double m_lambda = 0.99;

    QVector<double> m_weights;            ///< 滤波器权值
    QVector<double> m_delayBuffer;        ///< 延迟线缓冲
    QVector<QVector<double>> m_P;         ///< RLS逆相关矩阵
    QVector<double> m_errorHistory;       ///< 误差历史
    int m_delayIdx = 0;

    Stats m_stats;
    double m_timeSum = 0.0;
};

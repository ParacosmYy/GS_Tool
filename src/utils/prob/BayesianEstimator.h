/**
 * @file BayesianEstimator.h
 * @brief 贝叶斯参数估计器 — Beta共轭先验的伯努利推断
 *
 * 功能: 使用Beta分布作为伯努利过程的共轭先验，支持后验更新、
 *       置信区间计算、预测概率估计。适用于串口数据中的
 *       误码率估计、丢包率推断等场景。
 *
 * 协作: StatDistribution(分布拟合) / DataQualityScorer(质量评估)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <QElapsedTimer>

/**
 * @brief 贝叶斯参数估计器 — Beta共轭先验
 *
 * 使用Beta(alpha, beta)作为伯努利参数theta的先验分布，
 * 观测到successes/trials后后验仍为Beta分布，
 * 无需数值积分即可完成贝叶斯推断。
 */
class BayesianEstimator : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalUpdates = 0;          ///< 累计后验更新次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit BayesianEstimator(QObject* parent = nullptr);

    /**
     * @brief 设置Beta先验参数
     * @param alpha Beta分布alpha参数( successes + 1 )
     * @param beta  Beta分布beta参数( failures + 1 )
     */
    void setPrior(double alpha, double beta);

    /**
     * @brief 使用观测数据更新后验分布
     * @param successes 成功次数
     * @param trials    总试验次数
     */
    void update(int successes, int trials);

    /**
     * @brief 计算后验均值 E[theta]
     * @return 后验均值
     */
    double posteriorMean() const;

    /**
     * @brief 计算后验方差 Var[theta]
     * @return 后验方差
     */
    double posteriorVariance() const;

    /**
     * @brief 计算可信区间(Credible Interval)
     * @param confidence 置信水平(0~1, 如0.95)
     * @return QPair(lower, upper) 可信区间下界和上界
     */
    QPair<double, double> credibleInterval(double confidence) const;

    /**
     * @brief 预测下一次试验的成功概率
     * @param newTrials 新试验次数(默认1)
     * @return 预测成功概率
     */
    double predict(int newTrials = 1) const;

    /** @brief 获取当前alpha参数 */
    double alpha() const { return m_alpha; }

    /** @brief 获取当前beta参数 */
    double beta() const { return m_beta; }

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 后验更新完成信号 @param newMean 新后验均值 */
    void posteriorUpdated(double newMean);

private:
    /**
     * @brief 不完全Beta函数 I_x(a,b)
     * @param x 积分上限(0~1)
     * @param a alpha参数
     * @param b beta参数
     * @return 正则化不完全Beta函数值
     */
    double incompleteBeta(double x, double a, double b) const;

    /**
     * @brief 二分法求不完全Beta函数的逆
     * @param target 目标概率值
     * @param a alpha参数
     * @param b beta参数
     * @return 满足I_x(a,b)=target的x
     */
    double betaInv(double target, double a, double b) const;

    /**
     * @brief 连分数展开计算不完全Beta函数
     * @param x 积分上限
     * @param a alpha参数
     * @param b beta参数
     * @return 不完全Beta函数值
     */
    double betaCF(double x, double a, double b) const;

    double  m_alpha;           ///< Beta分布alpha参数
    double  m_beta;            ///< Beta分布beta参数
    int     m_totalSuccesses;  ///< 累计成功次数
    int     m_totalTrials;     ///< 累计试验次数

    QElapsedTimer m_timer;     ///< 计时器
    double  m_timeSum;         ///< 累计耗时
    Stats   m_stats;           ///< 统计信息
};

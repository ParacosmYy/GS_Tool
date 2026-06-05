/**
 * @file KullbackLeibler.h
 * @brief KL散度与交叉熵计算器 — 分布差异度量
 *
 * 功能: 计算KL散度、交叉熵、JS散度(Jensen-Shannon)和Shannon熵，
 *       用于比较两个概率分布的差异。适用于通信协议模式变化检测、
 *       数据流异常检测、模型选择等场景。
 *
 * 协作: EntropyCalculator(熵计算) / AnomalyDetector(异常检测)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QElapsedTimer>

/**
 * @brief KL散度与交叉熵计算器 — 分布差异度量
 *
 * 所有输入向量视为离散概率分布(自动归一化)，
 * KL散度非对称，JS散度对称，交叉熵可度量编码效率。
 */
class KullbackLeibler : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalComputations = 0;       ///< 累计计算次数
        double  avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
    };

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit KullbackLeibler(QObject* parent = nullptr);

    /**
     * @brief 计算KL散度 D_KL(P || Q) = sum P(i) * log(P(i)/Q(i))
     * @param p 分布P(自动归一化)
     * @param q 分布Q(自动归一化)
     * @return KL散度值；长度不匹配或无效输入返回NaN
     */
    double klDivergence(const QVector<double>& p,
                        const QVector<double>& q) const;

    /**
     * @brief 计算交叉熵 H(P, Q) = -sum P(i) * log(Q(i))
     * @param p 真实分布P
     * @param q 近似分布Q
     * @return 交叉熵值
     */
    double crossEntropy(const QVector<double>& p,
                        const QVector<double>& q) const;

    /**
     * @brief 计算JS散度(Jensen-Shannon), KL散度的对称版本
     * @param p 分布P
     * @param q 分布Q
     * @return JS散度值(0~1之间, 以2为底时)
     */
    double jsDivergence(const QVector<double>& p,
                        const QVector<double>& q) const;

    /**
     * @brief 计算Shannon熵 H(P) = -sum P(i) * log2(P(i))
     * @param p 概率分布
     * @return Shannon熵(bits)
     */
    double shannonEntropy(const QVector<double>& p) const;

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 计算完成信号 @param value 结果值 @param type 计算类型 */
    void computationCompleted(double value, const QString& type);

private:
    /**
     * @brief 归一化概率向量(使总和为1)
     * @param v 输入向量
     * @return 归一化后的向量
     */
    static QVector<double> normalize(const QVector<double>& v);

    /**
     * @brief 安全对数(处理零值: 返回0)
     * @param x 输入值
     * @param base 对数底
     * @return log_base(x)，x<=0时返回0
     */
    static double safeLog(double x, double base);

    mutable QElapsedTimer m_timer;   ///< 计时器
    mutable double  m_timeSum;       ///< 累计耗时
    mutable Stats   m_stats;         ///< 统计信息
};

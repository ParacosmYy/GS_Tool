/**
 * @file RenyiEntropy.h
 * @brief Rényi熵和散度计算器 — 广义信息熵度量
 *
 * 功能: 计算Rényi熵(alpha阶广义熵)、Tsallis熵、Rényi散度、
 *       互信息和条件熵。支持连续概率分布和联合分布分析。
 *
 * 协作: EntropyCalculator(Shannon熵) / StatDistribution(分布拟合)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Rényi熵和散度计算器
 */
class RenyiEntropy : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalComputations = 0;   ///< 累计计算次数
        double  avgProcessingTimeMs = 0.0;///< 平均处理时间(ms)
    };

    /** @brief 默认构造函数 @param parent 父对象 */
    explicit RenyiEntropy(QObject* parent = nullptr);

    /**
     * @brief 计算Rényi熵
     * @param probabilities 概率分布(所有值>=0, 总和应接近1)
     * @param alpha 阶数参数(>0, alpha!=1时为Rényi熵, alpha->1时退化为Shannon熵)
     * @return Rényi熵值(nats)
     *
     * alpha=0: Hartley熵(最大熵)
     * alpha=1: 极限为Shannon熵
     * alpha=2: 碰撞熵
     * alpha=∞: 最小熵
     */
    double entropy(const QVector<double>& probabilities, double alpha);

    /**
     * @brief 计算Tsallis熵
     * @param probabilities 概率分布
     * @param q 非广延参数(q!=1)
     * @return Tsallis熵值
     *
     * Tsallis熵是Rényi熵的非对数变体, q->1时退化为Shannon熵。
     */
    double tsallisEntropy(const QVector<double>& probabilities, double q);

    /**
     * @brief 计算Rényi散度(两个分布间的距离)
     * @param p 第一个概率分布
     * @param q 第二个概率分布(参考分布)
     * @param alpha 阶数参数(>0, !=1)
     * @return Rényi散度值(>=0, 0表示两分布相同)
     *
     * 两个分布必须长度相同且对齐。散度非对称: D(p||q) != D(q||p)。
     */
    double divergence(const QVector<double>& p, const QVector<double>& q,
                      double alpha);

    /**
     * @brief 计算互信息I(X;Y)
     * @param jointDist 联合概率分布矩阵(外层行=X, 内层列=Y)
     * @return 互信息值(nats)
     *
     * I(X;Y) = H(X) + H(Y) - H(X,Y), 衡量两个变量的统计依赖程度。
     */
    double mutualInformation(const QVector<QVector<double>>& jointDist);

    /**
     * @brief 计算条件熵H(Y|X)
     * @param jointDist 联合概率分布矩阵
     * @return 条件熵值(nats)
     *
     * H(Y|X) = H(X,Y) - H(X), 给定X后Y的不确定性。
     */
    double conditionalEntropy(const QVector<QVector<double>>& jointDist);

    /** @brief 获取统计 @return 统计信息常引用 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 计算完成 @param value 结果值 @param type 计算类型 */
    void computationCompleted(double value, const QString& type);

private:
    /**
     * @brief 计算Shannon熵(自然对数底)
     * @param probabilities 概率分布
     * @return Shannon熵(nats)
     */
    double shannon(const QVector<double>& probabilities) const;

    Stats  m_stats;                ///< 统计信息
    double m_timeSum;              ///< 处理时间累加器
};

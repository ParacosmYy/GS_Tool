/**
 * @file EarthMoverDistance.h
 * @brief 推土机距离(Wasserstein) — 分布相似度度量
 *
 * 功能: 计算两个概率分布之间的推土机距离(EMD)，
 *       也称Wasserstein距离，用于分布比较和相似度度量。
 *
 * 协作: RenyiEntropy(信息度量) / GaussianMixture(分布拟合)
 */
#ifndef EARTHMOVERDISTANCE_H
#define EARTHMOVERDISTANCE_H

#include <QObject>
#include <QVector>

/**
 * @brief 推土机距离计算器
 */
class EarthMoverDistance : public QObject {
    Q_OBJECT

public:
    /** @brief 统计 */
    struct Stats {
        quint64 totalComputations = 0;     ///< 累计计算次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit EarthMoverDistance(QObject* parent = nullptr);

    /**
     * @brief 计算两个分布的EMD
     * @param dist1 第一个概率分布(非负，自动归一化)
     * @param dist2 第二个概率分布(非负，自动归一化)
     * @return 推土机距离
     */
    double compute(const QVector<double>& dist1,
                   const QVector<double>& dist2);

    /**
     * @brief 计算带权重的EMD
     * @param values1 第一个分布的值位置
     * @param weights1 第一个分布的权重
     * @param values2 第二个分布的值位置
     * @param weights2 第二个分布的权重
     * @return 加权推土机距离
     */
    double computeWeighted(const QVector<double>& values1,
                           const QVector<double>& weights1,
                           const QVector<double>& values2,
                           const QVector<double>& weights2);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 计算完成 @param distance 推土机距离 */
    void computationCompleted(double distance);

private:
    /** @brief 归一化分布使权重和为1 */
    static QVector<double> normalize(const QVector<double>& dist);

    Stats m_stats;                   ///< 统计信息
    double m_timeSum = 0.0;          ///< 累计耗时
};

#endif // EARTHMOVERDISTANCE_H

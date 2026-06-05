/**
 * @file MinMaxScaler.h
 * @brief Min-Max特征缩放 — 归一化到指定范围
 *
 * 功能: 将数据线性映射到目标范围(默认[0,1])，
 *       支持fit/transform/inverseTransform三步流程。
 *
 * 协作: ZScoreNormalizer(Z-Score标准化) / DataNormalizer(通用归一化)
 */
#ifndef MINMAXSCALER_H
#define MINMAXSCALER_H

#include <QObject>
#include <QVector>

/**
 * @brief Min-Max特征缩放器
 */
class MinMaxScaler : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalTransforms = 0;     ///< 累计变换次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit MinMaxScaler(QObject* parent = nullptr);

    /** @brief 拟合数据: 计算最小/最大值
     *  @param data 输入数据 */
    void fit(const QVector<double>& data);

    /** @brief 变换数据: 缩放到目标范围
     *  @param data 输入数据
     *  @return 缩放后的数据 */
    QVector<double> transform(const QVector<double>& data);

    /** @brief 逆变换: 从目标范围还原到原始范围
     *  @param data 缩放后的数据
     *  @return 还原后的数据 */
    QVector<double> inverseTransform(const QVector<double>& data);

    /** @brief 设置目标范围
     *  @param min 目标最小值 @param max 目标最大值 */
    void setTargetRange(double min, double max);

    /** @brief 获取拟合的最小值 @return 最小值 */
    double dataMin() const { return m_dataMin; }

    /** @brief 获取拟合的最大值 @return 最大值 */
    double dataMax() const { return m_dataMax; }

    /** @brief 是否已拟合 @return 是否已拟合 */
    bool isFitted() const { return m_fitted; }

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 拟合完成 @param min 数据最小值 @param max 数据最大值 */
    void fitCompleted(double min, double max);

    /** @brief 变换完成 @param count 数据点数 */
    void transformCompleted(int count);

private:
    double m_dataMin;      ///< 数据最小值
    double m_dataMax;      ///< 数据最大值
    double m_targetMin;    ///< 目标范围最小值
    double m_targetMax;    ///< 目标范围最大值
    bool   m_fitted;       ///< 是否已拟合
    double m_timeSum;      ///< 处理时间累加器
    Stats  m_stats;        ///< 统计信息
};

#endif // MINMAXSCALER_H

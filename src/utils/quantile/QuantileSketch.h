/**
 * @file QuantileSketch.h
 * @brief 分位数草图 — 流式近似分位数计算
 *
 * 提供基于GK算法(Greenwald-Khanna)的流式近似分位数计算,
 * 支持合并操作, 适用于嵌入式调试场景中的
 * 数据质量监控和实时统计摘要。
 */
#ifndef QUANTILE_SKETCH_H
#define QUANTILE_SKETCH_H

#include <QObject>
#include <QVector>
#include <QMultiMap>

/**
 * @class QuantileSketch
 * @brief 流式近似分位数草图
 *
 * 典型用法:
 * @code
 *   QuantileSketch sketch;
 *   sketch.setEpsilon(0.01);
 *   sketch.add(3.14);
 *   double p50 = sketch.query(0.5);
 * @endcode
 */
class QuantileSketch : public QObject {
    Q_OBJECT

public:
    /** @brief 操作统计结构 */
    struct Stats {
        quint64 totalAdds = 0;              ///< 添加操作总次数
        quint64 totalQueries = 0;           ///< 查询操作总次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    /** @brief 草图内部元组(v, g, delta) */
    struct Tuple {
        double value;   ///< 观测值
        int g;          ///< 该值与前一值的最大间距下界
        int delta;      ///< 该值与前一值的最大间距上界增量
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit QuantileSketch(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~QuantileSketch() override;

    // ── 配置 ──

    /**
     * @brief 设置近似误差范围
     * @param epsilon 误差参数, 范围(0, 1), 默认0.01
     */
    void setEpsilon(double epsilon);

    /** @brief 获取当前误差参数 */
    double epsilon() const;

    // ── 操作 ──

    /**
     * @brief 向草图中添加一个观测值
     * @param value 观测值
     */
    void add(double value);

    /**
     * @brief 查询指定分位数的近似值
     * @param percentile 百分位, 范围[0, 1]
     * @return 近似分位数值
     */
    double query(double percentile);

    /**
     * @brief 合并另一个草图到当前草图
     * @param other 待合并的草图指针
     */
    void merge(const QuantileSketch& other);

    // ── 查询 ──

    /** @brief 已添加的观测值总数 */
    quint64 count() const;

    // ── 统计 ──

    /** @brief 获取当前统计数据快照 */
    Stats stats() const;

    /** @brief 重置所有统计计数器归零 */
    void resetStatistics();

signals:
    /** @brief 添加完成信号 @param value 添加的值 */
    void valueAdded(double value);
    /** @brief 查询完成信号 @param percentile 查询百分位 @param result 结果值 */
    void queryCompleted(double percentile, double result);
    /** @brief 合并完成信号 @param count 合并后的总数 */
    void mergeCompleted(quint64 count);

private:
    /**
     * @brief 压缩摘要中的冗余元组
     * @param band 当前band宽度
     */
    void compress(int band);

    /**
     * @brief 计算band值
     * @param delta delta值
     * @return band编号
     */
    int band(int delta) const;

    /**
     * @brief 更新平均处理时间
     * @param elapsedMs 本次耗时(ms)
     */
    void updateAvgTime(double elapsedMs) const;

    /** @brief 摘要元组列表 */
    QVector<Tuple> m_summary;

    /** @brief 已观测值总数 */
    quint64 m_count = 0;

    /** @brief 误差参数 */
    double m_epsilon = 0.01;

    /** @brief 操作统计(mutable支持const方法更新) */
    mutable Stats m_stats;
};

#endif // QUANTILE_SKETCH_H

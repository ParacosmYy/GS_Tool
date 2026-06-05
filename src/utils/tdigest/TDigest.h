/**
 * @file TDigest.h
 * @brief T-Digest分位数估计器 — 流式分位数/百分位数/合并
 *
 * 提供T-Digest数据结构的完整实现, 支持流式数据插入、
 * 分位数查询(p50/p90/p99等)、自动合并压缩、多Digest合并。
 * 适用于嵌入式调试中的实时性能监控和延迟分布统计。
 */
#ifndef T_DIGEST_H
#define T_DIGEST_H

#include <QObject>
#include <QVector>
#include <QByteArray>

/**
 * @class TDigest
 * @brief T-Digest流式分位数估计器
 *
 * 基于合并排序的T-Digest实现, 空间复杂度O(δ),
 * 其中δ控制精度和内存的权衡。
 */
class TDigest : public QObject {
    Q_OBJECT

public:
    /** @brief 质心结构 */
    struct Centroid {
        double mean = 0.0;    ///< 质心均值
        double weight = 0.0;  ///< 质心权重(数据点数量)
    };

    /** @brief 操作统计结构 */
    struct Stats {
        quint64 totalAdds = 0;          ///< 添加操作总次数
        quint64 totalQueries = 0;       ///< 查询操作总次数
        quint64 totalMerges = 0;        ///< 合并操作总次数
        double  minValue = 0.0;         ///< 数据最小值
        double  maxValue = 0.0;         ///< 数据最大值
        double  totalWeight = 0.0;      ///< 总权重(数据点总数)
    };

    /**
     * @brief 构造函数
     * @param delta 压缩参数(默认300, 越大越精确)
     * @param parent 父对象
     */
    explicit TDigest(double delta = 300.0, QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~TDigest() override;

    // ── 数据操作 ──

    /**
     * @brief 添加一个数据点
     * @param value 数据值
     * @param weight 权重(默认1.0)
     */
    void add(double value, double weight = 1.0);

    /**
     * @brief 批量添加数据点
     * @param values 数据值列表
     */
    void addBatch(const QVector<double>& values);

    /**
     * @brief 合并另一个T-Digest
     * @param other 另一个T-Digest实例
     */
    void merge(const TDigest& other);

    // ── 查询 ──

    /**
     * @brief 查询分位数值
     * @param q 分位数(0.0~1.0), 如0.5为中位数
     * @return 对应分位数的值
     */
    double quantile(double q);

    /**
     * @brief 查询百分位数
     * @param p 百分位(0~100), 如50为中位数
     * @return 对应百分位的值
     */
    double percentile(double p);

    /** @brief 获取中位数 */
    double median();

    /** @brief 获取截尾均值(去掉最高最低各trim比例) */
    double trimmedMean(double trim);

    // ── 属性查询 ──

    /** @brief 获取数据最小值 */
    double minimum() const;

    /** @brief 获取数据最大值 */
    double maximum() const;

    /** @brief 获取总权重(数据点总数) */
    double totalWeight() const;

    /** @brief 获取质心数量 */
    int centroidCount() const;

    /** @brief 判断是否为空 */
    bool isEmpty() const;

    // ── 序列化 ──

    /** @brief 导出为字节数组 */
    QByteArray serialize() const;

    /** @brief 从字节数组导入 */
    bool deserialize(const QByteArray& data);

    // ── 统计 ──

    /** @brief 获取当前统计数据快照 */
    Stats stats() const;

    /** @brief 重置统计计数器 */
    void resetStatistics();

    /** @brief 重置所有数据 */
    void reset();

signals:
    /** @brief 分位数查询信号 @param q 分位数 @param value 对应值 */
    void quantileQueried(double q, double value);
    /** @brief 错误信号 @param msg 错误描述 */
    void error(const QString& msg);

private:
    /** @brief 压缩质心(合并排序) */
    void compress();

    /** @brief 计算质心的k0大小(用于排序) */
    double kSize(double q) const;

    /** @brief 查找分位数对应的值(在已压缩的质心上) */
    double interpolateQuantile(double q) const;

    double m_delta;                     ///< 压缩参数
    QVector<Centroid> m_centroids;      ///< 质心列表
    double m_minValue;                  ///< 数据最小值
    double m_maxValue;                  ///< 数据最大值
    double m_totalWeight;               ///< 总权重
    bool m_compressed;                  ///< 是否已压缩

    mutable Stats m_stats;              ///< 操作统计
};

#endif // T_DIGEST_H

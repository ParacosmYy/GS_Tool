/**
 * @file HyperLogLog.h
 * @brief HyperLogLog基数估计器 — 稀疏/密集表示/基数估计
 *
 * 提供HyperLogLog(HLL)算法的完整实现, 支持稀疏表示(小基数优化)、
 * 密集表示(标准HLL)、稀疏到密集的自动转换、多HLL合并。
 * 适用于嵌入式调试中的网络流量去重、唯一标识符计数等场景。
 */
#ifndef HYPER_LOG_LOG_H
#define HYPER_LOG_LOG_H

#include <QObject>
#include <QByteArray>
#include <QVector>
#include <QSet>

/**
 * @class HyperLogLog
 * @brief HyperLogLog基数估计器
 *
 * 使用 O(m) 空间估计 n 个不同元素的数量, 标准误差约 1.04/√m。
 * m=2^p, p为精度参数(4~18)。
 */
class HyperLogLog : public QObject {
    Q_OBJECT

public:
    /** @brief 操作统计结构 */
    struct Stats {
        quint64 totalAdds = 0;          ///< 添加操作总次数
        quint64 totalMerges = 0;        ///< 合并操作总次数
        quint64 totalQueries = 0;       ///< 查询操作总次数
        double  lastEstimate = 0.0;     ///< 最近一次基数估计
    };

    /**
     * @brief 构造函数
     * @param precision 精度参数p(4~18), 默认12
     * @param parent 父对象
     */
    explicit HyperLogLog(int precision = 12, QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~HyperLogLog() override;

    // ── 数据操作 ──

    /**
     * @brief 添加一个元素
     * @param data 元素数据(字节数组)
     */
    void add(const QByteArray& data);

    /**
     * @brief 添加一个32位整数
     * @param value 整数值
     */
    void addInt(quint32 value);

    /**
     * @brief 合并另一个HLL(并集)
     * @param other 另一个HyperLogLog实例
     */
    void merge(const HyperLogLog& other);

    // ── 查询 ──

    /**
     * @brief 估计当前基数(不同元素数量)
     * @return 基数估计值
     */
    double cardinality();

    /** @brief 判断是否为空 */
    bool isEmpty() const;

    // ── 序列化 ──

    /** @brief 导出为字节数组(可序列化存储) */
    QByteArray serialize() const;

    /**
     * @brief 从字节数组反序列化
     * @param data 序列化数据
     * @return true表示成功
     */
    bool deserialize(const QByteArray& data);

    // ── 配置 ──

    /** @brief 获取精度参数 */
    int precision() const;

    /** @brief 获取寄存器数量 m = 2^p */
    int registerCount() const;

    /** @brief 重置所有寄存器 */
    void reset();

    // ── 统计 ──

    /** @brief 获取当前统计数据快照 */
    Stats stats() const;

    /** @brief 重置统计计数器 */
    void resetStatistics();

signals:
    /** @brief 基数更新信号 @param estimate 当前估计值 */
    void cardinalityUpdated(double estimate);
    /** @brief 错误信号 @param msg 错误描述 */
    void error(const QString& msg);

private:
    /** @brief MurmurHash3 (32-bit) */
    static quint32 murmurHash3(const QByteArray& data, quint32 seed);

    /** @brief 计算前导零个数 + 1 */
    static int rho(quint32 w, int bits);

    /** @brief 稀疏表示转换为密集表示 */
    void promoteToDense();

    /** @brief 计算原始估计值(密集模式) */
    double rawEstimate() const;

    /** @brief 计算alpha修正因子 */
    double alpha(int m) const;

    int m_precision;                    ///< 精度参数p
    int m_registerCount;                ///< 寄存器数量 m=2^p
    QVector<quint8> m_registers;        ///< 密集表示: 寄存器数组
    QSet<quint32> m_sparseSet;          ///< 稀疏表示: 非零寄存器集合
    bool m_isDense;                      ///< 是否使用密集表示
    quint64 m_sparseLimit;              ///< 稀疏→密集转换阈值

    mutable Stats m_stats;              ///< 操作统计
};

#endif // HYPER_LOG_LOG_H

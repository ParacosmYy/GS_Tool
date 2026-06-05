/**
 * @file PolynomialHash.h
 * @brief 多项式滚动哈希 — Rabin指纹变体
 *
 * 功能: 基于多项式滚动哈希的Rabin指纹算法，支持滑动窗口
 *       O(1)增量更新，适用于数据流去重和字符串匹配。
 *
 * 协作: DataDeduplicator(数据去重) / BytePatternAnalyzer(模式检测)
 */
#ifndef POLYNOMIALHASH_H
#define POLYNOMIALHASH_H

#include <QObject>
#include <QByteArray>

/**
 * @brief 多项式滚动哈希计算器 — Rabin指纹变体
 */
class PolynomialHash : public QObject {
    Q_OBJECT

public:
    /** @brief 运行时统计 */
    struct Stats {
        quint64 totalHashes       = 0;   ///< 累计完整哈希次数
        quint64 totalRolls        = 0;   ///< 累计滚动更新次数
        quint64 totalBytesProcessed = 0; ///< 累计处理字节数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit PolynomialHash(QObject* parent = nullptr);

    /** @brief 设置多项式基数 @param base 基数(默认257) */
    void setBase(quint64 base = 257);

    /** @brief 设置模数 @param mod 模数(默认1e9+7) */
    void setModulo(quint64 mod = 1000000007ULL);

    /**
     * @brief 计算完整哈希
     * @param data 输入数据
     * @return 多项式哈希值
     */
    quint64 hash(const QByteArray& data);

    /**
     * @brief 滚动哈希 — O(1)窗口滑动
     * @param prevHash 前一次哈希值
     * @param outChar 窗口移出的字符
     * @param inChar 窗口移入的字符
     * @param windowSize 滑动窗口大小
     * @return 更新后的哈希值
     */
    quint64 roll(quint64 prevHash, char outChar, char inChar, int windowSize);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 哈希计算完成信号 @param inputSize 输入数据大小 */
    void hashComputed(qint64 inputSize);

private:
    /**
     * @brief 快速模幂运算
     * @param base 底数
     * @param exp  指数
     * @return (base^exp) % m_mod
     */
    quint64 powerMod(quint64 base, quint64 exp) const;

    Stats   m_stats;
    quint64 m_base;       ///< 多项式基数
    quint64 m_mod;        ///< 模数
    double  m_timeSumMs;  ///< 累计耗时(用于计算均值)
};

#endif // POLYNOMIALHASH_H

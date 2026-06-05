#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief Turbo码编码解码器
 *
 * 并行级联卷积码(PCCC)实现,利用迭代软判决解码
 * 获得接近香农极限的纠错性能,适用于深空通信与3G/4G无线。
 */
class TurboCode7 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalEncoded = 0; double avgProcessingTimeMs = 0.0; };

    explicit TurboCode7(QObject* parent = nullptr);

    /** @brief 设置迭代解码次数 */
    void setIterations(int iterations);

    /** @brief 对比特序列执行Turbo编码 */
    void encode(const QVector<int>& bits);

    /** @brief 对软信息执行迭代Turbo解码 */
    void decode(const QVector<double>& llr);

    /** @brief 获取当前统计信息 */
    Stats stats() const { return m_stats; }

    /** @brief 重置统计数据 */
    void resetStatistics();

signals:
    /** @brief 编解码完成信号 */
    void codingCompleted(int bitCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_iterations = 8;
};

#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief Spinal码编码解码器
 *
 * 基于哈希函数的速率兼容码,利用脊柱(Spine)结构
 * 实现渐进式编解码,适用于无线通信中的可靠传输。
 */
class SpinalCode9 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalEncoded = 0; double avgProcessingTimeMs = 0.0; };

    explicit SpinalCode9(QObject* parent = nullptr);

    /** @brief 设置脊柱长度 */
    void setSpineLength(int length);

    /** @brief 对整数序列执行Spinal编码 */
    void encode(const QVector<int>& data);

    /** @brief 对软信息执行Spinal解码 */
    void decode(const QVector<double>& symbols);

    /** @brief 获取当前统计信息 */
    Stats stats() const { return m_stats; }

    /** @brief 重置统计数据 */
    void resetStatistics();

signals:
    /** @brief 编解码完成信号 */
    void codingCompleted(int symbolCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_spineLength = 16;
};

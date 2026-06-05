#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Spinal码编解码器
 *
 * 基于哈希函数的无率码(Rateless Code)实现，
 * 通过脊柱(Spine)结构将消息映射为无限编码符号流。
 */
class SpinalCode10 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalCoded = 0; double avgProcessingTimeMs = 0.0; };

    explicit SpinalCode10(QObject* parent = nullptr);

    /** @brief 设置脊柱长度 */
    void setSpineLength(int length);

    /** @brief 编码整数序列 */
    QVector<int> encode(const QVector<int>& data);

    /** @brief 解码为整数序列 */
    QVector<int> decode(const QVector<double>& symbols);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 编解码完成信号 */
    void codingCompleted(int symbolCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_spineLength = 32;
};

/**
 * @file CountSketch.h
 * @brief Count-Sketch频率估计算法
 */

#pragma once

#include <QObject>
#include <QByteArray>
#include <QVector>
#include <QString>

/**
 * @class CountSketch
 * @brief Count-Sketch — 线性时间频率估计和Heavy Hitter检测
 *
 * 使用d×w的计数器矩阵和两两独立哈希函数族。
 * 适用于数据流中高频元素检测和频率估计。
 */
class CountSketch : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalUpdates = 0;       /**< 总更新次数 */
        int totalQueries = 0;       /**< 总查询次数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /**
     * @brief 构造函数
     * @param width 每行计数器数(默认1024)
     * @param depth 行数(哈希函数数，默认5)
     * @param parent 父对象
     */
    explicit CountSketch(int width = 1024, int depth = 5,
                          QObject* parent = nullptr);

    /** @brief 更新: 添加元素 */
    void update(const QByteArray& item, long long count = 1);

    /** @brief 更新: 添加字符串 */
    void updateString(const QString& item, long long count = 1);

    /** @brief 查询: 估计元素频率 */
    long long estimate(const QByteArray& item) const;

    /** @brief 查询: 估计字符串频率 */
    long long estimateString(const QString& item) const;

    /**
     * @brief 批量更新
     * @param items 元素列表
     * @param counts 对应计数
     */
    void batchUpdate(const QVector<QByteArray>& items,
                      const QVector<long long>& counts);

    /** @brief 重置 */
    void reset();

    /** @brief 获取宽度 */
    int width() const;

    /** @brief 获取深度 */
    int depth() const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 更新完成信号 */
    void updated(const QByteArray& item, long long count);

private:
    qint64 hash(const QByteArray& data, int row) const;
    int signHash(const QByteArray& data, int row) const;

    int m_width;
    int m_depth;
    QVector<QVector<long long>> m_counts;

    Stats m_stats;
    double m_timeSum;
};

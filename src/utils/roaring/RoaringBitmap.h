/**
 * @file RoaringBitmap.h
 * @brief Roaring位图 — 压缩位集
 *
 * 功能: Roaring bitmap压缩位图，支持AND/OR/XOR集合运算，
 *       基数统计，范围查询，统计操作次数/耗时。
 */
#ifndef ROARINGBITMAP_H
#define ROARINGBITMAP_H

#include <QObject>
#include <QSet>
#include <QVector>
#include <QSharedData>

class RoaringBitmap : public QObject {
    Q_OBJECT
public:
    /** 统计 */
    struct Stats {
        quint64 totalAdds = 0;
        quint64 totalRemoves = 0;
        quint64 totalSetOps = 0;     ///< 集合运算次数
        quint64 cardinality = 0;
        int     containerCount = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit RoaringBitmap(QObject* parent = nullptr);

    /** @brief 添加值 @param value 值 */
    void add(quint32 value);

    /** @brief 批量添加 @param values 值列表 */
    void addMany(const QVector<quint32>& values);

    /** @brief 移除值 @param value 值 */
    void remove(quint32 value);

    /** @brief 是否包含 @param value 值 @return 包含 */
    bool contains(quint32 value) const;

    /** @brief 基数(元素个数) @return 数量 */
    quint64 cardinality() const;

    /** @brief 是否为空 */
    bool isEmpty() const { return m_data.isEmpty(); }

    /** @brief 位或运算 @param other 另一个位图 @return 新位图 */
    RoaringBitmap* bitOr(const RoaringBitmap& other);

    /** @brief 位与运算 @param other 另一个位图 @return 新位图 */
    RoaringBitmap* bitAnd(const RoaringBitmap& other);

    /** @brief 位异或运算 @param other 另一个位图 @return 新位图 */
    RoaringBitmap* bitXor(const RoaringBitmap& other);

    /** @brief 范围查询 [min, max] @return 值列表 */
    QVector<quint32> rangeQuery(quint32 minVal, quint32 maxVal) const;

    /** @brief 导出所有值 @return 排序列表 */
    QVector<quint32> toVector() const;

    /** @brief 清空 */
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void valueAdded(quint32 value);
    void valueRemoved(quint32 value);
    void setOperationCompleted(const QString& op, quint64 resultCardinality);

private:
    QSet<quint32> m_data;
    mutable Stats m_stats;
    mutable double m_timeSum;
};

#endif // ROARINGBITMAP_H

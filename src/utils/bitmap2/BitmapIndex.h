/**
 * @file BitmapIndex.h
 * @brief 位图索引(Bitmap Index)
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QByteArray>

/**
 * @class BitmapIndex
 * @brief 位图索引 — 用于高效位运算的位集合
 *
 * 支持大规模位运算(AND/OR/XOR/NOT)、位计数、范围操作。
 * 适用于数据库位图索引、布隆过滤器、集合运算等。
 */
class BitmapIndex : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalOperations = 0;    /**< 总运算次数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /**
     * @brief 构造函数
     * @param size 位数(默认65536)
     * @param parent 父对象
     */
    explicit BitmapIndex(int size = 65536, QObject* parent = nullptr);

    /** @brief 设置位 */
    void set(int index);

    /** @brief 清除位 */
    void clear(int index);

    /** @brief 测试位 */
    bool test(int index) const;

    /** @brief 翻转位 */
    void flip(int index);

    /** @brief AND运算 */
    BitmapIndex* operatorAnd(const BitmapIndex& other) const;

    /** @brief OR运算 */
    BitmapIndex* operatorOr(const BitmapIndex& other) const;

    /** @brief XOR运算 */
    BitmapIndex* operatorXor(const BitmapIndex& other) const;

    /** @brief NOT运算 */
    BitmapIndex* operatorNot() const;

    /** @brief 计数(1的位数) */
    int count() const;

    /** @brief 是否为空(全0) */
    bool isEmpty() const;

    /** @brief 范围设置[start, end) */
    void setRange(int start, int end);

    /** @brief 清除范围 */
    void clearRange(int start, int end);

    /** @brief 大小 */
    int size() const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 运算完成信号 */
    void operationCompleted(const QString& opName, int resultCount);

private:
    QByteArray m_data;
    int m_size;

    Stats m_stats;
    double m_timeSum;
};

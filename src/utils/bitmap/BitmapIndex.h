/**
 * @file BitmapIndex.h
 * @brief 位图索引引擎 — 高效位操作/集合运算/人口计数
 *
 * 提供大规模位图索引的完整操作: 置位/清零/测试/翻转、
 * AND/OR/XOR集合运算、人口计数(popcount)、范围查询等,
 * 适用于嵌入式调试中的数据过滤和快速位集操作。
 */
#ifndef BITMAP_INDEX_H
#define BITMAP_INDEX_H

#include <QObject>
#include <QByteArray>
#include <QVector>

/**
 * @class BitmapIndex
 * @brief 位图索引引擎
 *
 * 基于QByteArray的紧凑位图, 支持高速位操作和集合运算。
 */
class BitmapIndex : public QObject {
    Q_OBJECT

public:
    /** @brief 操作统计结构 */
    struct Stats {
        quint64 totalOperations = 0;    ///< 操作总次数
        quint64 totalBitOps = 0;        ///< 位操作总次数
        quint64 totalSetBits = 0;       ///< 置位操作总次数
        quint64 totalClearBits = 0;     ///< 清零操作总次数
        double  avgPopcount = 0.0;      ///< 平均人口计数
    };

    /**
     * @brief 构造函数
     * @param size 位图位数(自动对齐到64)
     * @param parent 父对象
     */
    explicit BitmapIndex(qsizetype size = 256, QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~BitmapIndex() override;

    // ── 位操作 ──

    /** @brief 设置指定位为1 @param pos 位位置 */
    void setBit(qsizetype pos);

    /** @brief 设置指定位为0 @param pos 位位置 */
    void clearBit(qsizetype pos);

    /** @brief 测试指定位 @param pos 位位置 @return true表示该位为1 */
    bool testBit(qsizetype pos) const;

    /** @brief 翻转指定位 @param pos 位位置 */
    void toggleBit(qsizetype pos);

    /** @brief 设置范围位[begin,end)为1 */
    void setRange(qsizetype begin, qsizetype end);

    /** @brief 清除范围位[begin,end)为0 */
    void clearRange(qsizetype begin, qsizetype end);

    // ── 集合运算 ──

    /**
     * @brief 与另一个位图做AND运算
     * @param other 另一个位图(长度必须相同)
     */
    void bitwiseAnd(const BitmapIndex& other);

    /**
     * @brief 与另一个位图做OR运算
     * @param other 另一个位图
     */
    void bitwiseOr(const BitmapIndex& other);

    /**
     * @brief 与另一个位图做XOR运算
     * @param other 另一个位图
     */
    void bitwiseXor(const BitmapIndex& other);

    /** @brief 按位取反 */
    void bitwiseNot();

    // ── 查询 ──

    /** @brief 人口计数(统计1的个数) */
    qsizetype popcount() const;

    /** @brief 查找第一个为1的位 @return 位置, -1表示未找到 */
    qsizetype findFirst() const;

    /** @brief 查找下一个为1的位 @param pos 起始位置 @return 位置, -1表示未找到 */
    qsizetype findNext(qsizetype pos) const;

    /** @brief 获取位图位数 */
    qsizetype size() const;

    /** @brief 获取位图字节数 */
    qsizetype byteSize() const;

    /** @brief 判断位图是否为空(全0) */
    bool isEmpty() const;

    // ── 序列化 ──

    /** @brief 导出为字节数组 */
    QByteArray toByteArray() const;

    /**
     * @brief 从字节数组导入
     * @param data 字节数组
     * @param bitCount 位数
     */
    void fromByteArray(const QByteArray& data, qsizetype bitCount);

    // ── 统计 ──

    /** @brief 获取当前统计数据快照 */
    Stats stats() const;

    /** @brief 重置所有统计计数器归零 */
    void resetStatistics();

    /** @brief 清空位图(全部置0) */
    void clear();

signals:
    /** @brief 操作完成信号 @param op 操作描述 @param count 影响位数 */
    void operationCompleted(const QString& op, qsizetype count);
    /** @brief 错误信号 @param msg 错误描述 */
    void error(const QString& msg);

private:
    /** @brief 确保位图容量足够 */
    void ensureCapacity(qsizetype pos);

    /** @brief 内联人口计数(单个quint64) */
    static int popcount64(quint64 v);

    QByteArray m_data;          ///< 位图数据存储
    qsizetype m_size;           ///< 位图位数
    mutable Stats m_stats;      ///< 操作统计
};

#endif // BITMAP_INDEX_H

/**
 * @file RadixSort.h
 * @brief 基数排序 — LSD/MSD双模式，可配置基数与键提取
 *
 * 功能:
 *   - LSD(最低位优先)基数排序: 稳定排序，O(d*(n+b))复杂度
 *   - MSD(最高位优先)基数排序: 递归分桶，适合不等长键
 *   - 可配置基数(2/4/8/10/16/256等)
 *   - 自定义键提取器，支持任意数据类型的排序
 *   - 统计排序次数、元素数、处理时间
 */

#pragma once

#include <QObject>
#include <QVector>
#include <functional>

/**
 * @class RadixSort
 * @brief 基数排序引擎 — LSD与MSD双模式实现
 *
 * 通过模板化的键提取器支持整数、浮点数、字符串等多种数据类型。
 * LSD模式使用计数排序作为子程序，保证稳定性；
 * MSD模式递归处理高位到低位，支持不等长数据。
 */
class RadixSort : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalSorts = 0;           /**< 总排序次数 */
        int totalElementsSorted = 0;  /**< 总排序元素数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 键提取器类型: 从元素中提取用于排序的无符号整数值 */
    using KeyExtractor = std::function<quint64(const QVariant& item, int digitIndex)>;

    /**
     * @brief 构造函数
     * @param radix 排序基数(默认256，即按字节排序)
     * @param parent 父对象
     */
    explicit RadixSort(int radix = 256, QObject* parent = nullptr);

    /**
     * @brief LSD基数排序(稳定)
     * @param data 待排序数据
     * @param keyExtractor 键提取函数，返回指定数位上的值
     * @param maxDigits 最大位数(排序趟数)
     * @return 排序后的数据
     */
    QVector<QVariant> sortLSD(const QVector<QVariant>& data,
                               KeyExtractor keyExtractor,
                               int maxDigits) const;

    /**
     * @brief MSD基数排序(递归分桶)
     * @param data 待排序数据
     * @param keyExtractor 键提取函数
     * @param maxDigits 最大位数
     * @return 排序后的数据
     */
    QVector<QVariant> sortMSD(const QVector<QVariant>& data,
                               KeyExtractor keyExtractor,
                               int maxDigits) const;

    /**
     * @brief 便捷方法: 对整数向量进行LSD排序
     * @param values 整数向量
     * @return 排序后的向量
     */
    QVector<qint64> sortIntegers(const QVector<qint64>& values) const;

    /**
     * @brief 获取统计信息
     */
    Stats stats() const;

    /**
     * @brief 重置统计信息
     */
    void resetStatistics();

    /** @brief 获取当前基数 */
    int radix() const { return m_radix; }

    /** @brief 设置基数 */
    void setRadix(int radix);

signals:
    /** @brief 排序完成信号 */
    void sortCompleted(int elementCount, double elapsedMs);

private:
    /** @brief LSD单趟计数排序 */
    void countingSort(QVector<QVariant>& data, KeyExtractor& extractor,
                      int digitIndex) const;

    /** @brief MSD递归辅助 */
    void msdRecursive(QVector<QVariant>& data, KeyExtractor& extractor,
                      int digitIndex, int maxDigits) const;

    mutable Stats m_stats;      /**< 统计信息 */
    mutable double m_timeSum = 0.0; /**< 累计时间 */
    int m_radix;                /**< 排序基数 */
};

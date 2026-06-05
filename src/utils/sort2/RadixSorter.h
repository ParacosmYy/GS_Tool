/**
 * @file RadixSorter.h
 * @brief 基数排序器 — LSD/MSD稳定基数排序
 *
 * 功能: 支持无符号整数/有符号整数/字节的LSD基数排序，
 *       MSD递归基数排序，计数排序子程序，统计排序次数/耗时。
 */
#ifndef RADIXSORTER_H
#define RADIXSORTER_H

#include <QObject>
#include <QVector>

class RadixSorter : public QObject {
    Q_OBJECT
public:
    /** 排序方向 */
    enum class SortOrder {
        Ascending,  ///< 升序
        Descending  ///< 降序
    };
    Q_ENUM(SortOrder)

    /** 统计 */
    struct Stats {
        quint64 totalSorts = 0;
        quint64 totalElementsSorted = 0;
        int     radixBits = 8;         ///< 每趟基数位数
        double  avgProcessingTimeMs = 0.0;
    };

    explicit RadixSorter(int radixBits = 8, QObject* parent = nullptr);

    /** @brief LSD基数排序(无符号) @param data 数据 @param order 方向 @return 排序结果 */
    QVector<quint32> sortLsd(const QVector<quint32>& data,
                              SortOrder order = SortOrder::Ascending);

    /** @brief LSD基数排序(有符号) @param data 数据 @return 排序结果 */
    QVector<qint32> sortLsdSigned(const QVector<qint32>& data);

    /** @brief MSD基数排序 @param data 数据 @param order 方向 @return 排序结果 */
    QVector<quint32> sortMsd(const QVector<quint32>& data,
                              SortOrder order = SortOrder::Ascending);

    /** @brief 字节计数排序 @param data 字节数组 @return 排序结果 */
    QVector<quint8> sortByCounting(const QVector<quint8>& data);

    /** @brief 原地LSD排序 @param data 数据 @param order 方向 */
    void sortLsdInPlace(QVector<quint32>& data,
                         SortOrder order = SortOrder::Ascending);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void sorted(int elementCount, double elapsedMs);

private:
    void msdRecursion(QVector<quint32>& data, int from, int to, int bitOffset);

    int m_radixBits;
    Stats m_stats;
    double m_timeSum;
};

#endif // RADIXSORTER_H

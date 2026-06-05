/**
 * @file SparseArray.h
 * @brief 稀疏数组 — 压缩存储大部分为零的数据
 *
 * 功能: 稀疏数组，使用坐标列表(COO)存储非零元素，
 *       支持随机访问、迭代、压缩/解压、统计密度/耗时。
 */
#ifndef SPARSEARRAY_H
#define SPARSEARRAY_H

#include <QObject>
#include <QVector>
#include <QMap>
#include <QPair>

class SparseArray : public QObject {
    Q_OBJECT
public:
    /** 稀疏元素 */
    struct Element {
        int    index;   ///< 原始索引
        double value;   ///< 非零值
    };

    /** 统计 */
    struct Stats {
        quint64 totalAccesses = 0;
        quint64 totalNonZeroElements = 0;
        int     logicalSize = 0;
        double  sparsity = 1.0;          ///< 稀疏度(零元素占比)
        double  avgProcessingTimeMs = 0.0;
    };

    explicit SparseArray(int logicalSize = 0, QObject* parent = nullptr);

    /** @brief 设置值 @param index 索引 @param value 值 */
    void setValue(int index, double value);

    /** @brief 获取值 @param index 索引 @return 值(越界或零元素返回0) */
    double getValue(int index) const;

    /** @brief 增加值 @param index 索引 @param delta 增量 */
    void addValue(int index, double delta);

    /** @brief 获取所有非零元素 @return 元素列表 */
    QVector<Element> nonZeroElements() const;

    /** @brief 转为密集数组 @return 密集数组 */
    QVector<double> toDense() const;

    /** @brief 从密集数组构建 @param data 密集数据 @param threshold 阈值 */
    void fromDense(const QVector<double>& data, double threshold = 0.0);

    /** @brief 逻辑大小 */
    int size() const { return m_logicalSize; }

    /** @brief 非零元素数 */
    int nonZeroCount() const { return m_data.size(); }

    /** @brief 压缩率 */
    double compressionRatio() const;

    /** @brief 裁剪低于阈值的元素 @param threshold 阈值 */
    void prune(double threshold);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void valueSet(int index, double value);
    void pruned(int removedCount);

private:
    void updateSparsity();

    int m_logicalSize;
    QMap<int, double> m_data;   ///< index → value (仅非零)
    mutable Stats m_stats;
    mutable double m_timeSum;
};

#endif // SPARSEARRAY_H

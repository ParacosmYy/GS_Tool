/**
 * @file BurrowsWheelerTransform.h
 * @brief Burrows-Wheeler变换 — 数据压缩预处理
 *
 * 功能: 正变换/逆变换BWT，统计变换/逆变换次数/耗时，
 *       变换完成信号。
 */
#ifndef BURROWSWHEELERTRANSFORM_H
#define BURROWSWHEELERTRANSFORM_H

#include <QObject>
#include <QByteArray>

class BurrowsWheelerTransform : public QObject {
    Q_OBJECT
public:
    /** BWT结果 */
    struct Result {
        QByteArray transformed;  ///< 变换后数据
        int originalIndex = 0;   ///< 原始行在排序后的索引
    };

    /** 操作统计 */
    struct Stats {
        quint64 totalTransforms = 0;
        quint64 totalInverses = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit BurrowsWheelerTransform(QObject* parent = nullptr);

    /** @brief 正变换BWT @param input 输入数据 @return 变换结果(含索引) */
    Result transform(const QByteArray& input);

    /** @brief 逆变换BWT @param transformed 变换后数据 @param index 原始索引 @return 原始数据 */
    QByteArray inverseTransform(const QByteArray& transformed, int index);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 变换完成 @param inputSize 输入大小 @param index 原始索引 */
    void transformCompleted(int inputSize, int index);

private:
    Stats m_stats;
    double m_timeSum;
};

#endif // BURROWSWHEELERTRANSFORM_H

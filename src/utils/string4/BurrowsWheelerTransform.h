/**
 * @file BurrowsWheelerTransform.h
 * * @brief Burrows-Wheeler变换 — 数据压缩预处理
 *
 * 功能: 实现BWT变换和逆变换，用于数据压缩预处理阶段。
 *       BWT将数据重排为更易于压缩的形式(相同字符聚集)。
 *       支持二进制数据(QByteArray)和文本数据(QString)两种接口。
 *
 * 协作: DataCompressor(数据压缩) / EntropyCalculator(熵分析)
 * @author Serial Tool Team
 * @date 2026-06-05
 */
#pragma once

#include <QObject>
#include <QByteArray>
#include <QString>
#include <QPair>
#include <QVector>
#include <QtGlobal>

/**
 * @class BurrowsWheelerTransform
 * @brief Burrows-Wheeler变换与逆变换
 *
 * BWT变换通过循环移位排序将输入数据重新排列，使得
 * 相似的上下文字符聚集在一起，便于后续压缩编码。
 * 逆变换使用"先列后行"(LF mapping)算法从变换结果
 * 和原始行索引恢复原始数据。
 */
class BurrowsWheelerTransform : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息结构体 */
    struct Stats {
        quint64 totalTransforms = 0;       ///< 累计正向变换次数
        quint64 totalInverse = 0;          ///< 累计逆变换次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit BurrowsWheelerTransform(QObject *parent = nullptr);

    /**
     * @brief 对二进制数据执行BWT变换
     * @param data 输入数据
     * @return QPair<变换后数据, 原始行索引>
     */
    QPair<QByteArray, int> transform(const QByteArray &data);

    /**
     * @brief 对二进制数据执行BWT逆变换
     * @param transformed 变换后数据
     * @param originalIndex 原始行索引
     * @return 恢复的原始数据
     */
    QByteArray inverseTransform(const QByteArray &transformed,
                                int originalIndex);

    /**
     * @brief 对字符串执行BWT变换
     * @param str 输入字符串
     * @return QPair<变换后字符串, 原始行索引>
     */
    QPair<QString, int> transformString(const QString &str);

    /**
     * @brief 对字符串执行BWT逆变换
     * @param transformed 变换后字符串
     * @param index 原始行索引
     * @return 恢复的原始字符串
     */
    QString inverseTransformString(const QString &transformed, int index);

    /** @brief 获取统计信息 @return 常量引用 */
    const Stats &stats() const { return m_stats; }

    /** @brief 重置统计计数器 */
    void resetStatistics();

signals:
    /** @brief 正向变换完成信号 @param inputSize 输入大小 @param index 原始索引 */
    void transformCompleted(int inputSize, int index);
    /** @brief 逆变换完成信号 @param outputSize 输出大小 */
    void inverseCompleted(int outputSize);

private:
    Stats m_stats;          ///< 统计信息
    double m_timeSum = 0.0; ///< 处理时间累加器
};

/**
 * @file GrayCode2.h
 * @brief 格雷码工具 — 二进制/格雷码转换与生成
 *
 * 功能: 提供二进制与格雷码互转、n位格雷码序列生成、
 *       循环格雷码、格雷码距离计算等工具函数。
 *
 * 协作: DataEncoder(编码) / StateTracker(状态机编码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

/**
 * @brief 格雷码工具 — 二进制与格雷码转换及生成
 */
class GrayCode2 : public QObject {
    Q_OBJECT

public:
    /** @brief 统计 */
    struct Stats {
        quint64 totalConversions = 0;       ///< 累计转换次数
        quint64 totalGenerations = 0;       ///< 累计生成次数
        quint64 totalCodesGenerated = 0;    ///< 累计生成码字数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理时间(ms)
        quint64 maxBitsUsed = 0;            ///< 历史最大位宽
    };

    explicit GrayCode2(QObject* parent = nullptr);

    /**
     * @brief 二进制转格雷码
     * @param binary 二进制值
     * @return 格雷码值
     */
    quint64 binaryToGray(quint64 binary);

    /**
     * @brief 格雷码转二进制
     * @param gray 格雷码值
     * @return 二进制值
     */
    quint64 grayToBinary(quint64 gray);

    /**
     * @brief 生成n位格雷码完整序列
     * @param n 位宽
     * @return 格雷码序列(长度2^n)
     */
    QVector<quint64> generateSequence(int n);

    /**
     * @brief 生成循环格雷码(首尾汉明距离为1)
     * @param n 位宽
     * @return 循环格雷码序列
     */
    QVector<quint64> generateCyclic(int n);

    /**
     * @brief 计算两个格雷码之间的汉明距离
     * @param grayA 格雷码A
     * @param grayB 格雷码B
     * @param bits 位宽
     * @return 汉明距离
     */
    int grayDistance(quint64 grayA, quint64 grayB, int bits);

    /**
     * @brief 查找格雷码序列中指定码的位置
     * @param sequence 格雷码序列
     * @param gray 目标格雷码
     * @return 索引，未找到返回-1
     */
    int findGrayIndex(const QVector<quint64>& sequence, quint64 gray);

    /**
     * @brief 验证序列是否为合法格雷码序列
     * @param sequence 待验证序列
     * @param bits 位宽
     * @return 相邻码距离是否全部为1
     */
    bool validateGraySequence(const QVector<quint64>& sequence, int bits);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 转换完成 @param input 输入值 @param output 输出值 */
    void conversionDone(quint64 input, quint64 output);

    /** @brief 序列生成完成 @param n 位宽 @param count 码字数 */
    void sequenceGenerated(int n, int count);

private:
    int popcount(quint64 v) const;

    Stats m_stats;
    double m_timeSum = 0.0;     ///< 处理时间累加器
};

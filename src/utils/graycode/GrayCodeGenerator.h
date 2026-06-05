/**
 * @file GrayCodeGenerator.h
 * @brief Gray码(格雷码)生成器
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QByteArray>
#include <QString>

/**
 * @class GrayCodeGenerator
 * @brief Gray码生成与转换工具
 *
 * 支持n位Gray码序列生成、二进制↔Gray码互转、Gray码距离计算。
 * 用于旋转编码器、Karnaugh图、错误校正等场景。
 */
class GrayCodeGenerator : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalGenerated = 0;      /**< 总生成序列数 */
        int totalConversions = 0;    /**< 总转换次数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit GrayCodeGenerator(QObject* parent = nullptr);

    /**
     * @brief 生成n位Gray码完整序列
     * @param bits 位数
     * @return Gray码序列(十进制值)
     */
    QVector<int> generate(int bits) const;

    /**
     * @brief 二进制 → Gray码
     * @param binary 二进制值
     * @return Gray码值
     */
    int binaryToGray(int binary) const;

    /**
     * @brief Gray码 → 二进制
     * @param gray Gray码值
     * @return 二进制值
     */
    int grayToBinary(int gray) const;

    /**
     * @brief 批量二进制 → Gray码转换
     * @param values 二进制值列表
     * @return Gray码值列表
     */
    QVector<int> batchToGray(const QVector<int>& values) const;

    /**
     * @brief 批量Gray码 → 二进制转换
     * @param values Gray码值列表
     * @return 二进制值列表
     */
    QVector<int> batchToBinary(const QVector<int>& values) const;

    /**
     * @brief 计算两个Gray码之间的汉明距离
     * @param g1 第一个Gray码
     * @param g2 第二个Gray码
     * @return 汉明距离
     */
    int hammingDistance(int g1, int g2) const;

    /**
     * @brief 获取指定位置的单步Gray码
     * @param bits 总位数
     * @param step 步骤索引(0~2^bits-1)
     * @return Gray码值
     */
    int grayCodeAt(int bits, int step) const;

    /**
     * @brief 将Gray码转为二进制字符串
     * @param gray Gray码值
     * @param bits 显示位数
     * @return 二进制字符串表示
     */
    QString toString(int gray, int bits) const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 序列生成完成信号 */
    void sequenceGenerated(int bits, int count);

private:
    mutable Stats m_stats;      /**< 统计信息 */
    mutable double m_timeSum;   /**< 累计时间 */
};

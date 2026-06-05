#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief GrayCode10 - 格雷码第10代实现
 *
 * 提供格雷码的生成与转换功能，支持二进制-格雷码互转、
 * n位格雷码序列生成及多位格雷码距离计算。
 */
class GrayCode10 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalConvertOps = 0; double avgProcessingTimeMs = 0.0; };
    explicit GrayCode10(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 生成指定位宽的格雷码序列
     * @param numBits 位宽
     * @return 格雷码整数序列
     */
    QVector<int> generateSequence(int numBits);

    /**
     * @brief 二进制转格雷码
     * @param binary 输入二进制值
     * @return 对应格雷码值
     */
    int binaryToGray(int binary) const;

    /**
     * @brief 格雷码转二进制
     * @param gray 输入格雷码值
     * @return 对应二进制值
     */
    int grayToBinary(int gray) const;

    /**
     * @brief 计算两个格雷码之间的汉明距离
     * @param grayA 格雷码A
     * @param grayB 格雷码B
     * @return 汉明距离
     */
    int hammingDistance(int grayA, int grayB) const;

signals:
    void conversionCompleted(int count);

private:
    Stats m_stats; double m_timeSum = 0.0;
};

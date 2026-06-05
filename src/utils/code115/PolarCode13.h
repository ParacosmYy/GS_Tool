#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief PolarCode13 - 极化码第13代实现
 *
 * 提供Polar码的编解码功能，支持Bhattacharyya参数信道极化、
 * SC/SCL/CA-SCL解码算法及信息位选择。
 */
class PolarCode13 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalDecodeOps = 0; double avgProcessingTimeMs = 0.0; };
    explicit PolarCode13(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief Polar码编码
     * @param infoBits 信息比特序列
     * @param codeLength 码长（2的幂）
     * @return 编码后的码字
     */
    QVector<int> encode(const QVector<int>& infoBits, int codeLength);

    /**
     * @brief SC（连续消除）解码
     * @param llrValues 对数似然比序列
     * @return 解码后的信息比特
     */
    QVector<int> decodeSC(const QVector<double>& llrValues);

    /**
     * @brief SCL（连续消除列表）解码
     * @param llrValues 对数似然比序列
     * @param listSize 列表大小
     * @return 解码后的信息比特
     */
    QVector<int> decodeSCL(const QVector<double>& llrValues, int listSize = 8);

    /**
     * @brief 基于Bhattacharyya参数选择信息位位置
     * @param codeLength 码长
     * @param infoLength 信息位长度
     * @param designSnrDb 设计信噪比 (dB)
     * @return 信息位索引集合
     */
    QVector<int> selectInfoBits(int codeLength, int infoLength, double designSnrDb);

signals:
    void decodeCompleted(int listSize);

private:
    Stats m_stats; double m_timeSum = 0.0;
};

#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief TurboCode10 - Turbo码第10代实现
 *
 * 提供并行级联卷积码的编码与迭代解码，
 * 支持MAP/Log-MAP/Max-Log-MAP解码算法及交织器设计。
 */
class TurboCode10 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalDecodeOps = 0; double avgProcessingTimeMs = 0.0; };
    explicit TurboCode10(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief Turbo码编码（两个RSC编码器并行级联）
     * @param infoBits 信息比特序列
     * @return 编码后的码字序列
     */
    QVector<int> encode(const QVector<int>& infoBits);

    /**
     * @brief 迭代Turbo解码
     * @param systematic 系统位LLR
     * @param parity1 第一编码器校验位LLR
     * @param parity2 第二编码器校验位LLR
     * @param maxIterations 最大迭代次数
     * @return 解码后的信息比特
     */
    QVector<int> decode(const QVector<double>& systematic,
                        const QVector<double>& parity1,
                        const QVector<double>& parity2,
                        int maxIterations = 8);

    /**
     * @brief 生成交织/解交织映射表
     * @param blockLength 块长度
     * @return 交织映射索引序列
     */
    QVector<int> generateInterleaver(int blockLength);

    /**
     * @brief 设置MAP解码算法变体
     * @param variant 算法变体 (MAP/LogMAP/MaxLogMAP)
     */
    void setMAPVariant(const QString& variant);

signals:
    void decodeCompleted(int iterationCount);

private:
    Stats m_stats; double m_timeSum = 0.0;
};

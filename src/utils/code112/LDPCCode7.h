#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief LDPCCode7 - 低密度奇偶校验码第7代实现
 *
 * 提供LDPC编解码功能，支持多种校验矩阵构造方法、
 * 置信传播迭代解码及误码率性能评估。
 */
class LDPCCode7 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalDecodeOps = 0; double avgProcessingTimeMs = 0.0; };
    explicit LDPCCode7(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 使用LDPC码对数据进行编码
     * @param dataBits 原始信息比特序列
     * @return 编码后的码字序列
     */
    QVector<int> encode(const QVector<int>& dataBits);

    /**
     * @brief 置信传播迭代解码
     * @param receivedBits 接收到的含噪码字
     * @param maxIterations 最大迭代次数
     * @return 解码后的信息比特序列
     */
    QVector<int> decodeBP(const QVector<double>& receivedBits, int maxIterations = 50);

    /**
     * @brief 构造规则LDPC校验矩阵
     * @param blockLength 码长
     * @param columnWeight 列重
     * @param rowWeight 行重
     * @return 稀疏校验矩阵
     */
    QVector<QVector<int>> buildParityMatrix(int blockLength, int columnWeight, int rowWeight);

    /**
     * @brief 计算当前LDPC码的误码率性能
     * @param snrDb 信噪比 (dB)
     * @param numTrials 蒙特卡洛仿真次数
     * @return 误码率值
     */
    double computeBER(double snrDb, int numTrials = 1000);

signals:
    void decodeCompleted(int iterations);

private:
    Stats m_stats; double m_timeSum = 0.0;
};

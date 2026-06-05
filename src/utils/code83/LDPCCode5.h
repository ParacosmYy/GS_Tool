#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief LDPC低密度奇偶校验码
 *
 * 稀疏校验矩阵的线性分组码，支持置信传播迭代解码。
 */
class LDPCCode5 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalBlocksEncoded = 0;
        int totalBlocksDecoded = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit LDPCCode5(QObject* parent = nullptr);

    /** @brief 编码数据块 */
    QVector<int> encode(const QVector<int>& data);

    /** @brief 置信传播解码 */
    QVector<int> decode(const QVector<double>& llr, int maxIterations = 50);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decodeCompleted(int iterations, bool converged);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_blockLength = 0;
};

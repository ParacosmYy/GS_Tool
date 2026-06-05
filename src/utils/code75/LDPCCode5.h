#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief LDPCCode5 - 低密度奇偶校验码编解码器
 *
 * 支持规则和非规则LDPC码，使用置信传播(BP)
 * 算法进行迭代解码，接近Shannon极限性能。
 */
class LDPCCode5 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalBlocksDecoded = 0;
        int totalIterations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit LDPCCode5(QObject* parent = nullptr);

    /** @brief 从奇偶校验矩阵H初始化LDPC码 */
    bool initialize(const QVector<QVector<int>>& parityMatrix);

    /** @brief 编码信息位 */
    QVector<double> encode(const QVector<double>& message);

    /** @brief 使用置信传播解码软判决输入 */
    QVector<int> decode(const QVector<double>& llr, int maxIterations = 50);

    /** @brief 获取码率和码长 */
    QPair<double,int> codeParameters() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decodingCompleted(int iterations, bool converged);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_blockLength = 0;
    int m_messageLength = 0;
    QVector<QVector<int>> m_H;
};

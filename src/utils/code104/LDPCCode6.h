#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief LDPC低密度奇偶校验码实现 (码型6)
 *
 * 提供LDPC编码与解码功能，支持稀疏校验矩阵和置信传播迭代译码。
 */
class LDPCCode6 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构
    struct Stats {
        int totalEncodingRuns = 0;      ///< 总编码次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
        int totalBitErrors = 0;         ///< 总比特错误数
    };

    explicit LDPCCode6(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief LDPC编码
     * @param informationBits 信息比特序列
     * @return 编码后的码字
     */
    QVector<int> encode(const QVector<int>& informationBits);

    /**
     * @brief LDPC置信传播译码
     * @param receivedBits 接收的软判决或硬判决序列
     * @param maxIter 最大迭代次数
     * @return 译码后的信息比特
     */
    QVector<int> decode(const QVector<double>& receivedBits, int maxIter = 50);

    /**
     * @brief 设置校验矩阵
     * @param parityCheckMatrix H矩阵的稀疏表示
     */
    void setParityMatrix(const QVector<QVector<int>>& parityCheckMatrix);

    /**
     * @brief 计算当前码率
     * @return 码率比值
     */
    double codeRate() const;

signals:
    /// 编码完成信号
    void encodingCompleted(int blockLength);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    QVector<QVector<int>> m_parityMatrix;
    int m_blockLength = 0;
};

#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief TurboCode5 - Turbo码编解码器
 *
 * 并行级联卷积码(PCCC)实现，使用两个RSC编码器
 * 和MAP/Log-MAP迭代解码，接近Shannon极限。
 */
class TurboCode5 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalBlocksDecoded = 0;
        int totalIterations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit TurboCode5(QObject* parent = nullptr);

    /** @brief 设置RSC分量编码器参数 */
    bool setComponentCodes(int constraintLength, const QVector<int>& generators);

    /** @brief 设置交织器类型和大小 */
    void setInterleaver(const QString& type, int size);

    /** @brief 编码信息块 */
    QVector<double> encode(const QVector<double>& message);

    /** @brief 迭代Turbo解码 */
    QVector<int> decode(const QVector<double>& received, int maxIterations = 8);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decodingCompleted(int iterations, bool converged);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_blockSize = 0;
    QString m_interleaverType = "random";
    QVector<int> m_interleaver;
};

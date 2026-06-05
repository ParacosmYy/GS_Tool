#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief ConvolutionalCode5 - 卷积码编解码器
 *
 * 支持可配置约束长度和生成多项式的卷积码，
 * 使用Viterbi算法进行最大似然解码。
 */
class ConvolutionalCode5 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalFramesEncoded = 0;
        int totalBitsDecoded = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ConvolutionalCode5(QObject* parent = nullptr);

    /** @brief 设置约束长度和生成多项式(八进制) */
    bool setPolynomials(int constraintLength, const QVector<int>& generators);

    /** @brief 编码输入比特流 */
    QVector<int> encode(const QVector<int>& bits);

    /** @brief 使用Viterbi算法解码软判决输入 */
    QVector<int> decode(const QVector<double>& softInput);

    /** @brief 获取编码率 */
    double codeRate() const;

    /** @brief 设置回溯深度 */
    void setTracebackDepth(int depth);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decodingCompleted(int frameSize, double ber);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_constraintLength = 7;
    int m_tracebackDepth = 0;
    QVector<int> m_generators;
};

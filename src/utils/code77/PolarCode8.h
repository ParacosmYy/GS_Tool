#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief PolarCode8 - 极化码编解码器
 *
 * 基于信道极化理论的线性分组码，使用SC/SCL解码，
 * 是目前唯一被证明可达Shannon极限的码族。
 */
class PolarCode8 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalBlocksDecoded = 0;
        int totalListDecodings = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit PolarCode8(QObject* parent = nullptr);

    /** @brief 设置码长N(2的幂)和信息位K */
    bool initialize(int N, int K);

    /** @brief 设置解码算法: sc/scl/cai */
    void setDecoder(const QString& decoder, int listSize = 1);

    /** @brief 编码信息位 */
    QVector<double> encode(const QVector<double>& message);

    /** @brief 解码接收信号(LLR输入) */
    QVector<int> decode(const QVector<double>& llr);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decodingCompleted(int listSize, bool crcPassed);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_N = 0;
    int m_K = 0;
    QString m_decoder = "sc";
    int m_listSize = 1;
};

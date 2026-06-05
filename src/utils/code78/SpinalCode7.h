#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief SpinalCode7 - 脊柱码编解码器
 *
 * 基于随机线性映射的无速率码，使用哈希函数
 * 生成无限编码符号，适合不固定码率传输场景。
 */
class SpinalCode7 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalSymbolsGenerated = 0;
        int totalDecodingAttempts = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SpinalCode7(QObject* parent = nullptr);

    /** @brief 设置脊柱深度k和哈希函数位数B */
    bool initialize(int k, int B);

    /** @brief 编码消息，生成指定数量的编码符号 */
    QVector<double> encode(const QVector<int>& message, int numSymbols);

    /** @brief 使用气泡解码器解码 */
    QVector<int> decode(const QVector<double>& received, int messageLength, int beamWidth = 16);

    /** @brief 设置哈希种子 */
    void setSeed(quint64 seed);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decodingCompleted(int symbols, bool success);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_k = 4;
    int m_B = 8;
    quint64 m_seed = 0;
};

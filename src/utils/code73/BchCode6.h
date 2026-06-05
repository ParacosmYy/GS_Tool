#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief BchCode6 - BCH纠错编解码器
 *
 * 支持可配置码长的BCH循环码编码与解码，
 * 使用Berlekamp-Massey算法进行纠错。
 */
class BchCode6 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalBlocksEncoded = 0;
        int totalErrorsCorrected = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BchCode6(QObject* parent = nullptr);

    /** @brief 初始化BCH码参数: 码长n, 信息位k, 纠错能力t */
    bool initialize(int n, int k, int t);

    /** @brief 编码信息位，返回码字 */
    QVector<int> encode(const QVector<int>& message);

    /** @brief 解码接收码字，返回纠错后信息位 */
    QVector<int> decode(const QVector<int>& codeword);

    /** @brief 计算伴随式 */
    QVector<int> computeSyndrome(const QVector<int>& received);

    /** @brief 获取生成多项式系数 */
    QVector<int> generatorPolynomial() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decodingCompleted(int errorsCorrected);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_n = 0;
    int m_k = 0;
    int m_t = 0;
    QVector<int> m_genPoly;
};

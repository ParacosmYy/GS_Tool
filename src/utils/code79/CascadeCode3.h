#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief CascadeCode3 - 级联码编解码器
 *
 * 外码(Reed-Solomon)和内码(卷积码)的级联方案，
 * 结合突发纠错和随机纠错能力，广泛用于深空通信。
 */
class CascadeCode3 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalBlocksProcessed = 0;
        int totalInnerErrors = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit CascadeCode3(QObject* parent = nullptr);

    /** @brief 配置外码(RS)和内码(卷积码)参数 */
    bool configure(int rsN, int rsK, int convConstraint, const QVector<int>& convGen);

    /** @brief 级联编码 */
    QVector<double> encode(const QVector<int>& message);

    /** @brief 级联解码: 先Viterbi内码解码，再RS外码解码 */
    QVector<int> decode(const QVector<double>& received);

    /** @brief 获取内码解码后的误码统计 */
    QPair<int,int> errorStatistics() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decodingCompleted(int innerErrors, int outerErrors);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_rsN = 0;
    int m_rsK = 0;
    int m_innerErrors = 0;
    int m_outerErrors = 0;
};

#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief Reed-Solomon纠错编码器
 *
 * 实现Reed-Solomon编解码，支持可配置的多项式阶数，
 * 广泛用于通信和存储系统的前向纠错。
 */
class ReedSolomon10 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats {
        int totalEncoded = 0;        ///< 已编码数据块数
        int totalDecoded = 0;        ///< 已解码数据块数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit ReedSolomon10(QObject* parent = nullptr);

    /** @brief 设置生成多项式阶数 */
    void setPolyOrder(int order);
    /** @brief 编码数据向量 */
    QVector<int> encode(const QVector<int>& data);
    /** @brief 解码数据向量，纠正传输错误 */
    QVector<int> decode(const QVector<int>& data);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 编解码完成，返回纠错符号数 */
    void codingCompleted(int correctedSymbols);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_polyOrder = 8;
};

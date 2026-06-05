#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 卷积码编解码工具类
 *
 * 提供卷积码的编码与Viterbi解码功能，
 * 支持设置约束长度，用于通信前向纠错。
 */
class ConvCode5 : public QObject {
    Q_OBJECT
public:
    /// 编解码统计信息
    struct Stats {
        int totalOperations = 0;    ///< 总编解码操作次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit ConvCode5(QObject* parent = nullptr);

    /** @brief 设置卷积码约束长度 */
    void setConstraintLen(int len);

    /** @brief 对输入比特序列进行卷积编码 */
    QVector<int> encode(const QVector<int>& bits);

    /** @brief 对接收序列进行Viterbi解码 */
    QVector<int> decode(const QVector<int>& received);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 编解码完成信号，返回数据长度 */
    void codingCompleted(int dataLength);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_constraintLen = 7;
};

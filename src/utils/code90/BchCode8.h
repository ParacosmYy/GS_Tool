#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief BCH纠错码工具类
 *
 * 提供BCH码的编码与解码功能，支持设置多项式阶数，
 * 可纠正多位随机错误。
 */
class BchCode8 : public QObject {
    Q_OBJECT
public:
    /// 编解码统计信息
    struct Stats {
        int totalOperations = 0;    ///< 总编解码操作次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit BchCode8(QObject* parent = nullptr);

    /** @brief 设置生成多项式阶数 */
    void setPolyOrder(int order);

    /** @brief 对输入数据进行BCH编码 */
    QVector<int> encode(const QVector<int>& data);

    /** @brief 对含错误的码字进行BCH解码 */
    QVector<int> decode(const QVector<int>& codeword);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 编解码完成信号，返回数据长度 */
    void codingCompleted(int dataLength);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_polyOrder = 5;
};

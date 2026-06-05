#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief BCH纠错码编解码器
 *
 * 实现BCH码的编码和Berlekamp-Massey译码，
 * 支持可配置的纠错能力和多项式阶数。
 */
class BchCode9 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats {
        int totalEncoded = 0;        ///< 已编码码字数
        int totalDecoded = 0;        ///< 已解码码字数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit BchCode9(QObject* parent = nullptr);

    /** @brief 设置伽罗华域多项式阶数 */
    void setPolyOrder(int order);
    /** @brief 设置纠错能力(可纠正符号数) */
    void setCorrectionCap(int t);
    /** @brief BCH编码 */
    QVector<int> encode(const QVector<int>& data);
    /** @brief BCH译码，纠正错误 */
    QVector<int> decode(const QVector<int>& received);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 编解码完成，返回纠正错误数 */
    void codingCompleted(int correctedErrors);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_polyOrder = 5;
    int m_correctionCap = 3;
};

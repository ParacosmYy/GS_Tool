#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 极化码编解码工具类
 *
 * 提供Polar Code的编码与SC/SCL解码功能，
 * 支持设置码长和信息位数量。
 */
class PolarCode10 : public QObject {
    Q_OBJECT
public:
    /// 编解码统计信息
    struct Stats {
        int totalOperations = 0;    ///< 总编解码操作次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit PolarCode10(QObject* parent = nullptr);

    /** @brief 设置极化码长度(必须为2的幂) */
    void setCodeLength(int length);

    /** @brief 设置信息位数量 */
    void setInfoBits(int count);

    /** @brief 对输入信息位进行极化码编码 */
    QVector<int> encode(const QVector<int>& infoBits);

    /** @brief 对接收的LLR序列进行极化码解码 */
    QVector<int> decode(const QVector<double>& llr);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 编解码完成信号，返回码字长度 */
    void codingCompleted(int codeLength);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_codeLength = 256;
    int m_infoBits = 128;
};

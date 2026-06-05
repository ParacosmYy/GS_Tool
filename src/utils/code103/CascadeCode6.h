#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 级联码编解码器
 *
 * 将内码和外码串联组合实现更强的纠错能力，
 * 典型组合如RS+卷积码，广泛用于卫星通信。
 */
class CascadeCode6 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalCoded = 0; double avgProcessingTimeMs = 0.0; };

    explicit CascadeCode6(QObject* parent = nullptr);

    /** @brief 设置内码类型 */
    void setInnerCode(const QString& codeType);

    /** @brief 设置外码类型 */
    void setOuterCode(const QString& codeType);

    /** @brief 编码比特序列 */
    QVector<int> encode(const QVector<int>& data);

    /** @brief 解码软比特序列 */
    QVector<int> decode(const QVector<double>& softBits);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 编解码完成信号 */
    void codingCompleted(int bitCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    QString m_innerCode = QStringLiteral("Convolutional");
    QString m_outerCode = QStringLiteral("ReedSolomon");
};

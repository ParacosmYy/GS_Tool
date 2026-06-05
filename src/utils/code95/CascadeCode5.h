#pragma once
#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief 级联码编码解码器
 *
 * 将内码(如卷积码)与外码(如Reed-Solomon码)串联组合,
 * 利用两级纠错大幅提升突发错误与随机错误的综合纠错能力。
 */
class CascadeCode5 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalEncoded = 0; double avgProcessingTimeMs = 0.0; };

    explicit CascadeCode5(QObject* parent = nullptr);

    /** @brief 设置内码类型 */
    void setInnerCode(const QString& codeType);

    /** @brief 设置外码类型 */
    void setOuterCode(const QString& codeType);

    /** @brief 对比特序列执行级联编码 */
    void encode(const QVector<int>& bits);

    /** @brief 对软信息执行级联解码 */
    void decode(const QVector<double>& symbols);

    /** @brief 获取当前统计信息 */
    Stats stats() const { return m_stats; }

    /** @brief 重置统计数据 */
    void resetStatistics();

signals:
    /** @brief 编解码完成信号 */
    void codingCompleted(int blockCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    QString m_innerCode;
    QString m_outerCode;
};

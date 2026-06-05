#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 级联码(Cascade Code)编解码器实现
 *
 * 将多个纠错码(如RS码+卷积码)串联组合，外码纠正突发错误、内码纠正随机错误，
 * 通过迭代解调和交织实现极低误码率，适用于卫星通信和深空通信链路。
 */
class CascadeCode7 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalDecoded = 0; double avgProcessingTimeMs = 0.0; };

    explicit CascadeCode7(QObject* parent = nullptr);

    /** @brief 设置外码类型和参数(如RS码的n,k值) */
    void setOuterCode(const QString& type, int n, int k);

    /** @brief 设置内码类型和约束长度 */
    void setInnerCode(const QString& type, int constraintLength);

    /** @brief 对输入数据执行级联编码(外码+交织+内码) */
    QVector<int> encode(const QVector<int>& data);

    /** @brief 对接收数据执行级联解码(内码+解交织+外码) */
    QVector<int> decode(const QVector<double>& softData, int dataLength);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 解码完成信号，返回是否纠正成功和纠错数 */
    void decodingCompleted(bool success, int correctedErrors);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    QString m_outerType;
    int m_outerN = 255;
    int m_outerK = 223;
    QString m_innerType;
    int m_constraintLength = 7;
};

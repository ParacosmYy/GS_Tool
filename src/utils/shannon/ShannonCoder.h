/**
 * @file ShannonCoder.h
 * @brief Shannon-Fano编码器 — 信息论最优前缀码
 *
 * 功能: 基于符号频率的Shannon-Fano编码/解码，
 *       计算信源熵/编码效率/冗余度，
 *       统计编解码次数/符号数/耗时。
 */
#ifndef SHANNONCODER_H
#define SHANNONCODER_H

#include <QObject>
#include <QByteArray>
#include <QVector>
#include <QMap>

class ShannonCoder : public QObject {
    Q_OBJECT
public:
    /** 统计 */
    struct Stats {
        quint64 totalEncodes = 0;
        quint64 totalDecodes = 0;
        double  avgEfficiency = 0.0;     ///< 平均编码效率
        double  avgProcessingTimeMs = 0.0;
    };

    /** 符号统计 */
    struct SymbolInfo {
        int symbol = 0;           ///< 符号值
        double probability = 0.0; ///< 概率
        QByteArray code;          ///< 编码位串
    };

    explicit ShannonCoder(QObject* parent = nullptr);

    /** @brief 编码 @param data 输入数据 @return 编码结果(头部+压缩数据) */
    QByteArray encode(const QByteArray& data);

    /** @brief 解码 @param data 编码数据 @return 原始数据 */
    QByteArray decode(const QByteArray& data);

    /** @brief 计算Shannon熵 @param data 数据 @return 熵(bit/symbol) */
    double entropy(const QByteArray& data) const;

    /** @brief 获取符号表 @return 符号信息列表 */
    QVector<SymbolInfo> symbolTable() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encoded(const QByteArray& result, double efficiency);
    void decoded(const QByteArray& result);
    void error(const QString& message);

private:
    void buildCodeTable(const QVector<quint32>& freq, int total);
    QVector<quint32> buildFreqTable(const QByteArray& data) const;

    QMap<int, QByteArray> m_codeTable;
    QVector<SymbolInfo> m_symbolTable;
    Stats m_stats;
    double m_timeSum;
};

#endif // SHANNONCODER_H

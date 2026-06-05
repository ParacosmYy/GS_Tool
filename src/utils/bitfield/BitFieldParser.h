/**
 * @file BitFieldParser.h
 * @brief 位域解析器 — 二进制协议字段提取
 *
 * 功能: 支持从字节流中按位提取字段，支持大小端/符号扩展，
 *       统计解析次数/提取字段数/平均耗时。
 */
#ifndef BITFIELDPARSER_H
#define BITFIELDPARSER_H

#include <QObject>
#include <QByteArray>
#include <QList>
#include <QMap>
#include <utility>

/**
 * @class BitFieldParser
 * @brief 从二进制数据中按位提取结构化字段
 */
class BitFieldParser : public QObject {
    Q_OBJECT
public:
    /** 字段定义 */
    struct FieldDef {
        QString name;       ///< 字段名
        int startBit;       ///< 起始位(从0开始,从最高位计)
        int bitWidth;       ///< 位宽
        bool isSigned;      ///< 是否有符号
    };

    /** 解析结果 */
    struct FieldValue {
        QString name;       ///< 字段名
        quint64 rawValue;   ///< 原始无符号值
        double scaledValue; ///< 缩放后值
        QString hexString;  ///< 十六进制表示
    };

    /** 解析统计 */
    struct Stats {
        quint64 totalParses = 0;
        quint64 totalFieldsExtracted = 0;
        double  averageProcessingTimeMs = 0.0;
    };

    explicit BitFieldParser(QObject* parent = nullptr);

    /** 添加字段定义 */
    void addField(const FieldDef& def);
    void clearFields();

    /** 设置字节序 */
    void setBigEndian(bool bigEndian);

    /** 设置缩放因子 */
    void setScale(const QString& fieldName, double scale, double offset);

    /** 解析数据 */
    QList<FieldValue> parse(const QByteArray& data);

    /** 提取单个字段 */
    quint64 extractBits(const QByteArray& data, int startBit, int bitWidth) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void parseComplete(int fieldCount);
    void fieldExtracted(const FieldValue& value);

private:
    QList<FieldDef> m_fields;
    QMap<QString, std::pair<double,double>> m_scales; ///< fieldName → (scale, offset)
    bool m_bigEndian;
    Stats m_stats;
    double m_timeSum;
};

#endif // BITFIELDPARSER_H

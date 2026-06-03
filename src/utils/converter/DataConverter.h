/**
 * @file DataConverter.h
 * @brief 数据格式转换引擎
 * @author Serial Tool Team
 * @date 2026-06-02
 *
 * 支持 Hex/ASCII/Base64/URL编码/Binary/Decimal/Octal 之间的互转。
 */

#ifndef DATACONVERTER_H
#define DATACONVERTER_H

#include <QByteArray>
#include <QList>
#include <QMap>
#include <QObject>
#include <QString>

/**
 * @class DataConverter
 * @brief 数据格式转换引擎，纯计算无状态
 */
class DataConverter : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 数据格式枚举
     */
    enum Format {
        Hex,        ///< 十六进制
        Ascii,      ///< ASCII 文本
        Base64,     ///< Base64 编码
        UrlEncode,  ///< URL 编码
        Binary,     ///< 二进制
        Decimal,    ///< 十进制
        Octal       ///< 八进制
    };
    Q_ENUM(Format)

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit DataConverter(QObject *parent = nullptr);

    /**
     * @brief 格式转换
     * @param input 输入数据
     * @param from 源格式
     * @param to 目标格式
     * @return 转换后的数据
     */
    QByteArray convert(const QByteArray &input, Format from, Format to) const;

    /**
     * @brief 自动检测数据格式
     * @param data 待检测数据
     * @return 检测到的最可能格式
     */
    Format detectFormat(const QByteArray &data) const;

    /**
     * @brief 获取格式名称
     * @param format 格式枚举值
     * @return 格式名称字符串
     */
    static QString formatName(Format format);

    /**
     * @brief 获取格式的人类可读描述
     * @param format 格式枚举值
     * @return 描述字符串，适合作为UI工具提示
     */
    static QString formatDescription(Format format);

    /**
     * @brief 获取所有支持的格式列表
     * @return 格式枚举列表
     */
    static QList<Format> supportedFormats();

    /**
     * @brief 将输入数据转换到所有其他格式（对比视图）
     * @param input 输入数据
     * @param from 源格式
     * @return QMap<格式名, 转换结果>
     */
    QMap<QString, QByteArray> convertToAll(const QByteArray& input, Format from) const;

    /** @brief 获取累计转换次数 */
    qint64 conversionCount() const;

    /** @brief 重置转换计数 */
    void resetCount();

private:
    /**
     * @brief 将数据从指定格式解码为原始字节
     */
    QByteArray decodeToRaw(const QByteArray &input, Format from) const;

    /**
     * @brief 将原始字节编码为指定格式
     */
    QByteArray encodeFromRaw(const QByteArray &raw, Format to) const;

    /** @brief 累计转换次数 */
    mutable qint64 m_convCount = 0;
};

#endif // DATACONVERTER_H

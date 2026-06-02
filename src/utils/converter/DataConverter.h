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

private:
    /**
     * @brief 将数据从指定格式解码为原始字节
     */
    QByteArray decodeToRaw(const QByteArray &input, Format from) const;

    /**
     * @brief 将原始字节编码为指定格式
     */
    QByteArray encodeFromRaw(const QByteArray &raw, Format to) const;
};

#endif // DATACONVERTER_H

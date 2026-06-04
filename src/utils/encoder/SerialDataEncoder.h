/**
 * @file SerialDataEncoder.h
 * @brief 串口数据编解码引擎
 * @author Serial Tool Team
 * @date 2026-06-05
 *
 * 支持 Hex/Base64/Base32/Base85/ASCII/Binary/URL/Quoted-Printable
 * 八种编码格式的双向转换、自动检测和统计。
 */

#ifndef SERIALDATAENCODER_H
#define SERIALDATAENCODER_H

#include <QByteArray>
#include <QObject>
#include <QString>
#include <QtGlobal>

/**
 * @class SerialDataEncoder
 * @brief 串口数据编解码引擎
 *
 * 提供八种常用串口通信编码格式的编码/解码能力，
 * 支持自动格式检测、有效性校验和操作统计。
 */
class SerialDataEncoder : public QObject
{
    Q_OBJECT

public:
    /** @brief 编码格式枚举 */
    enum class Encoding {
        Hex,              ///< 十六进制
        Base64,           ///< Base64 (RFC 4648)
        Ascii,            ///< ASCII 明文
        Binary,           ///< 二进制字符串 (0/1)
        UrlEncode,        ///< URL 百分号编码 (RFC 3986)
        QuotedPrintable,  ///< Quoted-Printable (RFC 2045)
        Base32,           ///< Base32 (RFC 4648)
        Base85            ///< Base85 (Ascii85 变体)
    };
    Q_ENUM(Encoding)

    /** @brief 操作统计结构体 */
    struct Stats {
        quint64 totalEncodes = 0;              ///< 编码操作总次数
        quint64 totalDecodes = 0;              ///< 解码操作总次数
        quint64 totalBytesEncoded = 0;         ///< 编码处理的原始字节数
        quint64 totalBytesDecoded = 0;         ///< 解码产出的原始字节数
        quint64 encodeErrors = 0;              ///< 编码失败次数
        quint64 decodeErrors = 0;              ///< 解码失败次数
        quint64 operationsByEncoding[8] = {};  ///< 按编码格式统计操作次数，索引对应Encoding枚举
    };

    /** @brief 构造串口数据编解码引擎 @param parent 父对象 */
    explicit SerialDataEncoder(QObject *parent = nullptr);

    // ---- 核心编解码 API ----

    /**
     * @brief 编码原始字节数据为指定格式
     * @param data 原始字节数据
     * @param encoding 目标编码格式
     * @return 编码后的字节数组，失败返回空
     */
    QByteArray encode(const QByteArray &data, Encoding encoding) const;

    /**
     * @brief 从指定格式解码为原始字节
     * @param data 编码后的数据
     * @param encoding 源编码格式
     * @return 解码后的原始字节，失败返回空
     */
    QByteArray decode(const QByteArray &data, Encoding encoding) const;

    /**
     * @brief 编码为字符串形式
     * @param data 原始字节数据
     * @param encoding 目标编码格式
     * @return 编码后的字符串，失败返回空
     */
    QString encodeToString(const QByteArray &data, Encoding encoding) const;

    /**
     * @brief 自动检测数据最可能的编码格式
     * @param data 待检测数据
     * @return 最可能的编码格式枚举值
     */
    Encoding detectEncoding(const QByteArray &data) const;

    /**
     * @brief 检测数据是否符合指定编码格式
     * @param data 待检测数据
     * @param encoding 目标编码格式
     * @return true 表示数据符合该格式
     */
    bool isEncoded(const QByteArray &data, Encoding encoding) const;

    // ---- 元数据接口 ----

    /** @brief 获取编码格式的标准名称 @param encoding 编码格式枚举 @return 格式名称(如"Hex"/"Base64") */
    static QString encodingName(Encoding encoding);

    /** @brief 获取编码格式的人类可读中文描述 @param encoding 编码格式枚举 @return 描述字符串 */
    static QString encodingDescription(Encoding encoding);

    // ---- 统计接口 ----

    /** @brief 获取操作统计快照 @return 当前统计数据的只读引用 */
    const Stats &stats() const;

    /** @brief 重置所有统计计数器(编码/解码次数/字节数/错误数/按格式计数全部归零) */
    void resetStatistics();

signals:
    /** @brief 编码完成信号 @param result 编码结果 @param encoding 使用的编码格式 */
    void encodeComplete(const QByteArray &result, Encoding encoding) const;

    /** @brief 解码完成信号 @param result 解码结果 @param encoding 使用的编码格式 */
    void decodeComplete(const QByteArray &result, Encoding encoding) const;

    /** @brief 操作错误信号 @param errorMessage 错误描述(中文) @param encoding 相关的编码格式 */
    void error(const QString &errorMessage, Encoding encoding) const;

private:
    // ---- 各编码格式的内部实现 ----

    /** @brief 十六进制编码 @param data 原始字节 @return Hex字符串 */
    QByteArray encodeHex(const QByteArray &data) const;

    /** @brief 十六进制解码 @param data Hex字符串 @return 原始字节 */
    QByteArray decodeHex(const QByteArray &data) const;

    /** @brief Base32编码 @param data 原始字节 @return Base32字符串 */
    QByteArray encodeBase32(const QByteArray &data) const;

    /** @brief Base32解码 @param data Base32字符串 @return 原始字节 */
    QByteArray decodeBase32(const QByteArray &data) const;

    /** @brief Base85(Ascii85)编码 @param data 原始字节 @return Ascii85字符串 */
    QByteArray encodeBase85(const QByteArray &data) const;

    /** @brief Base85(Ascii85)解码 @param data Ascii85字符串 @return 原始字节 */
    QByteArray decodeBase85(const QByteArray &data) const;

    /** @brief URL百分号编码 @param data 原始字节 @return URL编码字符串 */
    QByteArray encodeUrl(const QByteArray &data) const;

    /** @brief URL百分号解码 @param data URL编码字符串 @return 原始字节 */
    QByteArray decodeUrl(const QByteArray &data) const;

    /** @brief Quoted-Printable编码 @param data 原始字节 @return QP字符串 */
    QByteArray encodeQuotedPrintable(const QByteArray &data) const;

    /** @brief Quoted-Printable解码 @param data QP字符串 @return 原始字节 */
    QByteArray decodeQuotedPrintable(const QByteArray &data) const;

    /** @brief 二进制字符串编码 @param data 原始字节 @return 0/1字符串(空格分隔) */
    QByteArray encodeBinary(const QByteArray &data) const;

    /** @brief 二进制字符串解码 @param data 0/1字符串 @return 原始字节 */
    QByteArray decodeBinary(const QByteArray &data) const;

    // ---- 检测辅助 ----

    /** @brief 检测数据是否为合法Hex格式 */
    bool isHexData(const QByteArray &data) const;

    /** @brief 检测数据是否为合法Base64格式 */
    bool isBase64Data(const QByteArray &data) const;

    /** @brief 检测数据是否为合法Base32格式 */
    bool isBase32Data(const QByteArray &data) const;

    /** @brief 检测数据是否为合法Base85格式 */
    bool isBase85Data(const QByteArray &data) const;

    /** @brief 检测数据是否为合法URL编码格式 */
    bool isUrlEncodedData(const QByteArray &data) const;

    /** @brief 检测数据是否为合法Quoted-Printable格式 */
    bool isQuotedPrintableData(const QByteArray &data) const;

    /** @brief 检测数据是否为合法二进制字符串 */
    bool isBinaryData(const QByteArray &data) const;

    mutable Stats m_stats;  ///< 操作统计(可变，允许const方法内更新)
};

#endif // SERIALDATAENCODER_H

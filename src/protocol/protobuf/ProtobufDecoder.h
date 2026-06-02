/**
 * @file ProtobufDecoder.h
 * @brief Protobuf解码器 — 加载.proto模式并解码/编码消息
 *
 * 解析.proto文件定义的消息结构，将二进制数据解码为结构化映射，
 * 或将映射数据编码为二进制Protobuf格式。
 */
#ifndef PROTOBUF_DECODER_H
#define PROTOBUF_DECODER_H

#include <QBuffer>
#include <QObject>
#include <QString>
#include <QByteArray>
#include <QVariantMap>

/**
 * @brief Protobuf解码/编码器
 * 加载.proto模式定义，提供消息的解码和编码功能。
 */
class ProtobufDecoder : public QObject {
    Q_OBJECT

public:
    explicit ProtobufDecoder(QObject* parent = nullptr);

    /**
     * @brief 加载.proto模式文件
     * @param filePath .proto文件路径
     * @return 是否加载成功
     */
    bool loadProtoFile(const QString& filePath);

    /** @brief 是否已加载模式 */
    bool isLoaded() const;

    /**
     * @brief 解码Protobuf二进制消息
     * @param data 原始二进制数据
     * @return 解码后的字段映射 {fieldNumber: {name, type, value}}
     */
    QVariantMap decodeMessage(const QByteArray& data);

    /**
     * @brief 编码为Protobuf二进制消息
     * @param fields 字段映射
     * @return 编码后的二进制数据
     */
    QByteArray encodeMessage(const QVariantMap& fields);

signals:
    /** @brief 消息解码完成 */
    void decoded(const QVariantMap& result);

    /** @brief 解码错误 */
    void decodeError(const QString& errorMsg);

private:
    /**
     * @brief 解析varint编码
     * @return {值, 消耗的字节数}
     */
    QPair<quint64, int> parseVarint(const QByteArray& data, int offset) const;

    /**
     * @brief 解码单个字段
     * @param data 原始数据
     * @param offset 当前偏移
     * @return {字段信息Map, 新偏移}
     */
    QPair<QVariantMap, int> decodeField(const QByteArray& data, int offset) const;

    /**
     * @brief 写入varint编码到缓冲区
     * @param buf 目标缓冲区
     * @param value 要编码的值
     */
    void writeVarint(QBuffer& buf, quint64 value) const;

    QString m_protoFilePath;  ///< .proto文件路径
    bool    m_loaded = false; ///< 是否已加载模式
};

#endif // PROTOBUF_DECODER_H

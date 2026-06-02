/**
 * @file FlatBuffersDecoder.h
 * @brief FlatBuffers解码器 — 加载.fbs模式并解码消息
 *
 * 解析FlatBuffers二进制格式，以零拷贝方式访问字段。
 */
#ifndef FLATBUFFERS_DECODER_H
#define FLATBUFFERS_DECODER_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QVariantMap>

/**
 * @brief FlatBuffers解码器
 * 加载.fbs模式定义，解码FlatBuffers二进制数据。
 */
class FlatBuffersDecoder : public QObject {
    Q_OBJECT

public:
    explicit FlatBuffersDecoder(QObject* parent = nullptr);

    /**
     * @brief 加载.fbs模式文件
     * @param filePath .fbs文件路径
     * @return 是否加载成功
     */
    bool loadFbsFile(const QString& filePath);

    /** @brief 是否已加载模式 */
    bool isLoaded() const;

    /**
     * @brief 解码FlatBuffers二进制消息
     * @param data 原始二进制数据
     * @return 解码后的字段映射
     */
    QVariantMap decodeMessage(const QByteArray& data);

private:
    /**
     * @brief 解析表（Table）结构
     * @param data 原始数据
     * @param tableOffset 表偏移量
     * @return 字段映射
     */
    QVariantMap parseTable(const QByteArray& data, int tableOffset) const;

    /**
     * @brief 读取指定偏移处的偏移量（32位）
     */
    quint32 readOffset(const QByteArray& data, int offset) const;

    QString m_fbsFilePath;  ///< .fbs文件路径
    bool    m_loaded = false; ///< 是否已加载模式
};

#endif // FLATBUFFERS_DECODER_H

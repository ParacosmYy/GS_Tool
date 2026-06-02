/**
 * @file PacketBuilder.h
 * @brief 数据包构建器
 * @author Serial Tool Team
 * @date 2026-06-02
 *
 * 根据字段定义构建二进制数据包，支持模板保存/加载。
 */

#ifndef PACKETBUILDER_H
#define PACKETBUILDER_H

#include <QByteArray>
#include <QList>
#include <QObject>
#include <QString>

#include "utils/packet/PacketField.h"

/**
 * @class PacketBuilder
 * @brief 数据包构建引擎，管理字段列表并生成二进制包
 */
class PacketBuilder : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit PacketBuilder(QObject *parent = nullptr);

    /**
     * @brief 添加字段
     * @param field 字段定义
     */
    void addField(const PacketField &field);

    /**
     * @brief 移除字段
     * @param index 字段索引
     */
    void removeField(int index);

    /**
     * @brief 获取所有字段
     * @return 字段列表
     */
    QList<PacketField> fields() const;

    /**
     * @brief 根据字段定义构建二进制数据包
     * @return 构建完成的字节数组
     */
    QByteArray buildPacket() const;

    /**
     * @brief 设置是否启用尾部校验
     * @param enabled 是否启用
     */
    void setChecksumSuffix(bool enabled);

    /**
     * @brief 从 JSON 文件加载模板
     * @param filePath JSON 文件路径
     * @return 是否加载成功
     */
    bool loadTemplate(const QString &filePath);

    /**
     * @brief 保存模板到 JSON 文件
     * @param filePath 保存路径
     * @return 是否保存成功
     */
    bool saveTemplate(const QString &filePath) const;

signals:
    /**
     * @brief 数据包构建完成信号
     * @param packet 构建的数据
     */
    void packetBuilt(const QByteArray &packet);

    /**
     * @brief 字段更新信号
     * @param index 更新的字段索引
     */
    void fieldUpdated(int index);

private:
    QList<PacketField> m_fields;        ///< 字段列表
    bool m_checksumEnabled = true;      ///< 是否添加尾部校验
};

#endif // PACKETBUILDER_H

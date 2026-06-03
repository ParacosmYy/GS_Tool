/**
 * @file PacketBuilder.h
 * @brief 数据包构建器
 * @author Serial Tool Team
 * @date 2026-06-02
 *
 * 根据字段定义构建二进制数据包，支持模板保存/加载到SettingsManager、
 * 字段校验、十六进制格式化输出与解析。
 */

#ifndef PACKETBUILDER_H
#define PACKETBUILDER_H

#include <QByteArray>
#include <QList>
#include <QMap>
#include <QObject>
#include <QString>
#include <QVariantMap>

#include "utils/packet/PacketField.h"

/**
 * @class PacketBuilder
 * @brief 数据包构建引擎，管理字段列表并生成二进制包
 *
 * 提供完整的字段管理、模板持久化(基于SettingsManager)、
 * 字段合法性校验和十六进制字符串互转能力。
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

    /**
     * @brief 从 SettingsManager 加载命名模板
     * @param name 模板名称
     * @return 是否加载成功
     */
    bool loadTemplate(const QString &name, const QVariantMap &options);

    /**
     * @brief 保存命名模板到 SettingsManager
     * @param name 模板名称
     * @param fields 字段映射 (可选，为空则使用当前字段)
     * @return 是否保存成功
     */
    bool saveTemplate(const QString &name, const QVariantMap &fields);

    /**
     * @brief 校验所有字段的偏移和长度是否合法
     * @return true 所有字段合法
     */
    bool validate() const;

    /**
     * @brief 将构建的数据包格式化为十六进制字符串
     * @return 格式化后的十六进制字符串 (如 "AA BB CC DD")
     */
    QString toHexString() const;

    /**
     * @brief 从十六进制字符串解析字段值
     * @param hex 十六进制字符串 (支持空格分隔或连续)
     * @return 解析后的字段列表
     */
    static QList<PacketField> fromHexString(const QString &hex);

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

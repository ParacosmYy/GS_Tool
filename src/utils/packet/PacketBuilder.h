/**
 * @file PacketBuilder.h
 * @brief 数据包构建器 - 根据字段定义构建二进制数据包
 *
 * 支持模板保存/加载到SettingsManager、字段校验、
 * 十六进制格式化输出与解析。
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
 * @brief 数据包构建引擎，管理字段列表并生成二进制包
 *
 * 提供完整的字段管理、模板持久化、字段合法性校验和十六进制字符串互转能力。
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

    /** @brief 添加字段 */
    void addField(const PacketField &field);

    /** @brief 移除字段 */
    void removeField(int index);

    /** @brief 获取所有字段 */
    QList<PacketField> fields() const;

    /** @brief 根据字段定义构建二进制数据包 */
    QByteArray buildPacket() const;

    /** @brief 设置是否启用尾部校验 */
    void setChecksumSuffix(bool enabled);

    /** @brief 从JSON文件加载模板 */
    bool loadTemplate(const QString &filePath);

    /** @brief 保存模板到JSON文件 */
    bool saveTemplate(const QString &filePath) const;

    /** @brief 从SettingsManager加载命名模板 */
    bool loadTemplate(const QString &name, const QVariantMap &options);

    /** @brief 保存命名模板到SettingsManager */
    bool saveTemplate(const QString &name, const QVariantMap &fields);

    /** @brief 校验所有字段的偏移和长度 */
    bool validate() const;

    /** @brief 格式化为十六进制字符串 */
    QString toHexString() const;

    /** @brief 从十六进制字符串解析字段值 */
    static QList<PacketField> fromHexString(const QString &hex);

    /** @brief 获取累计构建次数 */
    quint64 totalBuilds() const;
    /** @brief 获取累计构建字节数 */
    quint64 totalBytesBuilt() const;
    /** @brief 获取累计模板加载次数 */
    quint64 totalTemplateLoads() const;
    /** @brief 重置统计计数器 */
    void resetStats();

signals:
    void packetBuilt(const QByteArray &packet);
    void fieldUpdated(int index);

private:
    QList<PacketField> m_fields;        ///< 字段列表
    bool m_checksumEnabled = true;      ///< 是否添加尾部校验

    mutable quint64 m_totalBuilds = 0;       ///< 累计构建次数
    mutable quint64 m_totalBytesBuilt = 0;   ///< 累计构建字节数
    quint64 m_totalTemplateLoads = 0;        ///< 累计模板加载次数
};

#endif // PACKETBUILDER_H

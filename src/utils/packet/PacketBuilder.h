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

    /** @brief 添加字段到列表 @param field 要添加的字段定义 */
    void addField(const PacketField &field);

    /** @brief 移除指定索引的字段 @param index 字段索引 */
    void removeField(int index);

    /** @brief 获取所有字段 @return 字段列表 */
    QList<PacketField> fields() const;

    /** @brief 根据字段定义构建二进制数据包 @return 构建的QByteArray */
    QByteArray buildPacket() const;

    /** @brief 设置是否启用尾部校验 @param enabled true=添加CRC16尾，false=不添加 */
    void setChecksumSuffix(bool enabled);

    /** @brief 从JSON文件加载模板 @param filePath JSON文件路径 @return true加载成功 */
    bool loadTemplate(const QString &filePath);

    /** @brief 保存模板到JSON文件 @param filePath 输出路径 @return true保存成功 */
    bool saveTemplate(const QString &filePath) const;

    /** @brief 从SettingsManager加载命名模板 @param name 模板名称 @param options 预留参数 @return true加载成功 */
    bool loadTemplate(const QString &name, const QVariantMap &options);

    /** @brief 保存命名模板到SettingsManager @param name 模板名称 @param fields 字段映射 @return true保存成功 */
    bool saveTemplate(const QString &name, const QVariantMap &fields);

    /** @brief 校验所有字段的偏移和长度是否合法 @return true合法 */
    bool validate() const;

    /** @brief 将构建的数据包格式化为十六进制字符串 @return 如"AA BB CC DD"格式 */
    QString toHexString() const;

    /** @brief 从十六进制字符串解析字段列表 @param hex 空格分隔的十六进制字符串 @return 解析后的字段列表 */
    static QList<PacketField> fromHexString(const QString &hex);

    /** @brief 获取累计构建次数 @return 构建总次数 */
    quint64 totalBuilds() const;
    /** @brief 获取累计构建字节数 @return 字节总数 */
    quint64 totalBytesBuilt() const;
    /** @brief 获取累计模板加载次数 @return 加载总次数 */
    quint64 totalTemplateLoads() const;
    /** @brief 获取累计添加字段次数 @return 字段添加总次数 */
    quint64 totalFieldAdds() const { return m_totalFieldAdds; }
    /** @brief 获取累计发送数据包次数 @return 发送总次数 */
    quint64 totalSends() const { return m_totalSends; }
    /** @brief 重置统计计数器 */
    void resetStats();

signals:
    /** @brief 数据包构建完成信号 @param packet 构建的数据包字节 */
    void packetBuilt(const QByteArray &packet);
    /** @brief 字段更新信号 @param index 更新的字段索引 */
    void fieldUpdated(int index);

private:
    QList<PacketField> m_fields;        ///< 字段列表
    bool m_checksumEnabled = true;      ///< 是否添加尾部校验

    mutable quint64 m_totalBuilds = 0;       ///< 累计构建次数
    mutable quint64 m_totalBytesBuilt = 0;   ///< 累计构建字节数
    quint64 m_totalTemplateLoads = 0;        ///< 累计模板加载次数
    mutable quint64 m_totalFieldAdds = 0;    ///< 累计添加字段次数
    mutable quint64 m_totalSends = 0;        ///< 累计发送数据包次数
};

#endif // PACKETBUILDER_H

/**
 * @file ProtocolFieldMapper.h
 * @brief 协议字段到图表通道的映射器
 *
 * 管理协议解析结果（字段名→值）到图表数据通道的映射关系，
 * 并提供批量应用映射的方法。
 */

#ifndef PROTOCOL_FIELD_MAPPER_H
#define PROTOCOL_FIELD_MAPPER_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QVariantMap>

class ChartModel;

/**
 * @class ProtocolFieldMapper
 * @brief 协议字段→图表通道映射管理器
 *
 * 维护一个「字段名 → 通道名」的映射表，当协议引擎解析出新帧后，
 * 可调用 applyMappings() 将指定字段值推送到 ChartModel。
 */
class ProtocolFieldMapper : public QObject
{
    Q_OBJECT

public:
    /** @brief 构造函数 */
    explicit ProtocolFieldMapper(QObject *parent = nullptr);

    /** @brief 析构函数 */
    ~ProtocolFieldMapper() override;

    /**
     * @brief 添加字段到通道的映射
     * @param fieldName 协议字段名称
     * @param channelName 图表通道名称
     */
    void addMapping(const QString &fieldName, const QString &channelName);

    /**
     * @brief 移除指定字段的映射
     * @param fieldName 要移除的字段名称
     */
    void removeMapping(const QString &fieldName);

    /**
     * @brief 获取当前所有映射
     * @return 字段名→通道名的映射表
     */
    QMap<QString, QString> mappings() const;

    /**
     * @brief 将解析结果按映射应用到图表模型
     * @param parsedFields 协议引擎解析出的字段名→值映射
     * @param chartModel 目标图表数据模型
     */
    void applyMappings(const QVariantMap &parsedFields, ChartModel *chartModel);

private:
    QMap<QString, QString> m_mappings; ///< 字段名 → 图表通道名映射表
};

#endif // PROTOCOL_FIELD_MAPPER_H

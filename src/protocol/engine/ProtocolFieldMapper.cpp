/**
 * @file ProtocolFieldMapper.cpp
 * @brief 协议字段到图表通道的映射器实现
 */

#include "protocol/engine/ProtocolFieldMapper.h"

/**
 * @brief 构造函数
 * @param parent 父对象指针
 */
ProtocolFieldMapper::ProtocolFieldMapper(QObject *parent)
    : QObject(parent)
{
}

/**
 * @brief 析构函数
 */
ProtocolFieldMapper::~ProtocolFieldMapper() = default;

/**
 * @brief 添加字段到通道的映射
 *
 * 建立协议字段名与图表通道名的对应关系。若该字段已有映射，
 * 则覆盖旧的通道名。
 *
 * @paramFieldName 协议字段名称
 * @param channelName 对应的图表通道名称
 */
void ProtocolFieldMapper::addMapping(const QString &fieldName, const QString &channelName)
{
    m_mappings.insert(fieldName, channelName);
}

/**
 * @brief 移除指定字段的映射
 *
 * 从映射表中删除指定字段名的条目。若字段不存在则无操作。
 *
 * @param fieldName 要移除的字段名称
 */
void ProtocolFieldMapper::removeMapping(const QString &fieldName)
{
    m_mappings.remove(fieldName);
}

/**
 * @brief 获取当前所有映射
 * @return 字段名→通道名的映射表
 */
QMap<QString, QString> ProtocolFieldMapper::mappings() const
{
    return m_mappings;
}

/**
 * @brief 将解析结果按映射应用到图表模型
 *
 * 遍历当前映射表，从 parsedFields 中提取对应字段值，
 * 并推送到 chartModel 的指定通道中。
 *
 * @param parsedFields 协议引擎解析出的字段名→值映射
 * @param chartModel 目标图表数据模型
 */
void ProtocolFieldMapper::applyMappings(const QVariantMap &parsedFields, ChartModel *chartModel)
{
    Q_UNUSED(parsedFields)
    Q_UNUSED(chartModel)
    // TODO: 遍历 m_mappings，从 parsedFields 取值推送到 chartModel
}

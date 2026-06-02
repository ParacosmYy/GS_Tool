/**
 * @file ProtocolFieldMapper.cpp
 * @brief 协议字段到图表通道映射器的实现
 *
 * 提供字段名→通道名的映射管理，以及将解析后的协议字段数据
 * 推送到图表模型或通过信号通知的功能。
 */

#include "protocol/engine/ProtocolFieldMapper.h"

#include <QVariant>

// ---------------------------------------------------------------------------
// 构造 / 析构
// ---------------------------------------------------------------------------

/**
 * @brief 构造函数
 * @param parent 父对象指针
 */
ProtocolFieldMapper::ProtocolFieldMapper(QObject *parent)
    : QObject(parent)
{
}

/**
 * @brief 析构函数 — 清理映射表
 */
ProtocolFieldMapper::~ProtocolFieldMapper()
{
    m_mappings.clear();
}

// ---------------------------------------------------------------------------
// 映射管理
// ---------------------------------------------------------------------------

/**
 * @brief 添加或更新字段到通道的映射
 *
 * 若 fieldName 已存在则更新对应通道名，否则新增映射。
 * 空字段名或空通道名会被忽略。
 *
 * @param fieldName 协议字段名称
 * @param channelName 图表通道名称
 */
void ProtocolFieldMapper::addMapping(const QString &fieldName,
                                     const QString &channelName)
{
    if (fieldName.isEmpty() || channelName.isEmpty()) {
        return;
    }
    m_mappings[fieldName] = channelName;
}

/**
 * @brief 移除指定字段的映射
 *
 * 从映射表中删除指定字段名的条目，不存在则无操作。
 *
 * @param fieldName 要移除的字段名称
 */
void ProtocolFieldMapper::removeMapping(const QString &fieldName)
{
    m_mappings.remove(fieldName);
}

/**
 * @brief 获取当前所有映射
 * @return 字段名→通道名的映射表（副本）
 */
QMap<QString, QString> ProtocolFieldMapper::mappings() const
{
    return m_mappings;
}

/**
 * @brief 获取当前映射数量
 * @return 映射条目数
 */
int ProtocolFieldMapper::mappingCount() const
{
    return m_mappings.size();
}

/**
 * @brief 检查指定字段是否已有映射
 * @param fieldName 字段名称
 * @return 存在映射返回 true，否则 false
 */
bool ProtocolFieldMapper::hasMapping(const QString &fieldName) const
{
    return m_mappings.contains(fieldName);
}

/**
 * @brief 清除所有映射关系
 */
void ProtocolFieldMapper::clearMappings()
{
    m_mappings.clear();
}

// ---------------------------------------------------------------------------
// 数据应用
// ---------------------------------------------------------------------------

/**
 * @brief 将解析结果按映射应用到图表模型
 *
 * 遍历当前映射表，对每个映射检查解析字段中是否存在对应的值。
 * 若值为数值类型（可转换为 double），则：
 *   1. 通过 ChartModel::onFrameParsed 槽推送数据
 *   2. 发射 channelDataMapped 信号通知外部组件
 *
 * 非数值字段（字符串、字节流等）会被跳过。
 *
 * @param parsedFields 协议引擎解析出的 字段名→值 映射
 * @param chartModel   目标图表数据模型（可为 nullptr，此时仅发射信号）
 */
void ProtocolFieldMapper::applyMappings(const QVariantMap &parsedFields,
                                        ChartModel *chartModel)
{
    if (parsedFields.isEmpty()) {
        return;
    }

    // 遍历所有映射规则：字段名 → 通道名
    for (auto it = m_mappings.constBegin(); it != m_mappings.constEnd(); ++it) {
        const QString &fieldName   = it.key();
        const QString &channelName = it.value();

        // 在解析结果中查找该字段
        const QVariant fieldValue = parsedFields.value(fieldName);
        if (!fieldValue.isValid()) {
            continue;
        }

        // 仅处理可转换为 double 的数值类型
        if (!fieldValue.canConvert<double>()) {
            continue;
        }

        bool ok = false;
        const double value = fieldValue.toDouble(&ok);
        if (!ok) {
            continue;
        }

        // 通过信号通知外部组件（ChartModel 等由上层 connect 处理）
        emit channelDataMapped(channelName, value);
    }
}

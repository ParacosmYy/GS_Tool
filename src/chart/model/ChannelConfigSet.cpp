/**
 * @file ChannelConfigSet.cpp
 * @brief 通道配置集合实现 — 通道增删查改、批量计算和JSON序列化
 *
 * 从 ChannelConfig.cpp 拆分而来，包含 ChannelConfigSet 类的所有方法:
 *   - 通道管理: removeChannel / findChannel / channels
 *   - 批量计算: computeAll
 *   - JSON序列化: toJson / fromJson
 *
 * ChannelConfig 单个通道的计算和序列化见 ChannelConfig.cpp。
 */

#include "chart/model/ChannelConfig.h"

#include <QJsonArray>
#include <cmath>

// ============================================================
// ChannelConfigSet 实现
// ============================================================

/** @brief 按displayName移除通道，更新移除计数和配置变更计数 @param displayName 通道显示名 */
void ChannelConfigSet::removeChannel(const QString& displayName)
{
    for (int i = 0; i < m_channels.size(); ++i) {
        if (m_channels[i].displayName == displayName) {
            m_channels.removeAt(i);
            ++m_totalConfigChanges;
            ++m_totalChannelRemoves;
            return;
        }
    }
}

/** @brief 返回通道列表(只读引用) @return 通道配置向量 */
const QVector<ChannelConfig>& ChannelConfigSet::channels() const
{
    return m_channels;
}

/** @brief 按名称查找通道(可修改) @param displayName 通道显示名 @return 通道指针，未找到返回nullptr */
ChannelConfig* ChannelConfigSet::findChannel(const QString& displayName)
{
    for (auto& cfg : m_channels) {
        if (cfg.displayName == displayName) {
            return &cfg;
        }
    }
    return nullptr;
}

/** @brief 按名称查找通道(只读) @param displayName 通道显示名 @return 通道const指针，未找到返回nullptr */
const ChannelConfig* ChannelConfigSet::findChannel(const QString& displayName) const
{
    for (const auto& cfg : m_channels) {
        if (cfg.displayName == displayName) {
            return &cfg;
        }
    }
    return nullptr;
}

/** @brief 计算所有启用通道的值(跳过禁用/字段缺失/NaN) @param fields 帧字段映射 @return 通道名→计算值映射 */
QMap<QString, double> ChannelConfigSet::computeAll(const QVariantMap& fields) const
{
    QMap<QString, double> result;

    for (const auto& cfg : m_channels) {
        // 跳过禁用的通道
        if (!cfg.enabled) {
            continue;
        }
        // 检查所需字段是否都存在
        if (!cfg.canCompute(fields)) {
            continue;
        }
        double value = cfg.compute(fields);
        // 跳过计算结果为NaN的通道（如除零）
        if (std::isnan(value)) {
            continue;
        }
        result[cfg.displayName] = value;
    }

    return result;
}

/** @brief 序列化通道集合为JSON对象 @return QJsonObject */
QJsonObject ChannelConfigSet::toJson() const
{
    QJsonArray channelsArr;
    for (const auto& cfg : m_channels) {
        channelsArr.append(cfg.toJson());
    }

    QJsonObject obj;
    obj["channels"] = channelsArr;
    return obj;
}

/** @brief 从JSON对象反序列化通道集合 @param obj JSON对象 @return ChannelConfigSet */
ChannelConfigSet ChannelConfigSet::fromJson(const QJsonObject& obj)
{
    ChannelConfigSet set;

    QJsonArray channelsArr = obj["channels"].toArray();
    for (const auto& val : channelsArr) {
        set.m_channels.append(ChannelConfig::fromJson(val.toObject()));
    }

    return set;
}

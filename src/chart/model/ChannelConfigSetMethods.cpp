/**
 * @file ChannelConfigSetMethods.cpp
 * @brief ChannelConfigSet集合操作方法 — 添加/默认生成/统计接口
 *
 * 从ChannelConfig.cpp拆分，包含addChannel、generateDefaults及统计getter/reset方法。
 */

#include "chart/model/ChannelConfig.h"
#include "protocol/parser/FrameDefinition.h"

// ============================================================
// ChannelConfigSet 集合操作
// ============================================================

/** @brief 添加通道配置到集合，更新添加计数和配置变更计数 @param config 通道配置 */
void ChannelConfigSet::addChannel(const ChannelConfig& config)
{
    m_channels.append(config);
    ++m_totalConfigChanges;
    ++m_totalChannelAdds;
    if (config.color.isValid()) {
        ++m_totalColorChanges;
    }
}

/** @brief 根据帧字段定义生成默认通道配置(自动分配颜色，跳过Raw类型字段) @param fields 帧字段定义列表 @return 默认ChannelConfigSet */
ChannelConfigSet ChannelConfigSet::generateDefaults(const QVector<FieldDef>& fields)
{
    ChannelConfigSet set;
    int colorIndex = 0;

    for (const auto& field : fields) {
        if (field.type == FieldDef::Raw) {
            continue;
        }

        ChannelConfig cfg;
        cfg.sourceMode = ChannelConfig::SourceMode::Direct;
        cfg.sourceField = field.name;
        cfg.scale = field.scale;
        cfg.offset = field.offsetVal;
        cfg.displayName = field.name;
        cfg.color = ChartColors::defaultColors()[colorIndex % ChartColors::defaultColors().size()];
        cfg.enabled = true;
        cfg.unit = field.unit;
        cfg.sampleDivisor = 1;

        set.addChannel(cfg);
        ++colorIndex;
    }

    return set;
}

// ============================================================
// 统计计数器接口
// ============================================================

/** @brief 返回配置变更总次数 @return 变更次数 */
quint64 ChannelConfigSet::totalConfigChanges() const
{
    return m_totalConfigChanges;
}

/** @brief 返回颜色变更总次数 @return 颜色变更次数 */
quint64 ChannelConfigSet::totalColorChanges() const
{
    return m_totalColorChanges;
}

/** @brief 重置所有配置统计计数器为初始值 */
void ChannelConfigSet::resetConfigStatistics()
{
    m_totalConfigChanges = 0;
    m_totalColorChanges = 0;
    m_totalChannelAdds = 0;
    m_totalChannelRemoves = 0;
}

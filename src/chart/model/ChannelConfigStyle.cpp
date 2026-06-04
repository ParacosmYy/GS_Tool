/**
 * @file ChannelConfigStyle.cpp
 * @brief 通道配置样式与统计实现 - 颜色分配、默认配置生成、配置统计计数器
 *
 * 职责拆分自 ChannelConfig.cpp:
 *   - 默认通道配置生成（含自动颜色分配）
 *   - 通道添加时的颜色变更统计
 *   - 配置变更/颜色变更统计计数器的查询与重置
 */

#include "chart/model/ChannelConfig.h"
#include "protocol/parser/FrameDefinition.h"

// ============================================================
// ChannelConfigSet -- 样式与默认生成
// ============================================================

/** @brief 添加通道配置到集合，同时更新配置变更与颜色变更统计计数器 @param config 待添加的通道配置 */
void ChannelConfigSet::addChannel(const ChannelConfig& config)
{
    m_channels.append(config);
    ++m_totalConfigChanges;
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
        // 跳过Raw类型字段，无法绘制波形
        if (field.type == FieldDef::Raw) {
            continue;
        }

        ChannelConfig cfg;
        cfg.sourceMode = ChannelConfig::SourceMode::Direct;
        cfg.sourceField = field.name;
        // Combine模式的字段在Direct模式下不需要设置
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

/** @brief 返回配置变更总次数（包括增删通道、属性修改） */
quint64 ChannelConfigSet::totalConfigChanges() const
{
    return m_totalConfigChanges;
}

/** @brief 返回颜色变更总次数 */
quint64 ChannelConfigSet::totalColorChanges() const
{
    return m_totalColorChanges;
}

/** @brief 重置所有配置统计计数器为初始值 */
void ChannelConfigSet::resetConfigStatistics()
{
    m_totalConfigChanges = 0;
    m_totalColorChanges = 0;
}

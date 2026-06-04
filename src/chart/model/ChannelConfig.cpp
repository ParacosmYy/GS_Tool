/**
 * @file ChannelConfig.cpp
 * @brief 通道配置实现 - 波形通道的计算逻辑和JSON序列化
 *
 * 支持两种数据源模式:
 *   - Direct: 从帧字段直接取值
 *   - Combine: 对两个字段执行加减乘除运算
 */

#include "chart/model/ChannelConfig.h"

#include <QVariant>
#include <QJsonValue>
#include <cmath>

// ============================================================
// ChannelConfig 实现
// ============================================================

/** @brief 计算通道值(Direct模式取字段值, Combine模式执行四则运算, 最终应用线性变换) @param fields 帧字段映射 @return 计算结果，字段缺失/计算失败返回NaN */
double ChannelConfig::compute(const QVariantMap& fields) const
{
    // 禁用的通道不参与计算
    if (!enabled) {
        return qQNaN();
    }

    double rawValue = 0.0;

    if (sourceMode == SourceMode::Direct) {
        // Direct模式: 从fields中取sourceField对应的值
        if (!fields.contains(sourceField)) {
            return qQNaN();
        }
        bool ok = false;
        rawValue = fields.value(sourceField).toDouble(&ok);
        if (!ok) {
            return qQNaN();
        }
    } else {
        // Combine模式: 从fields中取sourceFieldA和sourceFieldB
        if (!fields.contains(sourceFieldA) || !fields.contains(sourceFieldB)) {
            return qQNaN();
        }

        bool okA = false, okB = false;
        double valA = fields.value(sourceFieldA).toDouble(&okA);
        double valB = fields.value(sourceFieldB).toDouble(&okB);
        if (!okA || !okB) {
            return qQNaN();
        }

        // 按combineOp执行运算
        switch (combineOp) {
        case CombineOp::Add:
            rawValue = valA + valB;
            break;
        case CombineOp::Subtract:
            rawValue = valA - valB;
            break;
        case CombineOp::Multiply:
            rawValue = valA * valB;
            break;
        case CombineOp::Divide:
            // 除零保护: B为0时返回NaN
            if (valB == 0.0) {
                return qQNaN();
            }
            rawValue = valA / valB;
            break;
        default:
            rawValue = qQNaN();
            break;
        }
    }

    // 应用线性变换: displayValue = scale * rawValue + offset
    return scale * rawValue + offset;
}

/** @brief 检查通道是否能从给定字段中计算值 @param fields 帧字段映射 @return true=字段齐全可计算 */
bool ChannelConfig::canCompute(const QVariantMap& fields) const
{
    if (!enabled) {
        return false;
    }

    if (sourceMode == SourceMode::Direct) {
        return fields.contains(sourceField);
    } else {
        // Combine模式需要两个字段都存在
        return fields.contains(sourceFieldA) && fields.contains(sourceFieldB);
    }
}

/** @brief 序列化通道配置为JSON对象 @return QJsonObject */
QJsonObject ChannelConfig::toJson() const
{
    QJsonObject obj;
    // sourceMode: 0=Direct, 1=Combine
    obj["sourceMode"] = static_cast<int>(sourceMode);
    obj["sourceField"] = sourceField;
    obj["sourceFieldA"] = sourceFieldA;
    obj["sourceFieldB"] = sourceFieldB;
    // combineOp: 0=Add, 1=Subtract, 2=Multiply, 3=Divide
    obj["combineOp"] = static_cast<int>(combineOp);
    obj["scale"] = scale;
    obj["offset"] = offset;
    obj["displayName"] = displayName;
    // 颜色序列化为HEX字符串，空字符串表示自动分配
    obj["color"] = color.isValid() ? color.name(QColor::HexRgb) : QString("");
    obj["enabled"] = enabled;
    obj["unit"] = unit;
    obj["sampleDivisor"] = sampleDivisor;
    // yAxisSide: 0=Left, 1=Right
    obj["yAxisSide"] = static_cast<int>(yAxisSide);
    obj["autoYRange"] = autoYRange;
    return obj;
}

/** @brief 从JSON对象反序列化通道配置(含枚举范围校验) @param obj JSON对象 @return ChannelConfig */
ChannelConfig ChannelConfig::fromJson(const QJsonObject& obj)
{
    ChannelConfig cfg;
    // 枚举值范围校验: 防止损坏的JSON导致 static_cast 产生无效枚举值
    int sm = obj["sourceMode"].toInt(0);
    cfg.sourceMode = (sm >= 0 && sm <= 1) ? static_cast<SourceMode>(sm) : SourceMode::Direct;
    cfg.sourceField = obj["sourceField"].toString();
    cfg.sourceFieldA = obj["sourceFieldA"].toString();
    cfg.sourceFieldB = obj["sourceFieldB"].toString();
    int co = obj["combineOp"].toInt(0);
    cfg.combineOp = (co >= 0 && co <= 3) ? static_cast<CombineOp>(co) : CombineOp::Add;
    cfg.scale = obj["scale"].toDouble(1.0);
    if (cfg.scale == 0.0) cfg.scale = 1.0;
    cfg.offset = obj["offset"].toDouble(0.0);
    cfg.displayName = obj["displayName"].toString();
    // 颜色反序列化: 空字符串表示无效颜色（自动分配）
    QString colorStr = obj["color"].toString();
    if (!colorStr.isEmpty()) {
        cfg.color = QColor(colorStr);
    }
    cfg.enabled = obj["enabled"].toBool(true);
    cfg.unit = obj["unit"].toString();
    int sd = obj["sampleDivisor"].toInt(1);
    cfg.sampleDivisor = (sd >= 1) ? sd : 1;  // 降采样因子最小为1
    // yAxisSide 反序列化（0=Left, 1=Right，默认Left）
    int yas = obj["yAxisSide"].toInt(0);
    cfg.yAxisSide = (yas >= 0 && yas <= 1) ? static_cast<YAxisSide>(yas) : YAxisSide::Left;
    cfg.autoYRange = obj["autoYRange"].toBool(true);
    return cfg;
}

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


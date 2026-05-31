#include "chart/ChannelConfig.h"
#include "protocol/FrameDefinition.h"

#include <QVariant>
#include <QJsonValue>
#include <cmath>

// ============================================================
// ChannelConfig 实现
// ============================================================

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
        }
    }

    // 应用线性变换: displayValue = scale * rawValue + offset
    return scale * rawValue + offset;
}

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
    return obj;
}

ChannelConfig ChannelConfig::fromJson(const QJsonObject& obj)
{
    ChannelConfig cfg;
    cfg.sourceMode = static_cast<SourceMode>(obj["sourceMode"].toInt(0));
    cfg.sourceField = obj["sourceField"].toString();
    cfg.sourceFieldA = obj["sourceFieldA"].toString();
    cfg.sourceFieldB = obj["sourceFieldB"].toString();
    cfg.combineOp = static_cast<CombineOp>(obj["combineOp"].toInt(0));
    cfg.scale = obj["scale"].toDouble(1.0);
    cfg.offset = obj["offset"].toDouble(0.0);
    cfg.displayName = obj["displayName"].toString();
    // 颜色反序列化: 空字符串表示无效颜色（自动分配）
    QString colorStr = obj["color"].toString();
    if (!colorStr.isEmpty()) {
        cfg.color = QColor(colorStr);
    }
    cfg.enabled = obj["enabled"].toBool(true);
    cfg.unit = obj["unit"].toString();
    cfg.sampleDivisor = obj["sampleDivisor"].toInt(1);
    return cfg;
}

// ============================================================
// ChannelConfigSet 实现
// ============================================================

// 默认颜色表 -- 与ChartWidget::kDefaultColors保持一致
const QVector<QColor> ChannelConfigSet::kDefaultColors = {
    QColor("#89b4fa"), QColor("#a6e3a1"), QColor("#f9e2af"),
    QColor("#f38ba8"), QColor("#94e2d5"), QColor("#cba6f7"),
    QColor("#fab387"), QColor("#74c7ec"), QColor("#f5c2e7"),
    QColor("#b4befe")
};

void ChannelConfigSet::addChannel(const ChannelConfig& config)
{
    m_channels.append(config);
}

void ChannelConfigSet::removeChannel(const QString& displayName)
{
    for (int i = 0; i < m_channels.size(); ++i) {
        if (m_channels[i].displayName == displayName) {
            m_channels.removeAt(i);
            return;
        }
    }
}

const QVector<ChannelConfig>& ChannelConfigSet::channels() const
{
    return m_channels;
}

ChannelConfig* ChannelConfigSet::findChannel(const QString& displayName)
{
    for (auto& cfg : m_channels) {
        if (cfg.displayName == displayName) {
            return &cfg;
        }
    }
    return nullptr;
}

const ChannelConfig* ChannelConfigSet::findChannel(const QString& displayName) const
{
    for (const auto& cfg : m_channels) {
        if (cfg.displayName == displayName) {
            return &cfg;
        }
    }
    return nullptr;
}

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

ChannelConfigSet ChannelConfigSet::fromJson(const QJsonObject& obj)
{
    ChannelConfigSet set;

    QJsonArray channelsArr = obj["channels"].toArray();
    for (const auto& val : channelsArr) {
        set.m_channels.append(ChannelConfig::fromJson(val.toObject()));
    }

    return set;
}

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
        cfg.color = kDefaultColors[colorIndex % kDefaultColors.size()];
        cfg.enabled = true;
        cfg.unit = field.unit;
        cfg.sampleDivisor = 1;

        set.addChannel(cfg);
        ++colorIndex;
    }

    return set;
}

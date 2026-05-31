#ifndef CHANNELCONFIG_H
#define CHANNELCONFIG_H

#include <QString>
#include <QColor>
#include <QVector>
#include <QMap>
#include <QVariantMap>
#include <QJsonObject>
#include <QJsonArray>
#include "chart/ChartColors.h"

// 前向声明，避免直接依赖协议层头文件
struct FieldDef;

// 单个通道的配置 -- 描述如何从协议帧字段映射到一条曲线
// 支持两种模式:
//   Direct:  单字段直接映射，公式 y = scale * field_value + offset
//   Combine: 双字段组合公式，公式 y = scale * (fieldA op fieldB) + offset
struct ChannelConfig {

    // ---- 数据源映射 ----

    // 数据源类型: 单字段直接映射 或 双字段组合公式
    enum class SourceMode {
        Direct,         // y = scale * field_value + offset
        Combine         // y = scale * (fieldA op fieldB) + offset
    };
    SourceMode sourceMode = SourceMode::Direct;

    // 单字段模式的源字段名（对应FrameDefinition中FieldDef.name）
    QString sourceField;

    // 双字段组合模式的两个源字段名
    QString sourceFieldA;
    QString sourceFieldB;

    // 组合运算符（仅Combine模式使用）
    enum class CombineOp {
        Add,        // A + B
        Subtract,   // A - B
        Multiply,   // A * B
        Divide      // A / B
    };
    CombineOp combineOp = CombineOp::Add;

    // 线性变换系数: displayValue = scale * rawValue + offset
    double scale = 1.0;
    double offset = 0.0;

    // ---- 通道显示属性 ----

    // 通道名称（显示在图例中，可与sourceField不同）
    QString displayName;

    // 通道颜色（无效颜色表示使用自动分配）
    QColor color;

    // 是否启用此通道（禁用时不接收数据、不显示曲线）
    bool enabled = true;

    // 单位（显示在Y轴标题或图例中）
    QString unit;

    // ---- 采样控制 ----

    // 降采样: 每隔 sampleDivisor 帧取一个数据点 (1=不降采样)
    int sampleDivisor = 1;

    // ---- 计算接口 ----

    // 从帧解析结果中计算本通道的数值
    // fields: FrameParser发出的QVariantMap
    // 返回: 计算后的double值; 如果字段不存在或通道被禁用，返回NaN
    double compute(const QVariantMap& fields) const;

    // 判断帧数据中是否包含本通道所需的所有字段
    bool canCompute(const QVariantMap& fields) const;

    // ---- JSON序列化 ----

    // 序列化为JSON对象，用于通过SettingsManager持久化
    QJsonObject toJson() const;

    // 从JSON对象反序列化，用于从SettingsManager加载配置
    static ChannelConfig fromJson(const QJsonObject& obj);
};

// 通道配置集合 -- 管理多个ChannelConfig，负责整体序列化和批量计算
// 典型用法:
//   1. 从FrameDefinition生成默认配置: ChannelConfigSet::generateDefaults(def.fields)
//   2. 用户在配置面板中调整各通道参数
//   3. 每帧到来时调用 computeAll(fields) 获取所有通道的值
//   4. 通过 toJson()/fromJson() 持久化到SettingsManager
class ChannelConfigSet {
public:
    // 添加通道配置
    void addChannel(const ChannelConfig& config);

    // 移除通道配置（按displayName匹配）
    void removeChannel(const QString& displayName);

    // 获取所有通道配置（只读）
    const QVector<ChannelConfig>& channels() const;

    // 按displayName查找通道配置（返回nullptr表示未找到）
    ChannelConfig* findChannel(const QString& displayName);
    const ChannelConfig* findChannel(const QString& displayName) const;

    // 从帧解析结果中计算所有启用通道的值
    // 返回: QMap<displayName, value>，仅包含能成功计算的通道
    // 跳过: 被禁用的通道、字段不存在的通道
    QMap<QString, double> computeAll(const QVariantMap& fields) const;

    // ---- JSON序列化 ----

    // 序列化为JSON对象，格式: { "channels": [ ... ] }
    QJsonObject toJson() const;

    // 从JSON对象反序列化
    static ChannelConfigSet fromJson(const QJsonObject& obj);

    // 根据当前FrameDefinition的字段列表，生成默认通道配置
    // 每个数值字段生成一个Direct模式的ChannelConfig
    // 跳过type为Raw的字段
    // displayName默认等于字段名，颜色使用kDefaultColors自动分配
    static ChannelConfigSet generateDefaults(const QVector<FieldDef>& fields);

private:
    QVector<ChannelConfig> m_channels;
};

#endif // CHANNELCONFIG_H

/** @file ChannelConfig.h @brief 通道配置管理 -- JSON序列化的数据源映射和通道颜色配置。管理波形图通道的配置信息：数据源映射(协议字段->通道)、颜色、可见性等。支持JSON序列化/反序列化 */
#ifndef CHANNELCONFIG_H
#define CHANNELCONFIG_H

#include <QString>
#include <QColor>
#include <QVector>
#include <QMap>
#include <QVariantMap>
#include <QJsonObject>
#include <QJsonArray>
#include "chart/widget/ChartColors.h"
#include "chart/scale/YAxisManager.h"

/** @brief 前向声明，避免直接依赖协议层头文件 */
struct FieldDef;

/** @brief 单个通道的配置 -- 描述如何从协议帧字段映射到一条曲线。支持Direct(单字段y=scale*val+offset)和Combine(双字段y=scale*(A op B)+offset)两种模式 */
struct ChannelConfig {

    // ---- 数据源映射 ----

    /** @brief 数据源类型: 单字段直接映射 或 双字段组合公式 */
    enum class SourceMode {
        Direct,         ///< 单字段模式: y = scale * field_value + offset
        Combine         ///< 双字段组合模式: y = scale * (fieldA op fieldB) + offset
    };
    SourceMode sourceMode = SourceMode::Direct;  ///< 当前数据源模式

    QString sourceField;   ///< 单字段模式的源字段名（对应FrameDefinition中FieldDef.name）

    QString sourceFieldA;  ///< 双字段组合模式的第一个源字段名
    QString sourceFieldB;  ///< 双字段组合模式的第二个源字段名

    /** @brief 组合运算符（仅Combine模式使用） */
    enum class CombineOp {
        Add,        ///< A + B
        Subtract,   ///< A - B
        Multiply,   ///< A * B
        Divide      ///< A / B
    };
    CombineOp combineOp = CombineOp::Add;  ///< 当前组合运算符

    double scale = 1.0;   ///< 线性变换缩放系数: displayValue = scale * rawValue + offset
    double offset = 0.0;  ///< 线性变换偏移量

    // ---- 通道显示属性 ----

    QString displayName;  ///< 通道名称（显示在图例中，可与sourceField不同）

    QColor color;         ///< 通道颜色（无效颜色表示使用自动分配）

    bool enabled = true;  ///< 是否启用此通道（禁用时不接收数据、不显示曲线）

    QString unit;         ///< 单位文本（显示在Y轴标题或图例中）

    // ---- Y轴配置 ----

    YAxisSide yAxisSide = YAxisSide::Left;  ///< Y轴放置位置（左侧/右侧）

    bool autoYRange = true;  ///< 是否自动计算Y轴范围（true=根据数据自适应，false=手动固定）

    // ---- 采样控制 ----

    int sampleDivisor = 1;   ///< 降采样比率: 每隔 sampleDivisor 帧取一个数据点 (1=不降采样)

    // ---- 计算接口 ----
    double compute(const QVariantMap& fields) const; ///< 从帧解析结果中计算本通道数值(字段不存在返回NaN)
    bool canCompute(const QVariantMap& fields) const; ///< 判断帧数据是否包含本通道所需的所有字段

    // ---- JSON序列化 ----
    QJsonObject toJson() const;               ///< 序列化为JSON对象
    static ChannelConfig fromJson(const QJsonObject& obj); ///< 从JSON反序列化
};

/** @brief 通道配置集合 -- 管理多个ChannelConfig，负责整体序列化和批量计算。典型用法: generateDefaults()->调整参数->computeAll(fields)->toJson()/fromJson()持久化 */
class ChannelConfigSet {
public:
    void addChannel(const ChannelConfig& config); ///< 添加通道配置
    void removeChannel(const QString& displayName); ///< 移除通道配置(按displayName匹配)
    const QVector<ChannelConfig>& channels() const; ///< 获取所有通道配置(只读)
    ChannelConfig* findChannel(const QString& displayName); ///< 按名称查找通道(可修改)
    const ChannelConfig* findChannel(const QString& displayName) const; ///< 按名称查找通道(只读)
    QMap<QString, double> computeAll(const QVariantMap& fields) const; ///< 计算所有启用通道的值
    QJsonObject toJson() const;               ///< 序列化为JSON { "channels": [...] }
    static ChannelConfigSet fromJson(const QJsonObject& obj); ///< 从JSON反序列化
    static ChannelConfigSet generateDefaults(const QVector<FieldDef>& fields); ///< 从FieldDef生成默认配置

    // ---- 统计计数器接口 ----

    /** @brief 获取配置变更总次数（包括增删通道、属性修改） */
    quint64 totalConfigChanges() const;

    /** @brief 获取颜色变更总次数 */
    quint64 totalColorChanges() const;

    /** @brief 重置所有配置统计计数器为初始值 */
    void resetConfigStatistics();

private:
    QVector<ChannelConfig> m_channels;       ///< 通道配置列表

    // 统计计数器
    quint64 m_totalConfigChanges = 0;   ///< 配置变更总次数
    quint64 m_totalColorChanges = 0;    ///< 颜色变更总次数
};

#endif // CHANNELCONFIG_H

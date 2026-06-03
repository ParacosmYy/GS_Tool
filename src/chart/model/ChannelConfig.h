/**
 * @file ChannelConfig.h
 * @brief 通道配置管理 — JSON序列化的数据源映射和通道颜色配置
 *
 * 管理波形图通道的配置信息：数据源映射(协议字段→通道)、颜色、可见性等。
 * 支持JSON序列化/反序列化，可持久化到SettingsManager。
 */
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

/**
 * @brief 单个通道的配置 -- 描述如何从协议帧字段映射到一条曲线
 *
 * 支持两种映射模式:
 *   - Direct:  单字段直接映射，公式 y = scale * field_value + offset
 *   - Combine: 双字段组合公式，公式 y = scale * (fieldA op fieldB) + offset
 */
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

    /**
     * @brief 从帧解析结果中计算本通道的数值
     * @param fields FrameParser发出的QVariantMap
     * @return 计算后的double值; 如果字段不存在或通道被禁用，返回NaN
     */
    double compute(const QVariantMap& fields) const;

    /**
     * @brief 判断帧数据中是否包含本通道所需的所有字段
     * @param fields FrameParser发出的QVariantMap
     * @return true=字段齐全可计算, false=缺少必要字段
     */
    bool canCompute(const QVariantMap& fields) const;

    // ---- JSON序列化 ----

    /**
     * @brief 序列化为JSON对象，用于通过SettingsManager持久化
     * @return QJsonObject 包含所有通道配置字段
     */
    QJsonObject toJson() const;

    /**
     * @brief 从JSON对象反序列化，用于从SettingsManager加载配置
     * @param obj JSON对象
     * @return 反序列化得到的ChannelConfig
     */
    static ChannelConfig fromJson(const QJsonObject& obj);
};

/**
 * @brief 通道配置集合 -- 管理多个ChannelConfig，负责整体序列化和批量计算
 *
 * 典型用法:
 *   1. 从FrameDefinition生成默认配置: ChannelConfigSet::generateDefaults(def.fields)
 *   2. 用户在配置面板中调整各通道参数
 *   3. 每帧到来时调用 computeAll(fields) 获取所有通道的值
 *   4. 通过 toJson()/fromJson() 持久化到SettingsManager
 */
class ChannelConfigSet {
public:
    /** @brief 添加通道配置 @param config 要添加的通道配置 */
    void addChannel(const ChannelConfig& config);

    /** @brief 移除通道配置（按displayName匹配） @param displayName 要移除的通道名称 */
    void removeChannel(const QString& displayName);

    /** @brief 获取所有通道配置（只读） @return 通道配置向量的常引用 */
    const QVector<ChannelConfig>& channels() const;

    /**
     * @brief 按displayName查找通道配置
     * @param displayName 通道名称
     * @return 通道配置指针，未找到返回nullptr
     */
    ChannelConfig* findChannel(const QString& displayName);

    /**
     * @brief 按displayName查找通道配置（只读重载）
     * @param displayName 通道名称
     * @return 通道配置的只读指针，未找到返回nullptr
     */
    const ChannelConfig* findChannel(const QString& displayName) const;

    /**
     * @brief 从帧解析结果中计算所有启用通道的值
     *
     * 仅包含能成功计算的通道，跳过被禁用的通道和字段不存在的通道。
     * @param fields FrameParser发出的QVariantMap
     * @return QMap<displayName, value> 各通道计算结果
     */
    QMap<QString, double> computeAll(const QVariantMap& fields) const;

    // ---- JSON序列化 ----

    /**
     * @brief 序列化为JSON对象
     * @return QJsonObject，格式: { "channels": [ ... ] }
     */
    QJsonObject toJson() const;

    /**
     * @brief 从JSON对象反序列化
     * @param obj JSON对象
     * @return 反序列化得到的ChannelConfigSet
     */
    static ChannelConfigSet fromJson(const QJsonObject& obj);

    /**
     * @brief 根据FrameDefinition的字段列表，生成默认通道配置
     *
     * 每个数值字段生成一个Direct模式的ChannelConfig。
     * 跳过type为Raw的字段。displayName默认等于字段名，
     * 颜色使用kDefaultColors自动分配。
     * @param fields 帧定义中的字段列表
     * @return 自动生成的通道配置集合
     */
    static ChannelConfigSet generateDefaults(const QVector<FieldDef>& fields);

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

/**
 * @file DashboardModel.h
 * @brief 仪表盘配置模型 — 管理组件配置的增删查、数据通道管理与持久化
 */

#ifndef DASHBOARD_MODEL_H
#define DASHBOARD_MODEL_H

#include <QMap>
#include <QObject>
#include <QStringList>
#include <QVariantMap>
#include <QList>

/// @brief 仪表盘配置模型 — 负责组件配置的增删查、数据通道管理与持久化
class DashboardModel : public QObject
{
    Q_OBJECT

public:
    /** @brief 构造函数 @param parent 父对象 */
    explicit DashboardModel(QObject *parent = nullptr);

    /** @brief 析构函数 */
    ~DashboardModel() override;

    /** @brief 添加组件配置 @param config 组件配置 QVariantMap */
    void addComponentConfig(const QVariantMap &config);

    /** @brief 移除指定索引的组件配置 @param index 配置索引 */
    void removeComponentConfig(int index);

    /** @brief 获取所有组件配置 @return 配置列表 */
    QList<QVariantMap> componentConfigs() const;

    /** @brief 获取配置数量 @return 数量 */
    int configCount() const;

    /** @brief 将配置保存到文件 @param filePath 目标文件路径 @return true=成功 */
    bool saveToFile(const QString &filePath) const;

    /** @brief 从文件加载配置 @param filePath 源文件路径 @return true=成功 */
    bool loadFromFile(const QString &filePath);

    /** @brief 添加数据通道 @param name 通道名称 @param initialValue 初始值 */
    void addChannel(const QString &name, double initialValue = 0.0);

    /** @brief 移除数据通道 @param name 通道名称 */
    void removeChannel(const QString &name);

    /** @brief 更新通道值并通知视图 @param name 通道名称 @param value 新值 */
    void updateValue(const QString &name, double value);

    /** @brief 获取所有通道名称 @return 通道名称列表 */
    QStringList channelNames() const;

    /** @brief 获取指定通道的当前值 @param name 通道名称 @return 通道值，不存在返回0.0 */
    double value(const QString &name) const;

    /** @brief 获取指定通道的最小值 @param name 通道名称 @return 最小值 */
    double channelMin(const QString &name) const;

    /** @brief 获取指定通道的最大值 @param name 通道名称 @return 最大值 */
    double channelMax(const QString &name) const;

    /** @brief 获取指定通道的值变更次数 @param name 通道名称 @return 变更次数 */
    quint64 channelChangeCount(const QString &name) const;

    /** @brief 获取总更新次数（所有通道累计） @return 累计更新次数 */
    quint64 totalUpdateCount() const;

    /** @brief 重置所有通道统计（min/max/changeCount/totalUpdateCount） */
    void resetChannelStatistics();

    // ==================== 组件类型统计 ====================

    /** @brief 获取累计创建的组件总数(含已删除) @return 创建总数 */
    quint64 totalWidgetsCreated() const;

    /** @brief 获取累计删除的组件总数 @return 删除总数 */
    quint64 totalWidgetsRemoved() const;

    /** @brief 获取当前活跃组件数量 @return 当前配置数 */
    quint64 activeWidgetCount() const;

    /** @brief 获取指定类型的累计创建计数 @param type 组件类型(gauge/progressbar/led/numeric/chart) @return 该类型创建总数 */
    quint64 widgetsCreatedByType(const QString &type) const;

    // ==================== 布局变更跟踪 ====================

    /** @brief 获取累计布局变更次数(增/删组件触发) @return 变更总数 */
    quint64 layoutChanges() const;

    // ==================== 配置文件管理统计 ====================

    /** @brief 获取累计配置保存次数 @return 保存总数 */
    quint64 profileSaves() const;

    /** @brief 获取累计配置加载次数 @return 加载总数 */
    quint64 profileLoads() const;

    /** @brief 获取累计配置删除次数 @return 删除总数 */
    quint64 profileDeletes() const;

    // ==================== 布局生命周期统计 ====================

    /** @brief 获取累计布局变更次数(增/删组件触发) @return 变更总数 */
    quint64 totalLayoutChanges() const;

    /** @brief 获取累计组件添加次数 @return 添加总数 */
    quint64 totalWidgetAdditions() const;

    /** @brief 获取累计组件移除次数 @return 移除总数 */
    quint64 totalWidgetRemovals() const;

    /** @brief 获取累计布局保存次数 @return 保存总数 */
    quint64 totalLayoutSaves() const;

    /** @brief 获取累计布局加载次数 @return 加载总数 */
    quint64 totalLayoutLoads() const;

    /** @brief 重置所有统计计数器(通道+组件+布局+配置文件+生命周期) */
    void resetAllStatistics();

signals:
    /** @brief 配置发生变更时发射 */
    void configChanged();
    /** @brief 数据通道集合发生变化时发射(增/删通道) */
    void channelsChanged();
    /** @brief 指定通道的值发生更新 */
    void valueChanged(const QString &name, double value);

private:
    QList<QVariantMap> m_configs;                ///< 组件配置列表
    QMap<QString, double> m_channels;            ///< 数据通道 <名称, 当前值>
    QMap<QString, double> m_channelMin;          ///< 每个通道的最小值
    QMap<QString, double> m_channelMax;          ///< 每个通道的最大值
    QMap<QString, quint64> m_channelChangeCount; ///< 每个通道的值变更次数
    quint64 m_totalUpdateCount = 0;              ///< 所有通道累计更新次数

    // ---- 组件类型统计计数器 ----
    mutable quint64 m_totalWidgetsCreated = 0;       ///< 累计创建组件总数
    quint64 m_totalWidgetsRemoved = 0;               ///< 累计删除组件总数
    QMap<QString, quint64> m_widgetsCreatedByType;   ///< 按类型统计创建计数
    mutable quint64 m_layoutChanges = 0;             ///< 累计布局变更次数

    // ---- 配置文件管理统计计数器 ----
    mutable quint64 m_profileSaves = 0;              ///< 累计配置保存次数
    mutable quint64 m_profileLoads = 0;              ///< 累计配置加载次数
    mutable quint64 m_profileDeletes = 0;            ///< 累计配置删除次数

    // ---- 布局生命周期统计计数器 ----
    mutable quint64 m_totalLayoutChanges = 0;                ///< 累计布局变更次数
    mutable quint64 m_totalWidgetAdditions = 0;              ///< 累计组件添加次数
    mutable quint64 m_totalWidgetRemovals = 0;               ///< 累计组件移除次数
    mutable quint64 m_totalLayoutSaves = 0;                  ///< 累计布局保存次数
    mutable quint64 m_totalLayoutLoads = 0;                  ///< 累计布局加载次数
};

#endif // DASHBOARD_MODEL_H

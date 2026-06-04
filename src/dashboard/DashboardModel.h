/** @file DashboardModel.h @brief 仪表盘配置模型 — 管理组件配置的增删查、数据通道管理与持久化 */
#ifndef DASHBOARD_MODEL_H
#define DASHBOARD_MODEL_H

#include <QMap>
#include <QObject>
#include <QStringList>
#include <QVariantMap>
#include <QList>

/** @brief 仪表盘配置模型 — 负责组件配置的增删查、数据通道管理与持久化 */
class DashboardModel : public QObject {
    Q_OBJECT

public:
    /** @brief 构造仪表盘配置模型 @param parent 父对象指针 */
    explicit DashboardModel(QObject *parent = nullptr);
    /** @brief 析构函数 */
    ~DashboardModel() override;

    /** @brief 添加组件配置 @param config 组件配置(QVariantMap) */
    void addComponentConfig(const QVariantMap &config);
    /** @brief 移除指定索引的组件配置 @param index 组件索引 */
    void removeComponentConfig(int index);
    /** @brief 获取所有组件配置 @return 配置列表 */
    QList<QVariantMap> componentConfigs() const;
    /** @brief 获取配置数量 @return 组件配置总数 */
    int configCount() const;
    /** @brief 将配置保存到文件 @param filePath 目标文件路径 @return true=保存成功 */
    bool saveToFile(const QString &filePath) const;
    /** @brief 从文件加载配置 @param filePath 源文件路径 @return true=加载成功 */
    bool loadFromFile(const QString &filePath);

    /** @brief 添加数据通道 @param name 通道名称 @param initialValue 初始值(默认0.0) */
    void addChannel(const QString &name, double initialValue = 0.0);
    /** @brief 移除数据通道 @param name 通道名称 */
    void removeChannel(const QString &name);
    /** @brief 更新通道值并通知视图 @param name 通道名称 @param value 新值 */
    void updateValue(const QString &name, double value);
    /** @brief 获取所有通道名称 @return 通道名称列表 */
    QStringList channelNames() const;
    /** @brief 获取通道当前值 @param name 通道名称 @return 当前值 */
    double value(const QString &name) const;
    /** @brief 获取通道最小值 @param name 通道名称 @return 最小值 */
    double channelMin(const QString &name) const;
    /** @brief 获取通道最大值 @param name 通道名称 @return 最大值 */
    double channelMax(const QString &name) const;
    /** @brief 获取通道值变更次数 @param name 通道名称 @return 变更计数 */
    quint64 channelChangeCount(const QString &name) const;
    /** @brief 获取总更新次数 @return 累计更新计数 */
    quint64 totalUpdateCount() const;
    /** @brief 重置所有通道统计 */
    void resetChannelStatistics();

    // ---- 组件类型统计 ----
    /** @brief 获取累计创建的组件总数 @return 创建计数 */
    quint64 totalWidgetsCreated() const;
    /** @brief 获取累计移除的组件总数 @return 移除计数 */
    quint64 totalWidgetsRemoved() const;
    /** @brief 获取当前活跃组件数量 @return 活跃计数 */
    quint64 activeWidgetCount() const;
    /** @brief 获取指定类型的组件创建总数 @param type 组件类型 @return 创建计数 */
    quint64 widgetsCreatedByType(const QString &type) const;

    // ---- 布局变更/配置文件/生命周期统计 ----
    /** @brief 获取累计布局变更次数 @return 变更计数 */
    quint64 layoutChanges() const;
    /** @brief 获取累计配置文件保存次数 @return 保存计数 */
    quint64 profileSaves() const;
    /** @brief 获取累计配置文件加载次数 @return 加载计数 */
    quint64 profileLoads() const;
    /** @brief 获取累计配置文件删除次数 @return 删除计数 */
    quint64 profileDeletes() const;
    /** @brief 获取累计布局变更总次数 @return 变更总数 */
    quint64 totalLayoutChanges() const;
    /** @brief 获取累计组件添加次数 @return 添加计数 */
    quint64 totalWidgetAdditions() const;
    /** @brief 获取累计组件移除次数 @return 移除计数 */
    quint64 totalWidgetRemovals() const;
    /** @brief 获取累计布局保存次数 @return 保存计数 */
    quint64 totalLayoutSaves() const;
    /** @brief 获取累计布局加载次数 @return 加载计数 */
    quint64 totalLayoutLoads() const;
    /** @brief 获取累计序列化操作次数 @return 序列化计数 */
    quint64 totalSerializations() const;
    /** @brief 重置所有统计计数器 */
    void resetAllStatistics();
    /** @brief 重置所有统计计数器(别名，调用resetAllStatistics) */
    void resetStats();

signals:
    void configChanged();                              ///< 配置发生变更
    void channelsChanged();                            ///< 数据通道集合变化
    void valueChanged(const QString &name, double value); ///< 通道值更新

private:
    QList<QVariantMap> m_configs;
    QMap<QString, double> m_channels, m_channelMin, m_channelMax;
    QMap<QString, quint64> m_channelChangeCount;
    quint64 m_totalUpdateCount = 0;
    mutable quint64 m_totalWidgetsCreated = 0;
    quint64 m_totalWidgetsRemoved = 0;
    QMap<QString, quint64> m_widgetsCreatedByType;
    mutable quint64 m_layoutChanges = 0;
    mutable quint64 m_profileSaves = 0, m_profileLoads = 0, m_profileDeletes = 0;
    mutable quint64 m_totalLayoutChanges = 0, m_totalWidgetAdditions = 0, m_totalWidgetRemovals = 0;
    mutable quint64 m_totalLayoutSaves = 0, m_totalLayoutLoads = 0, m_totalSerializations = 0;
};

#endif // DASHBOARD_MODEL_H

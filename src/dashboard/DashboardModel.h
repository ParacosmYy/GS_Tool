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
    explicit DashboardModel(QObject *parent = nullptr);
    ~DashboardModel() override;

    void addComponentConfig(const QVariantMap &config);    ///< 添加组件配置
    void removeComponentConfig(int index);                 ///< 移除指定索引的组件配置
    QList<QVariantMap> componentConfigs() const;           ///< 获取所有组件配置
    int configCount() const;                               ///< 获取配置数量
    bool saveToFile(const QString &filePath) const;        ///< 将配置保存到文件
    bool loadFromFile(const QString &filePath);            ///< 从文件加载配置

    void addChannel(const QString &name, double initialValue = 0.0); ///< 添加数据通道
    void removeChannel(const QString &name);               ///< 移除数据通道
    void updateValue(const QString &name, double value);   ///< 更新通道值并通知视图
    QStringList channelNames() const;                      ///< 获取所有通道名称
    double value(const QString &name) const;               ///< 获取通道当前值
    double channelMin(const QString &name) const;          ///< 获取通道最小值
    double channelMax(const QString &name) const;          ///< 获取通道最大值
    quint64 channelChangeCount(const QString &name) const; ///< 获取通道值变更次数
    quint64 totalUpdateCount() const;                      ///< 获取总更新次数
    void resetChannelStatistics();                         ///< 重置所有通道统计

    // ---- 组件类型统计 ----
    quint64 totalWidgetsCreated() const;
    quint64 totalWidgetsRemoved() const;
    quint64 activeWidgetCount() const;
    quint64 widgetsCreatedByType(const QString &type) const;

    // ---- 布局变更/配置文件/生命周期统计 ----
    quint64 layoutChanges() const;
    quint64 profileSaves() const;
    quint64 profileLoads() const;
    quint64 profileDeletes() const;
    quint64 totalLayoutChanges() const;
    quint64 totalWidgetAdditions() const;
    quint64 totalWidgetRemovals() const;
    quint64 totalLayoutSaves() const;
    quint64 totalLayoutLoads() const;
    quint64 totalSerializations() const;
    void resetAllStatistics();
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

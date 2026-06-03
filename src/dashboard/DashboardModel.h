/**
 * @file DashboardModel.h
 * @brief 仪表盘配置模型
 *
 * 管理仪表盘组件的配置信息与数据通道，支持序列化到文件与从文件反序列化。
 * 数据通道提供实时值更新，配合波形/仪表组件使用。
 */

#ifndef DASHBOARD_MODEL_H
#define DASHBOARD_MODEL_H

#include <QMap>
#include <QObject>
#include <QStringList>
#include <QVariantMap>
#include <QList>

/**
 * @class DashboardModel
 * @brief 仪表盘配置模型 —— 负责组件配置的增删查、数据通道管理与持久化
 */
class DashboardModel : public QObject
{
    Q_OBJECT

public:
    /// 构造函数
    explicit DashboardModel(QObject *parent = nullptr);

    /// 析构函数
    ~DashboardModel() override;

    /**
     * @brief 添加组件配置
     * @param config 组件配置 QVariantMap
     */
    void addComponentConfig(const QVariantMap &config);

    /**
     * @brief 移除指定索引的组件配置
     * @param index 配置索引
     */
    void removeComponentConfig(int index);

    /**
     * @brief 获取所有组件配置
     * @return 配置列表
     */
    QList<QVariantMap> componentConfigs() const;

    /**
     * @brief 获取配置数量
     * @return 数量
     */
    int configCount() const;

    /**
     * @brief 将配置保存到文件
     * @param filePath 目标文件路径
     * @return true=成功
     */
    bool saveToFile(const QString &filePath) const;

    /**
     * @brief 从文件加载配置
     * @param filePath 源文件路径
     * @return true=成功
     */
    bool loadFromFile(const QString &filePath);

    /**
     * @brief 添加数据通道
     * @param name 通道名称
     * @param initialValue 初始值
     */
    void addChannel(const QString &name, double initialValue = 0.0);

    /**
     * @brief 移除数据通道
     * @param name 通道名称
     */
    void removeChannel(const QString &name);

    /**
     * @brief 更新通道值并通知视图
     * @param name 通道名称
     * @param value 新值
     */
    void updateValue(const QString &name, double value);

    /**
     * @brief 获取所有通道名称
     * @return 通道名称列表
     */
    QStringList channelNames() const;

    /**
     * @brief 获取指定通道的当前值
     * @param name 通道名称
     * @return 通道值，通道不存在时返回 0.0
     */
    double value(const QString &name) const;

signals:
    /// 配置发生变更时发射
    void configChanged();

    /// 数据通道集合发生变化时发射 (增/删通道)
    void channelsChanged();

    /// 指定通道的值发生更新
    void valueChanged(const QString &name, double value);

private:
    QList<QVariantMap> m_configs;            ///< 组件配置列表
    QMap<QString, double> m_channels;        ///< 数据通道 <名称, 当前值>
};

#endif // DASHBOARD_MODEL_H

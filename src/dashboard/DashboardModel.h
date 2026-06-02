/**
 * @file DashboardModel.h
 * @brief 仪表盘配置模型
 *
 * 管理仪表盘组件的配置信息，支持序列化到文件与从文件反序列化。
 */

#ifndef DASHBOARD_MODEL_H
#define DASHBOARD_MODEL_H

#include <QObject>
#include <QVariantMap>
#include <QList>

/**
 * @class DashboardModel
 * @brief 仪表盘配置模型 —— 负责组件配置的增删查与持久化
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

signals:
    /// 配置发生变更时发射
    void configChanged();

private:
    QList<QVariantMap> m_configs;  ///< 组件配置列表
};

#endif // DASHBOARD_MODEL_H

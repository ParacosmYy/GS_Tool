/**
 * @file DashboardModel.cpp
 * @brief 仪表盘配置模型实现
 */

#include "dashboard/DashboardModel.h"

/**
 * @brief 构造函数
 * @param parent 父对象
 */
DashboardModel::DashboardModel(QObject *parent)
    : QObject(parent)
{
}

/** @brief 析构函数 */
DashboardModel::~DashboardModel() = default;

/**
 * @brief 添加组件配置
 * @param config 组件配置
 */
void DashboardModel::addComponentConfig(const QVariantMap &config)
{
    m_configs.append(config);
}

/**
 * @brief 移除指定索引的组件配置
 * @param index 配置索引
 */
void DashboardModel::removeComponentConfig(int index)
{
    if (index >= 0 && index < m_configs.size()) {
        m_configs.removeAt(index);
    }
}

/**
 * @brief 获取所有组件配置
 * @return 配置列表
 */
QList<QVariantMap> DashboardModel::componentConfigs() const
{
    return m_configs;
}

/**
 * @brief 将配置保存到文件（暂未实现）
 * @param filePath 目标文件路径
 * @return false（暂未实现）
 */
bool DashboardModel::saveToFile(const QString &filePath) const
{
    Q_UNUSED(filePath)
    // TODO: 序列化 m_configs 为 JSON 并写入文件
    return false;
}

/**
 * @brief 从文件加载配置（暂未实现）
 * @param filePath 源文件路径
 * @return false（暂未实现）
 */
bool DashboardModel::loadFromFile(const QString &filePath)
{
    Q_UNUSED(filePath)
    // TODO: 从文件读取 JSON 并解析到 m_configs
    return false;
}

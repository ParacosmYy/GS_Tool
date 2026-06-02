/**
 * @file DashboardModel.cpp
 * @brief 仪表盘配置模型实现
 *
 * 管理组件配置的增删查，使用 QJsonDocument 实现文件持久化。
 */

#include "dashboard/DashboardModel.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

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
    emit configChanged();
}

/**
 * @brief 移除指定索引的组件配置
 * @param index 配置索引
 */
void DashboardModel::removeComponentConfig(int index)
{
    if (index >= 0 && index < m_configs.size()) {
        m_configs.removeAt(index);
        emit configChanged();
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
 * @brief 获取配置数量
 * @return 数量
 */
int DashboardModel::configCount() const
{
    return m_configs.size();
}

/**
 * @brief 将配置保存到文件
 *
 * 序列化 m_configs 为 JSON 数组并写入文件。
 * @param filePath 目标文件路径
 * @return true=成功
 */
bool DashboardModel::saveToFile(const QString &filePath) const
{
    if (filePath.isEmpty()) {
        return false;
    }

    QJsonArray arr;
    for (const auto &config : m_configs) {
        QJsonObject obj = QJsonObject::fromVariantMap(config);
        arr.append(obj);
    }

    QJsonDocument doc(arr);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

/**
 * @brief 从文件加载配置
 *
 * 从 JSON 文件读取配置并填充到 m_configs。
 * @param filePath 源文件路径
 * @return true=成功
 */
bool DashboardModel::loadFromFile(const QString &filePath)
{
    if (filePath.isEmpty()) {
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError) {
        return false;
    }
    if (!doc.isArray()) {
        return false;
    }

    m_configs.clear();
    const QJsonArray arr = doc.array();
    for (const auto &item : arr) {
        if (item.isObject()) {
            m_configs.append(item.toObject().toVariantMap());
        }
    }

    emit configChanged();
    return true;
}

/**
 * @file DashboardModel.cpp
 * @brief 仪表盘配置模型实现
 *
 * 管理组件配置的增删查、数据通道的实时值更新，
 * 使用 QJsonDocument 实现文件持久化。
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

/**
 * @brief 添加数据通道
 *
 * 创建新的数据通道，若名称已存在则更新其初始值。
 * 通道增减后发射 channelsChanged 信号。
 *
 * @param name 通道名称
 * @param initialValue 初始值，默认 0.0
 */
void DashboardModel::addChannel(const QString &name, double initialValue)
{
    m_channels.insert(name, initialValue);
    emit channelsChanged();
}

/**
 * @brief 移除数据通道
 *
 * 删除指定名称的通道，通道不存在时无操作。
 * 通道增减后发射 channelsChanged 信号。
 *
 * @param name 通道名称
 */
void DashboardModel::removeChannel(const QString &name)
{
    if (m_channels.remove(name) > 0) {
        emit channelsChanged();
    }
}

/**
 * @brief 更新通道值并通知视图
 *
 * 更新指定通道的当前值，若值发生变化则发射 valueChanged 信号。
 * 通道不存在时不做任何操作。
 *
 * @param name 通道名称
 * @param value 新值
 */
void DashboardModel::updateValue(const QString &name, double value)
{
    auto it = m_channels.find(name);
    if (it == m_channels.end()) {
        return;
    }

    if (!qFuzzyCompare(it.value(), value)) {
        it.value() = value;
        emit valueChanged(name, value);
    }
}

/**
 * @brief 获取所有通道名称
 * @return 通道名称列表 (按插入顺序)
 */
QStringList DashboardModel::channelNames() const
{
    return m_channels.keys();
}

/**
 * @brief 获取指定通道的当前值
 * @param name 通道名称
 * @return 通道值，通道不存在时返回 0.0
 */
double DashboardModel::value(const QString &name) const
{
    return m_channels.value(name, 0.0);
}

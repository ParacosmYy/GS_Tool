/**
 * @file DeviceRegistry.cpp
 * @brief 设备配置注册表实现
 * @author Serial Tool Team
 * @date 2026-06-02
 */

#include "core/device/DeviceRegistry.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>

/**
 * @brief 构造函数
 */
DeviceRegistry::DeviceRegistry(QObject *parent)
    : QObject(parent)
{
}

/**
 * @brief 添加设备配置
 */
void DeviceRegistry::addProfile(const DeviceProfile &profile)
{
    m_profiles.append(profile);
    ++m_totalAdds;
    emit profileAdded(profile.name);
    emit profilesChanged();
}

/**
 * @brief 移除指定名称的设备配置
 */
void DeviceRegistry::removeProfile(const QString &name)
{
    for (int i = 0; i < m_profiles.size(); ++i) {
        if (m_profiles[i].name == name) {
            m_profiles.removeAt(i);
            ++m_totalRemoves;
            emit profileRemoved(name);
            emit profilesChanged();
            return;
        }
    }
}

/**
 * @brief 获取所有设备配置
 */
QList<DeviceProfile> DeviceRegistry::profiles() const
{
    return m_profiles;
}

/**
 * @brief 按名称查找设备配置
 */
DeviceProfile DeviceRegistry::findProfile(const QString &name) const
{
    for (const auto &profile : m_profiles) {
        if (profile.name == name) {
            return profile;
        }
    }
    return {};
}

/**
 * @brief 保存所有配置到 JSON 文件
 */
bool DeviceRegistry::saveToFile(const QString &filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    QJsonArray arr;
    for (const auto &profile : m_profiles) {
        arr.append(DeviceProfile::toJson(profile));
    }

    QJsonDocument doc(arr);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    ++m_totalSaves;
    return true;
}

/**
 * @brief 从 JSON 文件加载配置
 */
bool DeviceRegistry::loadFromFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isArray()) {
        return false;
    }

    m_profiles.clear();
    QJsonArray arr = doc.array();
    for (const auto &item : arr) {
        m_profiles.append(DeviceProfile::fromJson(item.toObject()));
    }

    emit profilesChanged();
    ++m_totalLoads;
    return true;
}

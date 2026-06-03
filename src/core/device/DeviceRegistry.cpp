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

/** @brief 构造设备注册表 @param parent 父对象指针 */
DeviceRegistry::DeviceRegistry(QObject *parent)
    : QObject(parent)
{
}

/**
 * @brief 添加一个设备配置到注册表
 * @param profile 要添加的设备配置
 */
void DeviceRegistry::addProfile(const DeviceProfile &profile)
{
    m_profiles.append(profile);
    ++m_totalAdds;
    emit profileAdded(profile.name);
    emit profilesChanged();
}

/**
 * @brief 按名称移除设备配置
 * @param name 要移除的配置名称
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

/** @brief 获取所有已注册的设备配置列表 @return 设备配置列表 */
QList<DeviceProfile> DeviceRegistry::profiles() const
{
    return m_profiles;
}

/**
 * @brief 按名称查找设备配置
 * @param name 要查找的配置名称
 * @return 匹配的设备配置，未找到时返回空配置
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
 * @brief 将所有设备配置序列化保存到JSON文件
 * @param filePath 目标文件路径
 * @return 保存成功返回true，文件打开失败返回false
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
 * @brief 从JSON文件加载设备配置并替换当前列表
 * @param filePath 源文件路径
 * @return 加载成功返回true，文件打开失败或格式错误返回false
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

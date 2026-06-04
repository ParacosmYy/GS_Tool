/**
 * @file DashboardSerializerProfile.cpp
 * @brief 仪表盘布局QSettings命名配置文件查询与辅助方法
 *
 * 从DashboardSerializer.cpp拆分而来，负责QSettings命名配置文件的查询操作和内部辅助方法:
 *   - listProfiles(): 列出所有已保存的配置文件名称
 *   - setCurrentProfile()/currentProfile(): 当前活跃配置文件管理
 *   - hasProfile(): 检查配置文件是否存在
 *   - ensureProfileListContains()/removeProfileListEntry(): 内部辅助方法
 *
 * CRUD操作(保存/加载/删除/重命名)见DashboardSerializerProfileCrud.cpp。
 */

#include "dashboard/DashboardSerializer.h"

#include <QSettings>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(lcDashboardProfile, "dashboard.profile")

/// QSettings配置文件系统常量键
static const QString kSettingsGroup  = QStringLiteral("DashboardProfiles");
static const QString kProfilesKey    = QStringLiteral("profiles");
static const QString kCurrentKey     = QStringLiteral("current");

/** @brief 列出所有已保存的配置文件名称 @return 配置文件名称列表 */
QStringList DashboardSerializer::listProfiles() const
{
    QSettings settings;
    settings.beginGroup(kSettingsGroup);
    const QStringList names = settings.value(kProfilesKey).toStringList();
    settings.endGroup();
    return names;
}

/** @brief 设置当前活跃配置文件 @param profileName 配置文件名称 */
void DashboardSerializer::setCurrentProfile(const QString& profileName)
{
    QSettings settings;
    settings.beginGroup(kSettingsGroup);
    settings.setValue(kCurrentKey, profileName);
    settings.endGroup();
    emit currentProfileChanged(profileName);
}

/** @brief 获取当前活跃配置文件名称 @return 配置文件名称 */
QString DashboardSerializer::currentProfile() const
{
    QSettings settings;
    settings.beginGroup(kSettingsGroup);
    const QString name = settings.value(kCurrentKey).toString();
    settings.endGroup();
    return name;
}

/** @brief 检查指定配置文件是否存在 @param profileName 配置文件名称 @return true=存在 */
bool DashboardSerializer::hasProfile(const QString& profileName) const
{
    QSettings settings;
    settings.beginGroup(kSettingsGroup);
    const bool exists = settings.childGroups().contains(profileName);
    settings.endGroup();
    return exists;
}

/** @brief 确保配置文件列表包含指定名称(内部辅助) @param profileName 配置文件名称 */
void DashboardSerializer::ensureProfileListContains(const QString& profileName)
{
    QSettings settings;
    settings.beginGroup(kSettingsGroup);
    QStringList names = settings.value(kProfilesKey).toStringList();
    if (!names.contains(profileName)) {
        names.append(profileName);
        settings.setValue(kProfilesKey, names);
    }
    settings.endGroup();
}

/** @brief 从配置文件列表中移除指定名称(内部辅助) @param profileName 配置文件名称 */
void DashboardSerializer::removeProfileListEntry(const QString& profileName)
{
    QSettings settings;
    settings.beginGroup(kSettingsGroup);
    QStringList names = settings.value(kProfilesKey).toStringList();
    names.removeAll(profileName);
    settings.setValue(kProfilesKey, names);
    settings.endGroup();
}

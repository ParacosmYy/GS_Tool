/**
 * @file DashboardSerializerProfile.cpp
 * @brief 仪表盘布局QSettings命名配置文件管理 — 保存/加载/删除/重命名/列表/统计
 *
 * 从DashboardSerializer.cpp拆分，负责QSettings命名配置文件的完整CRUD操作。
 * 配置文件系统常量键定义在此文件中，与主文件的JSON文件操作相互独立。
 */

#include "dashboard/DashboardSerializer.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QSettings>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(lcDashboardProfile, "dashboard.profile")

/// QSettings配置文件系统常量键
static const QString kSettingsGroup  = QStringLiteral("DashboardProfiles");
static const QString kProfilesKey    = QStringLiteral("profiles");
static const QString kCurrentKey     = QStringLiteral("currentProfile");
static const QString kColumnsKey     = QStringLiteral("columns");
static const QString kItemsKey       = QStringLiteral("items");
static const QString kNameKey        = QStringLiteral("name");

/** @brief 保存布局到QSettings命名配置文件 @return true=成功 */
bool DashboardSerializer::saveToProfile(const QString& profileName,
                                        const QString& name, int columns,
                                        const QList<DashboardItemConfig>& items)
{
    if (profileName.isEmpty()) {
        m_lastError = tr("配置文件名称不能为空");
        ++m_totalErrors; return false;
    }

    QSettings settings;
    settings.beginGroup(kSettingsGroup);

    /* 序列化面板配置为JSON数组 */
    QJsonArray itemsArray;
    for (const DashboardItemConfig& item : items)
        itemsArray.append(item.toJson());
    const QByteArray itemsJson = QJsonDocument(itemsArray).toJson(QJsonDocument::Compact);

    /* 写入配置文件数据 */
    settings.beginGroup(profileName);
    settings.setValue(kNameKey, name);
    settings.setValue(kColumnsKey, columns);
    settings.setValue(kItemsKey, QString::fromUtf8(itemsJson));
    settings.endGroup();
    settings.endGroup();

    ensureProfileListContains(profileName);

    qCInfo(lcDashboardProfile) << "配置文件已保存:" << profileName
                               << "面板数:" << items.size();
    ++m_totalSaves; ++m_totalProfileSaves;
    emit profileSaved(profileName);
    return true;
}

/** @brief 从QSettings命名配置文件加载布局 @return true=成功 */
bool DashboardSerializer::loadFromProfile(const QString& profileName,
                                          QString& name, int& columns,
                                          QList<DashboardItemConfig>& items)
{
    if (profileName.isEmpty()) {
        m_lastError = tr("配置文件名称不能为空");
        ++m_totalErrors; return false;
    }

    QSettings settings;
    settings.beginGroup(kSettingsGroup);

    if (!settings.childGroups().contains(profileName)) {
        m_lastError = tr("配置文件 '%1' 不存在").arg(profileName);
        settings.endGroup();
        ++m_totalErrors;
        return false;
    }

    settings.beginGroup(profileName);
    name    = settings.value(kNameKey, tr("未命名布局")).toString();
    columns = settings.value(kColumnsKey, 4).toInt();
    const QString itemsJson = settings.value(kItemsKey).toString();
    settings.endGroup();
    settings.endGroup();

    /* 解析JSON面板配置 */
    items.clear();
    if (!itemsJson.isEmpty()) {
        QJsonParseError parseError;
        const QJsonDocument doc = QJsonDocument::fromJson(itemsJson.toUtf8(), &parseError);
        if (doc.isNull()) {
            m_lastError = tr("配置文件 '%1' 数据损坏: %2")
                              .arg(profileName).arg(parseError.errorString());
            ++m_totalErrors;
            return false;
        }
        const QJsonArray arr = doc.array();
        items.reserve(arr.size());
        for (const QJsonValue& val : arr)
            items.append(DashboardItemConfig::fromJson(val.toObject()));
    }

    qCInfo(lcDashboardProfile) << "已加载配置文件:" << profileName
                               << "面板数:" << items.size();
    ++m_totalLoads; ++m_totalProfileLoads;
    emit profileLoaded(profileName, items.size());
    return true;
}

/** @brief 删除命名配置文件 @return true=成功 */
bool DashboardSerializer::deleteProfile(const QString& profileName)
{
    if (profileName.isEmpty()) {
        m_lastError = tr("配置文件名称不能为空");
        ++m_totalErrors;
        return false;
    }

    QSettings settings;
    settings.beginGroup(kSettingsGroup);

    if (!settings.childGroups().contains(profileName)) {
        m_lastError = tr("配置文件 '%1' 不存在").arg(profileName);
        settings.endGroup();
        ++m_totalErrors;
        return false;
    }

    settings.beginGroup(profileName);
    settings.remove(QString());
    settings.endGroup();
    settings.endGroup();

    removeProfileListEntry(profileName);
    if (currentProfile() == profileName)
        setCurrentProfile(QString());

    qCInfo(lcDashboardProfile) << "已删除配置文件:" << profileName;
    ++m_totalDeletes;
    emit profileDeleted(profileName);
    return true;
}

/** @brief 重命名配置文件 @return true=成功 */
bool DashboardSerializer::renameProfile(const QString& oldName, const QString& newName)
{
    if (oldName.isEmpty() || newName.isEmpty()) {
        m_lastError = tr("配置文件名称不能为空");
        ++m_totalErrors;
        return false;
    }

    QSettings settings;
    settings.beginGroup(kSettingsGroup);

    if (!settings.childGroups().contains(oldName)) {
        m_lastError = tr("配置文件 '%1' 不存在").arg(oldName);
        settings.endGroup();
        ++m_totalErrors;
        return false;
    }

    /* 读取旧配置 → 写入新配置 → 删除旧配置 */
    settings.beginGroup(oldName);
    const QString layoutName = settings.value(kNameKey).toString();
    const int columns = settings.value(kColumnsKey).toInt();
    const QString itemsJson = settings.value(kItemsKey).toString();
    settings.endGroup();

    settings.beginGroup(newName);
    settings.setValue(kNameKey, layoutName);
    settings.setValue(kColumnsKey, columns);
    settings.setValue(kItemsKey, itemsJson);
    settings.endGroup();

    settings.beginGroup(oldName);
    settings.remove(QString());
    settings.endGroup();
    settings.endGroup();

    removeProfileListEntry(oldName);
    ensureProfileListContains(newName);
    if (currentProfile() == oldName) setCurrentProfile(newName);

    qCInfo(lcDashboardProfile) << "已重命名:" << oldName << "->" << newName;
    return true;
}

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

/**
 * @file DashboardSerializerProfileCrud.cpp
 * @brief 仪表盘布局QSettings命名配置文件CRUD操作 - 保存/加载/删除/重命名
 *
 * 从DashboardSerializerProfile.cpp拆分而来，包含配置文件的写操作:
 *   - saveToProfile(): 保存布局到QSettings命名配置文件
 *   - loadFromProfile(): 从QSettings命名配置文件加载布局
 *   - deleteProfile(): 删除命名配置文件
 *   - renameProfile(): 重命名配置文件
 *
 * 查询与辅助方法保留在DashboardSerializerProfile.cpp中。
 */

#include "dashboard/DashboardSerializer.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QSettings>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(lcDashboardCrud, "dashboard.profile.crud")

/// QSettings配置文件系统常量键
static const QString kCrudSettingsGroup  = QStringLiteral("DashboardProfiles");
static const QString kCrudColumnsKey     = QStringLiteral("columns");
static const QString kCrudItemsKey       = QStringLiteral("items");
static const QString kCrudNameKey        = QStringLiteral("name");

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
    settings.beginGroup(kCrudSettingsGroup);

    /* 序列化面板配置为JSON数组 */
    QJsonArray itemsArray;
    for (const DashboardItemConfig& item : items)
        itemsArray.append(item.toJson());
    const QByteArray itemsJson = QJsonDocument(itemsArray).toJson(QJsonDocument::Compact);
    m_totalBytesWritten += static_cast<quint64>(itemsJson.size());

    /* 写入配置文件数据 */
    settings.beginGroup(profileName);
    settings.setValue(kCrudNameKey, name);
    settings.setValue(kCrudColumnsKey, columns);
    settings.setValue(kCrudItemsKey, QString::fromUtf8(itemsJson));
    settings.endGroup();
    settings.endGroup();

    ensureProfileListContains(profileName);

    qCInfo(lcDashboardCrud) << "配置文件已保存:" << profileName
                           << "面板数:" << items.size();
    ++m_totalSaves; ++m_totalProfileSaves; ++m_totalSerializations;
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
    settings.beginGroup(kCrudSettingsGroup);

    if (!settings.childGroups().contains(profileName)) {
        m_lastError = tr("配置文件 '%1' 不存在").arg(profileName);
        settings.endGroup();
        ++m_totalErrors;
        return false;
    }

    settings.beginGroup(profileName);
    name    = settings.value(kCrudNameKey, tr("未命名布局")).toString();
    columns = settings.value(kCrudColumnsKey, 4).toInt();
    const QString itemsJson = settings.value(kCrudItemsKey).toString();
    settings.endGroup();
    settings.endGroup();

    /* 解析JSON面板配置 */
    items.clear();
    if (!itemsJson.isEmpty()) {
        m_totalBytesRead += static_cast<quint64>(itemsJson.toUtf8().size());
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

    qCInfo(lcDashboardCrud) << "已加载配置文件:" << profileName
                           << "面板数:" << items.size();
    ++m_totalLoads; ++m_totalProfileLoads; ++m_totalDeserializations;
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
    settings.beginGroup(kCrudSettingsGroup);

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

    qCInfo(lcDashboardCrud) << "已删除配置文件:" << profileName;
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
    settings.beginGroup(kCrudSettingsGroup);

    if (!settings.childGroups().contains(oldName)) {
        m_lastError = tr("配置文件 '%1' 不存在").arg(oldName);
        settings.endGroup();
        ++m_totalErrors;
        return false;
    }

    /* 读取旧配置 -> 写入新配置 -> 删除旧配置 */
    settings.beginGroup(oldName);
    const QString layoutName = settings.value(kCrudNameKey).toString();
    const int columns = settings.value(kCrudColumnsKey).toInt();
    const QString itemsJson = settings.value(kCrudItemsKey).toString();
    settings.endGroup();

    settings.beginGroup(newName);
    settings.setValue(kCrudNameKey, layoutName);
    settings.setValue(kCrudColumnsKey, columns);
    settings.setValue(kCrudItemsKey, itemsJson);
    settings.endGroup();

    settings.beginGroup(oldName);
    settings.remove(QString());
    settings.endGroup();
    settings.endGroup();

    removeProfileListEntry(oldName);
    ensureProfileListContains(newName);
    if (currentProfile() == oldName) setCurrentProfile(newName);

    qCInfo(lcDashboardCrud) << "已重命名:" << oldName << "->" << newName;
    return true;
}

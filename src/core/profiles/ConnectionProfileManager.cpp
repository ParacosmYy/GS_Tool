/**
 * @file ConnectionProfileManager.cpp
 * @brief 连接配置管理器实现
 */

#include "core/profiles/ConnectionProfileManager.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDateTime>
#include <QUuid>
#include <QSettings>

/** @brief 构造函数 @param parent 父对象 */
ConnectionProfileManager::ConnectionProfileManager(QObject* parent)
    : QObject(parent)
{
}

/** @brief 创建新配置 @param name 名称 @param type 连接类型 @return 配置ID */
QString ConnectionProfileManager::createProfile(const QString& name, ConnectionType type)
{
    QString id = generateId();
    Profile p;
    p.name = name;
    p.type = type;
    p.lastUsed = 0;
    p.useCount = 0;
    p.isFavorite = false;
    m_profiles.insert(id, p);

    ++m_stats.totalCreations;
    ++m_stats.totalProfiles;
    m_stats.peakProfiles = qMax(m_stats.peakProfiles, m_profiles.size());

    emit profileCreated(id);
    emit profilesChanged();
    return id;
}

/** @brief 删除配置 @param id 配置ID @return 是否成功 */
bool ConnectionProfileManager::deleteProfile(const QString& id)
{
    if (!m_profiles.contains(id)) return false;
    m_profiles.remove(id);
    m_recentIds.removeAll(id);

    ++m_stats.totalDeletions;
    --m_stats.totalProfiles;
    if (m_activeProfileId == id) {
        m_activeProfileId.clear();
    }

    emit profileDeleted(id);
    emit profilesChanged();
    return true;
}

/** @brief 更新配置 @param id 配置ID @param profile 新数据 @return 是否成功 */
bool ConnectionProfileManager::updateProfile(const QString& id, const Profile& profile)
{
    if (!m_profiles.contains(id)) return false;
    m_profiles[id] = profile;
    emit profileUpdated(id);
    emit profilesChanged();
    return true;
}

/** @brief 获取配置 @param id 配置ID @return 配置数据 */
ConnectionProfileManager::Profile ConnectionProfileManager::profile(const QString& id) const
{
    return m_profiles.value(id);
}

/** @brief 获取所有配置 @return 配置映射 */
QMap<QString, ConnectionProfileManager::Profile> ConnectionProfileManager::allProfiles() const
{
    return m_profiles;
}

/** @brief 获取最近使用的配置 @param count 数量 @return 配置列表 */
QList<ConnectionProfileManager::Profile> ConnectionProfileManager::recentProfiles(int count) const
{
    QList<Profile> result;
    for (int i = 0; i < qMin(m_recentIds.size(), count); ++i) {
        const QString& id = m_recentIds.at(i);
        if (m_profiles.contains(id)) {
            result.append(m_profiles.value(id));
        }
    }
    return result;
}

/** @brief 获取收藏的配置 @return 收藏列表 */
QList<ConnectionProfileManager::Profile> ConnectionProfileManager::favoriteProfiles() const
{
    QList<Profile> result;
    for (const auto& p : m_profiles) {
        if (p.isFavorite) result.append(p);
    }
    return result;
}

/** @brief 激活配置(标记为当前使用) @param id 配置ID */
void ConnectionProfileManager::activateProfile(const QString& id)
{
    if (!m_profiles.contains(id)) return;

    m_activeProfileId = id;
    m_profiles[id].lastUsed = QDateTime::currentMSecsSinceEpoch();
    ++m_profiles[id].useCount;

    updateRecentList(id);
    ++m_stats.totalSwitches;

    emit profileActivated(id);
}

/** @brief 克隆配置 @param sourceId 源配置ID @param newName 新名称 @return 新配置ID */
QString ConnectionProfileManager::cloneProfile(const QString& sourceId, const QString& newName)
{
    if (!m_profiles.contains(sourceId)) return {};

    Profile cloned = m_profiles.value(sourceId);
    QString newId = generateId();
    cloned.name = newName;
    cloned.lastUsed = 0;
    cloned.useCount = 0;
    m_profiles.insert(newId, cloned);

    ++m_stats.totalCreations;
    ++m_stats.totalProfiles;
    m_stats.peakProfiles = qMax(m_stats.peakProfiles, m_profiles.size());

    emit profileCreated(newId);
    emit profilesChanged();
    return newId;
}

/** @brief 导出配置到JSON @param id 配置ID @param filePath 目标路径 @return 是否成功 */
bool ConnectionProfileManager::exportProfile(const QString& id, const QString& filePath)
{
    if (!m_profiles.contains(id)) return false;

    const Profile& p = m_profiles.value(id);
    QJsonObject obj;
    obj[QStringLiteral("name")] = p.name;
    obj[QStringLiteral("type")] = static_cast<int>(p.type);
    obj[QStringLiteral("isFavorite")] = p.isFavorite;

    QJsonObject settingsObj;
    for (auto it = p.settings.constBegin(); it != p.settings.constEnd(); ++it) {
        settingsObj[it.key()] = QJsonValue::fromVariant(it.value());
    }
    obj[QStringLiteral("settings")] = settingsObj;

    QJsonDocument doc(obj);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) return false;
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    ++m_stats.totalExports;
    return true;
}

/** @brief 从JSON导入配置 @param filePath JSON文件路径 @return 新配置ID */
QString ConnectionProfileManager::importProfile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) return {};

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    if (!doc.isObject()) return {};

    QJsonObject obj = doc.object();
    Profile p;
    p.name = obj[QStringLiteral("name")].toString();
    p.type = static_cast<ConnectionType>(obj[QStringLiteral("type")].toInt());
    p.isFavorite = obj[QStringLiteral("isFavorite")].toBool();
    p.lastUsed = 0;
    p.useCount = 0;

    QJsonObject settingsObj = obj[QStringLiteral("settings")].toObject();
    for (auto it = settingsObj.constBegin(); it != settingsObj.constEnd(); ++it) {
        p.settings.insert(it.key(), it.value().toVariant());
    }

    QString id = generateId();
    m_profiles.insert(id, p);

    ++m_stats.totalImports;
    ++m_stats.totalProfiles;
    m_stats.peakProfiles = qMax(m_stats.peakProfiles, m_profiles.size());

    emit profileCreated(id);
    emit profilesChanged();
    return id;
}

/** @brief 保存所有配置到磁盘(QSettings) */
void ConnectionProfileManager::save()
{
    QSettings s;
    s.beginGroup(QStringLiteral("ConnectionProfiles"));

    s.beginWriteArray(QStringLiteral("profiles"), m_profiles.size());
    int idx = 0;
    for (auto it = m_profiles.constBegin(); it != m_profiles.constEnd(); ++it) {
        s.setArrayIndex(idx++);
        s.setValue(QStringLiteral("id"), it.key());
        s.setValue(QStringLiteral("name"), it.value().name);
        s.setValue(QStringLiteral("type"), static_cast<int>(it.value().type));
        s.setValue(QStringLiteral("lastUsed"), it.value().lastUsed);
        s.setValue(QStringLiteral("useCount"), it.value().useCount);
        s.setValue(QStringLiteral("isFavorite"), it.value().isFavorite);
        s.setValue(QStringLiteral("settings"), it.value().settings);
    }
    s.endArray();

    s.setValue(QStringLiteral("recent"), m_recentIds);
    s.setValue(QStringLiteral("active"), m_activeProfileId);
    s.endGroup();
}

/** @brief 从磁盘加载配置 */
void ConnectionProfileManager::load()
{
    QSettings s;
    s.beginGroup(QStringLiteral("ConnectionProfiles"));

    int size = s.beginReadArray(QStringLiteral("profiles"));
    for (int i = 0; i < size; ++i) {
        s.setArrayIndex(i);
        Profile p;
        QString id = s.value(QStringLiteral("id")).toString();
        p.name = s.value(QStringLiteral("name")).toString();
        p.type = static_cast<ConnectionType>(s.value(QStringLiteral("type")).toInt());
        p.lastUsed = s.value(QStringLiteral("lastUsed")).toLongLong();
        p.useCount = s.value(QStringLiteral("useCount")).toInt();
        p.isFavorite = s.value(QStringLiteral("isFavorite")).toBool();
        p.settings = s.value(QStringLiteral("settings")).toMap();
        m_profiles.insert(id, p);
    }
    s.endArray();

    m_recentIds = s.value(QStringLiteral("recent")).toStringList();
    m_activeProfileId = s.value(QStringLiteral("active")).toString();
    s.endGroup();

    m_stats.totalProfiles = static_cast<quint64>(m_profiles.size());
    m_stats.peakProfiles = m_profiles.size();
}

/** @brief 生成唯一ID @return UUID字符串 */
QString ConnectionProfileManager::generateId() const
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

/** @brief 更新最近使用列表 @param id 配置ID */
void ConnectionProfileManager::updateRecentList(const QString& id)
{
    m_recentIds.removeAll(id);
    m_recentIds.prepend(id);
    while (m_recentIds.size() > 20) {
        m_recentIds.removeLast();
    }
}

/** @brief 重置所有统计计数器 */
void ConnectionProfileManager::resetStatistics()
{
    m_stats = Stats{};
    m_stats.totalProfiles = static_cast<quint64>(m_profiles.size());
    m_stats.peakProfiles = m_profiles.size();
}

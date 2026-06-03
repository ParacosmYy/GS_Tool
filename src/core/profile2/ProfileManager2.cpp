#include "core/profile2/ProfileManager2.h"
ProfileManager::ProfileManager(QObject *parent) : QObject(parent) {}
ProfileManager::~ProfileManager() = default;
QString ProfileManager::createProfile(const QString &name, const QString &type) {
    QString id = QString("prof_%1").arg(++m_counter);
    DeviceProfile p; p.id = id; p.name = name; p.type = type;
    m_profiles[id] = p; emit profileCreated(id); return id;
}
void ProfileManager::deleteProfile(const QString &id) { m_profiles.remove(id); if (m_defaultId == id) m_defaultId.clear(); emit profileDeleted(id); }
void ProfileManager::updateProfile(const QString &id, const QVariantMap &s) { auto it = m_profiles.find(id); if (it != m_profiles.end()) { it->settings = s; emit profileUpdated(id); } }
void ProfileManager::renameProfile(const QString &id, const QString &n) { auto it = m_profiles.find(id); if (it != m_profiles.end()) it->name = n; }
ProfileManager::DeviceProfile ProfileManager::profile(const QString &id) const { return m_profiles.value(id); }
QList<ProfileManager::DeviceProfile> ProfileManager::profiles() const { return m_profiles.values(); }
QList<ProfileManager::DeviceProfile> ProfileManager::profilesByType(const QString &t) const { QList<DeviceProfile> r; for (const auto &p : m_profiles) if (p.type == t) r.append(p); return r; }
void ProfileManager::setDefault(const QString &id) { m_defaultId = id; for (auto &p : m_profiles) p.isDefault = (p.id == id); emit defaultChanged(id); }
ProfileManager::DeviceProfile ProfileManager::defaultProfile() const { return m_profiles.value(m_defaultId); }
int ProfileManager::count() const { return m_profiles.size(); }
void ProfileManager::importProfile(const QVariantMap &d) { DeviceProfile p; p.name = d["name"].toString(); p.type = d["type"].toString(); p.settings = d["settings"].toMap(); createProfile(p.name, p.type); }
QVariantMap ProfileManager::exportProfile(const QString &id) const { auto p = m_profiles.value(id); QVariantMap m; m["name"]=p.name; m["type"]=p.type; m["settings"]=p.settings; return m; }

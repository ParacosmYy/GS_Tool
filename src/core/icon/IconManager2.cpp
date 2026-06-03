#include "core/icon/IconManager2.h"
#include <QFile>
IconManager::IconManager(QObject *parent) : QObject(parent) {}
IconManager::~IconManager() = default;
void IconManager::registerIcon(const QString &name, const QString &path) { m_lightPaths[name] = path; m_darkPaths[name] = path; m_cache.remove(name); emit iconRegistered(name); }
void IconManager::registerThemedIcon(const QString &name, const QString &light, const QString &dark) { m_lightPaths[name] = light; m_darkPaths[name] = dark; m_cache.remove(name); emit iconRegistered(name); }
QIcon IconManager::icon(const QString &name) const {
    if (m_cache.contains(name)) return m_cache[name];
    QString path = m_darkTheme ? m_darkPaths.value(name) : m_lightPaths.value(name);
    if (path.isEmpty()) return QIcon();
    QIcon ic(path);
    const_cast<IconManager*>(this)->m_cache[name] = ic;
    return ic;
}
QPixmap IconManager::pixmap(const QString &name, const QSize &sz) const { return icon(name).pixmap(sz); }
bool IconManager::hasIcon(const QString &name) const { return m_lightPaths.contains(name); }
QStringList IconManager::availableIcons() const { return m_lightPaths.keys(); }
void IconManager::setTheme(bool dark) { m_darkTheme = dark; m_cache.clear(); emit themeChanged(dark); }
void IconManager::preloadAll(const QSize &sz) { for (auto it = m_lightPaths.constBegin(); it != m_lightPaths.constEnd(); ++it) icon(it.key()); }
void IconManager::clearCache() { m_cache.clear(); }

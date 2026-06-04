/**
 * @file IconManager2.cpp
 * @brief 图标管理器v2实现 — SVG图标注册、主题切换、缓存管理
 */
#include "core/icon/IconManager2.h"
#include <QFile>

/** @brief 构造函数 @param parent 父对象 */
IconManager::IconManager(QObject *parent) : QObject(parent) {}
/** @brief 析构函数 */
IconManager::~IconManager() = default;

/** @brief 注册图标(亮暗主题共用同一路径) @param name 图标名称 @param path SVG文件路径 */
void IconManager::registerIcon(const QString &name, const QString &path) { m_lightPaths[name] = path; m_darkPaths[name] = path; m_cache.remove(name); ++m_totalIconRegistrations; emit iconRegistered(name); }
/** @brief 注册带主题区分的图标 @param name 图标名称 @param light 亮色主题路径 @param dark 暗色主题路径 */
void IconManager::registerThemedIcon(const QString &name, const QString &light, const QString &dark) { m_lightPaths[name] = light; m_darkPaths[name] = dark; m_cache.remove(name); ++m_totalIconRegistrations; emit iconRegistered(name); }

/** @brief 获取图标(带缓存，按当前主题选择路径) @param name 图标名称 @return QIcon对象 */
QIcon IconManager::icon(const QString &name) const {
    ++m_totalIconLookups;
    if (m_cache.contains(name)) { ++m_totalCacheHits; return m_cache[name]; }
    QString path = m_darkTheme ? m_darkPaths.value(name) : m_lightPaths.value(name);
    if (path.isEmpty()) return QIcon();
    QIcon ic(path);
    const_cast<IconManager*>(this)->m_cache[name] = ic;
    return ic;
}

/** @brief 获取指定尺寸的图标像素图 @param name 图标名称 @param sz 目标尺寸 @return QPixmap对象 */
QPixmap IconManager::pixmap(const QString &name, const QSize &sz) const { return icon(name).pixmap(sz); }
/** @brief 检查图标是否已注册 @param name 图标名称 @return 已注册返回true */
bool IconManager::hasIcon(const QString &name) const { return m_lightPaths.contains(name); }
/** @brief 获取所有已注册图标名称 @return 图标名称列表 */
QStringList IconManager::availableIcons() const { return m_lightPaths.keys(); }
/** @brief 切换图标主题(亮/暗)并清空缓存 @param dark 是否为暗色主题 */
void IconManager::setTheme(bool dark) { m_darkTheme = dark; m_cache.clear(); ++m_totalThemeSwitches; emit themeChanged(dark); }
/** @brief 预加载所有已注册图标到缓存 */
void IconManager::preloadAll(const QSize &sz) { for (auto it = m_lightPaths.constBegin(); it != m_lightPaths.constEnd(); ++it) icon(it.key()); }
/** @brief 清空图标缓存 */
void IconManager::clearCache() { m_cache.clear(); }

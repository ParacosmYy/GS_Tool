/**
 * @file FontManager.cpp
 * @brief 字体管理器实现 — 应用字体加载、预设字体管理、默认字体初始化
 */
#include "core/font/FontManager.h"

/** @brief 构造函数，初始化默认预设字体 @param parent 父对象 */
FontManager::FontManager(QObject *parent) : QObject(parent) { initDefaults(); }
/** @brief 析构函数 */
FontManager::~FontManager() = default;

/** @brief 从文件加载字体到应用字体库 @param name 字体名称标识 @param path 字体文件路径 @return 加载成功返回true */
bool FontManager::loadFont(const QString &name, const QString &path) {
    int id = QFontDatabase::addApplicationFont(path);
    if (id < 0) return false;
    m_loadedIds[name] = id;
    emit fontLoaded(name);
    return true;
}

/** @brief 获取预设字体 @param p 预设字体枚举 @return QFont对象 */
QFont FontManager::font(PresetFont p) const { return m_presets.value(p); }
/** @brief 按名称和大小获取字体 @param name 字体族名 @param sz 字号(<=0使用默认) @return QFont对象 */
QFont FontManager::font(const QString &name, int sz) const {
    QFont f(name); if (sz > 0) f.setPointSize(sz); return f;
}

/** @brief 设置预设字体 @param p 预设字体枚举 @param f 字体对象 */
void FontManager::setFont(PresetFont p, const QFont &f) { m_presets[p] = f; emit fontChanged(p); }
/** @brief 设置预设字体大小 @param p 预设字体枚举 @param sz 字号 */
void FontManager::setFontSize(PresetFont p, int sz) { auto f = m_presets.value(p); f.setPointSize(sz); m_presets[p] = f; emit fontChanged(p); }
/** @brief 设置默认字体族名 @param f 字体族名 */
void FontManager::setDefaultFamily(const QString &f) { m_defaultFamily = f; }
/** @brief 获取默认字体族名 @return 字体族名 */
QString FontManager::defaultFamily() const { return m_defaultFamily; }
/** @brief 获取所有已加载字体名称 @return 字体名称列表 */
QStringList FontManager::loadedFonts() const { return m_loadedIds.keys(); }
/** @brief 获取系统可用字体族列表 @return 字体族名称列表 */
QStringList FontManager::availableFamilies() const { return QFontDatabase::families(); }

/** @brief 重置所有预设字体为默认值并通知变更 */
void FontManager::resetToDefaults() {
    initDefaults();
    for (int i = 0; i <= Monospace; ++i) emit fontChanged(static_cast<PresetFont>(i));
}

/** @brief 初始化预设字体 — Segoe UI系列+Consolas等宽字体 */
void FontManager::initDefaults() {
    m_defaultFamily = "Segoe UI";
    m_presets[Title] = QFont(m_defaultFamily, 18, QFont::Bold);
    m_presets[Heading] = QFont(m_defaultFamily, 14, QFont::DemiBold);
    m_presets[Body] = QFont(m_defaultFamily, 10);
    m_presets[Caption] = QFont(m_defaultFamily, 8);
    m_presets[Code] = QFont("Consolas", 10);
    m_presets[Monospace] = QFont("Consolas", 9);
}

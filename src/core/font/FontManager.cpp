#include "core/font/FontManager.h"

FontManager::FontManager(QObject *parent) : QObject(parent) { initDefaults(); }
FontManager::~FontManager() = default;

bool FontManager::loadFont(const QString &name, const QString &path) {
    int id = QFontDatabase::addApplicationFont(path);
    if (id < 0) return false;
    m_loadedIds[name] = id;
    emit fontLoaded(name);
    return true;
}

QFont FontManager::font(PresetFont p) const { return m_presets.value(p); }
QFont FontManager::font(const QString &name, int sz) const {
    QFont f(name); if (sz > 0) f.setPointSize(sz); return f;
}

void FontManager::setFont(PresetFont p, const QFont &f) { m_presets[p] = f; emit fontChanged(p); }
void FontManager::setFontSize(PresetFont p, int sz) { auto f = m_presets.value(p); f.setPointSize(sz); m_presets[p] = f; emit fontChanged(p); }
void FontManager::setDefaultFamily(const QString &f) { m_defaultFamily = f; }
QString FontManager::defaultFamily() const { return m_defaultFamily; }
QStringList FontManager::loadedFonts() const { return m_loadedIds.keys(); }
QStringList FontManager::availableFamilies() const { return QFontDatabase::families(); }

void FontManager::resetToDefaults() {
    initDefaults();
    for (int i = 0; i <= Monospace; ++i) emit fontChanged(static_cast<PresetFont>(i));
}

void FontManager::initDefaults() {
    m_defaultFamily = "Segoe UI";
    m_presets[Title] = QFont(m_defaultFamily, 18, QFont::Bold);
    m_presets[Heading] = QFont(m_defaultFamily, 14, QFont::DemiBold);
    m_presets[Body] = QFont(m_defaultFamily, 10);
    m_presets[Caption] = QFont(m_defaultFamily, 8);
    m_presets[Code] = QFont("Consolas", 10);
    m_presets[Monospace] = QFont("Consolas", 9);
}

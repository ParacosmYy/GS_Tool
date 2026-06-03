/**
 * @file FontManager.cpp
 * @brief 字体管理器实现
 * @since score-131
 */
#include "core/fonts/FontManager.h"
#include <QApplication>
#include <QFile>
#include <QFontDatabase>
#include <QFontInfo>
#include <QScreen>

FontManager::FontManager(QObject *parent) : QObject(parent) {}
FontManager::~FontManager() = default;
FontManager &FontManager::instance() { static FontManager inst; return inst; }

void FontManager::initialize() {
    m_roles[FontRole::Default] = {QApplication::font().family(), 10, {QStringLiteral("Segoe UI"), QStringLiteral("Arial"), QStringLiteral("Sans-serif")}};
    m_roles[FontRole::Monospace] = {QStringLiteral("Consolas"), 10, {QStringLiteral("JetBrains Mono"), QStringLiteral("Cascadia Code"), QStringLiteral("Source Code Pro"), QStringLiteral("Courier New")}};
    m_roles[FontRole::Title] = {QApplication::font().family(), 16, {QStringLiteral("Segoe UI"), QStringLiteral("Arial")}};
    m_roles[FontRole::Subtitle] = {QApplication::font().family(), 13, {QStringLiteral("Segoe UI"), QStringLiteral("Arial")}};
    m_roles[FontRole::Caption] = {QApplication::font().family(), 8, {QStringLiteral("Segoe UI"), QStringLiteral("Arial")}};
    m_roles[FontRole::Code] = {QStringLiteral("Consolas"), 10, {QStringLiteral("JetBrains Mono"), QStringLiteral("Cascadia Code"), QStringLiteral("Source Code Pro")}};
    m_roles[FontRole::DataDisplay] = {QStringLiteral("Consolas"), 12, {QStringLiteral("JetBrains Mono"), QStringLiteral("Source Code Pro")}};
    m_roles[FontRole::Toolbar] = {QApplication::font().family(), 9, {QStringLiteral("Segoe UI"), QStringLiteral("Arial")}};
    m_roles[FontRole::Status] = {QApplication::font().family(), 9, {QStringLiteral("Segoe UI"), QStringLiteral("Arial")}};
    m_roles[FontRole::UserInput] = {QApplication::font().family(), 10, {QStringLiteral("Segoe UI"), QStringLiteral("Arial")}};
    if (QScreen *screen = qApp->primaryScreen()) {
        double dpi = screen->logicalDotsPerInch();
        m_scaleFactor = dpi / 96.0;
        if (m_scaleFactor < 0.5) m_scaleFactor = 1.0;
    }
}

QFont FontManager::font(FontRole role, int pointSize) const {
    ++m_totalFontRequests;
    auto it = m_roles.find(role);
    if (it == m_roles.end()) it = m_roles.find(FontRole::Default);
    if (it == m_roles.end()) return QFont();
    const RoleConfig &cfg = it.value();
    int size = pointSize > 0 ? pointSize : cfg.pointSize;
    QString family = resolveFamily(role);
    QFont f(family);
    f.setPointSize(scaledSize(size));
    if (role == FontRole::Monospace || role == FontRole::Code || role == FontRole::DataDisplay) {
        f.setStyleHint(QFont::Monospace); f.setFixedPitch(true);
    }
    return f;
}
QFont FontManager::monospaceFont(int pointSize) const { return font(FontRole::Monospace, pointSize); }
QFont FontManager::codeFont(int pointSize) const { return font(FontRole::Code, pointSize); }

void FontManager::setRoleFamily(FontRole role, const QString &family) {
    if (m_roles.contains(role)) m_roles[role].family = family;
    else { RoleConfig cfg; cfg.family = family; cfg.pointSize = 10; m_roles[role] = cfg; }
    emit fontRoleChanged(role);
}
QString FontManager::roleFamily(FontRole role) const { auto it = m_roles.find(role); return it != m_roles.end() ? it->family : QString(); }
void FontManager::setRoleSize(FontRole role, int pointSize) { if (m_roles.contains(role)) m_roles[role].pointSize = pointSize; emit fontRoleChanged(role); }
int FontManager::roleSize(FontRole role) const { auto it = m_roles.find(role); return it != m_roles.end() ? it->pointSize : 10; }

QString FontManager::loadCustomFont(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) return QString();
    QByteArray data = file.readAll();
    int id = QFontDatabase::addApplicationFontFromData(data);
    if (id < 0) return QString();
    QStringList families = QFontDatabase::applicationFontFamilies(id);
    if (families.isEmpty()) return QString();
    QString family = families.first();
    m_loadedFamilies.append(family);
    ++m_totalCustomFontsLoaded;
    emit customFontLoaded(family);
    return family;
}
QStringList FontManager::loadCustomFonts(const QStringList &filePaths) {
    QStringList result;
    for (const auto &path : filePaths) { QString f = loadCustomFont(path); if (!f.isEmpty()) result.append(f); }
    return result;
}
QStringList FontManager::loadedFontFamilies() const { return m_loadedFamilies; }

void FontManager::setScaleFactor(double factor) { if (qAbs(m_scaleFactor - factor) > 0.01) { m_scaleFactor = factor; emit scaleFactorChanged(factor); } }
double FontManager::scaleFactor() const { return m_scaleFactor; }
int FontManager::scaledSize(int basePointSize) const { ++m_totalScaleOps; return qMax(6, qRound(basePointSize * m_scaleFactor)); }
void FontManager::setFallbackChain(FontRole role, const QStringList &families) { if (m_roles.contains(role)) m_roles[role].fallbackFamilies = families; }
QStringList FontManager::fallbackChain(FontRole role) const { auto it = m_roles.find(role); return it != m_roles.end() ? it->fallbackFamilies : QStringList(); }

quint64 FontManager::totalFontRequests() const { return m_totalFontRequests; }
quint64 FontManager::totalCustomFontsLoaded() const { return m_totalCustomFontsLoaded; }
quint64 FontManager::totalScaleOps() const { return m_totalScaleOps; }
void FontManager::resetStatistics() { m_totalFontRequests = 0; m_totalCustomFontsLoaded = 0; m_totalScaleOps = 0; }

QString FontManager::resolveFamily(FontRole role) const {
    auto it = m_roles.find(role);
    if (it == m_roles.end()) return QApplication::font().family();
    const RoleConfig &cfg = it.value();
    QFont testFont(cfg.family);
    if (QFontInfo(testFont).family().compare(cfg.family, Qt::CaseInsensitive) == 0) return cfg.family;
    for (const auto &fb : cfg.fallbackFamilies) {
        if (QFontInfo(QFont(fb)).family().compare(fb, Qt::CaseInsensitive) == 0) return fb;
    }
    return QApplication::font().family();
}

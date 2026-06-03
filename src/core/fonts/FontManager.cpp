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

/** @brief 构造函数 @param parent 父对象 */
FontManager::FontManager(QObject *parent) : QObject(parent) {}
/** @brief 析构函数 */
FontManager::~FontManager() = default;
/** @brief 获取单例实例 @return 字体管理器引用 */
FontManager &FontManager::instance() { static FontManager inst; return inst; }

/** @brief 初始化字体角色配置，设置默认/等宽/标题等字体 */
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

/** @brief 获取指定角色的字体 @param role 字体角色 @param pointSize 字号(0使用默认) @return 配置好的QFont对象 */
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
/** @brief 获取等宽字体 @param pointSize 字号 @return 等宽QFont */
QFont FontManager::monospaceFont(int pointSize) const { return font(FontRole::Monospace, pointSize); }
/** @brief 获取代码字体 @param pointSize 字号 @return 代码QFont */
QFont FontManager::codeFont(int pointSize) const { return font(FontRole::Code, pointSize); }

/** @brief 设置角色字体族 @param role 字体角色 @param family 字体族名称 */
void FontManager::setRoleFamily(FontRole role, const QString &family) {
    if (m_roles.contains(role)) m_roles[role].family = family;
    else { RoleConfig cfg; cfg.family = family; cfg.pointSize = 10; m_roles[role] = cfg; }
    emit fontRoleChanged(role);
}
/** @brief 获取角色字体族名称 @param role 字体角色 @return 字体族名称 */
QString FontManager::roleFamily(FontRole role) const { auto it = m_roles.find(role); return it != m_roles.end() ? it->family : QString(); }
/** @brief 设置角色字号 @param role 字体角色 @param pointSize 字号 */
void FontManager::setRoleSize(FontRole role, int pointSize) { if (m_roles.contains(role)) m_roles[role].pointSize = pointSize; emit fontRoleChanged(role); }
/** @brief 获取角色字号 @param role 字体角色 @return 字号 */
int FontManager::roleSize(FontRole role) const { auto it = m_roles.find(role); return it != m_roles.end() ? it->pointSize : 10; }

/** @brief 加载自定义字体文件 @param filePath 字体文件路径 @return 加载的字体族名称 */
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
/** @brief 批量加载自定义字体 @param filePaths 字体文件路径列表 @return 加载的字体族名称列表 */
QStringList FontManager::loadCustomFonts(const QStringList &filePaths) {
    QStringList result;
    for (const auto &path : filePaths) { QString f = loadCustomFont(path); if (!f.isEmpty()) result.append(f); }
    return result;
}
/** @brief 获取已加载的自定义字体族列表 @return 字体族名称列表 */
QStringList FontManager::loadedFontFamilies() const { return m_loadedFamilies; }

/** @brief 设置全局缩放因子 @param factor 缩放倍率 */
void FontManager::setScaleFactor(double factor) { if (qAbs(m_scaleFactor - factor) > 0.01) { m_scaleFactor = factor; emit scaleFactorChanged(factor); } }
/** @brief 获取当前缩放因子 @return 缩放倍率 */
double FontManager::scaleFactor() const { return m_scaleFactor; }
/** @brief 按缩放因子计算实际字号 @param basePointSize 基础字号 @return 缩放后字号 */
int FontManager::scaledSize(int basePointSize) const { ++m_totalScaleOps; return qMax(6, qRound(basePointSize * m_scaleFactor)); }
/** @brief 设置字体角色的回退链 @param role 字体角色 @param families 回退字体族列表 */
void FontManager::setFallbackChain(FontRole role, const QStringList &families) { if (m_roles.contains(role)) m_roles[role].fallbackFamilies = families; }
/** @brief 获取字体角色的回退链 @param role 字体角色 @return 回退字体族列表 */
QStringList FontManager::fallbackChain(FontRole role) const { auto it = m_roles.find(role); return it != m_roles.end() ? it->fallbackFamilies : QStringList(); }

/** @brief 获取累计字体请求次数 @return 请求次数 */
quint64 FontManager::totalFontRequests() const { return m_totalFontRequests; }
/** @brief 获取累计自定义字体加载次数 @return 加载次数 */
quint64 FontManager::totalCustomFontsLoaded() const { return m_totalCustomFontsLoaded; }
/** @brief 获取累计缩放计算次数 @return 缩放操作次数 */
quint64 FontManager::totalScaleOps() const { return m_totalScaleOps; }
/** @brief 重置所有统计计数器 */
void FontManager::resetStatistics() { m_totalFontRequests = 0; m_totalCustomFontsLoaded = 0; m_totalScaleOps = 0; }

/** @brief 解析字体角色的可用字体族(优先主字体→回退链→系统默认) @param role 字体角色 @return 可用字体族名称 */
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

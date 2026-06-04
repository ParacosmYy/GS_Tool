/**
 * @file ThemeManager.cpp
 * @brief 主题管理器实现 - QSS样式表加载、系统主题检测、持久化和切换动画
 *
 * 核心功能:
 *   1. 从Qt资源加载内置QSS主题并应用到全局qApp
 *   2. 检测Windows系统暗色/亮色模式（注册表读取）
 *   3. 通过SettingsManager持久化用户主题选择
 *   4. 主题切换时的QPropertyAnimation淡入淡出过渡（300ms InOutCubic）
 *   5. 解析QSS中的 --semantic-XXX 自定义属性为语义色板
 */

#include "core/theme/ThemeManager.h"
#include "utils/settings/SettingsManager.h"
#include "shared/AnimationConstants.h"

#include <QApplication>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QRegularExpression>
#include <QWidget>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QAbstractAnimation>

#ifdef Q_OS_WIN
#include <QSettings>
#endif

ThemeManager& ThemeManager::instance()
{
    static ThemeManager inst;
    return inst;
}

/** @brief 构造函数 - 注册内置主题并加载默认色板(不执行QSS加载) @param parent 父对象 */
ThemeManager::ThemeManager(QObject* parent)
    : QObject(parent)
    , m_transitionWidget(nullptr)
    , m_opacityEffect(nullptr)
{
    // 注册内置主题 (从Qt资源文件加载)
    m_themes["dark_terminal"] = ":/themes/dark_terminal.qss";
    m_themes["modern_dark"]   = ":/themes/modern_dark.qss";
    m_themes["light"]         = ":/themes/light.qss";

    // 加载默认色板（兜底值，任何主题未定义的颜色都用这里的值）
    loadDefaultColors();
}

/** @brief 加载默认色板(Catppuccin Mocha)，QSS解析失败时作为兜底颜色 */
void ThemeManager::loadDefaultColors()
{
    // ---- Catppuccin Mocha 色板（dark_terminal 默认值）----
    m_colorMap[SemanticColor::BgPrimary]       = QColor(30, 30, 46);    // #1e1e2e
    m_colorMap[SemanticColor::BgSecondary]     = QColor(49, 50, 68);    // #313244
    m_colorMap[SemanticColor::BgTertiary]      = QColor(69, 71, 90);    // #45475a
    m_colorMap[SemanticColor::BgHover]         = QColor(69, 71, 90);    // #45475a
    m_colorMap[SemanticColor::TextPrimary]     = QColor(205, 214, 244); // #cdd6f4
    m_colorMap[SemanticColor::TextSecondary]   = QColor(166, 173, 200); // #a6adc8
    m_colorMap[SemanticColor::TextMuted]       = QColor(108, 112, 134); // #6c7086
    m_colorMap[SemanticColor::Accent]          = QColor(137, 180, 250); // #89b4fa
    m_colorMap[SemanticColor::AccentHover]     = QColor(180, 208, 251); // #b4d0fb
    m_colorMap[SemanticColor::AccentPressed]   = QColor(116, 168, 247); // #74a8f7
    m_colorMap[SemanticColor::Border]          = QColor(49, 50, 68);    // #313244
    m_colorMap[SemanticColor::BorderFocus]     = QColor(137, 180, 250); // #89b4fa
    m_colorMap[SemanticColor::Success]         = QColor(166, 227, 161); // #a6e3a1
    m_colorMap[SemanticColor::Warning]         = QColor(249, 226, 175); // #f9e2af
    m_colorMap[SemanticColor::Error]           = QColor(243, 139, 168); // #f38ba8
    m_colorMap[SemanticColor::Scrollbar]       = QColor(69, 71, 90);    // #45475a
    m_colorMap[SemanticColor::ScrollbarHover]  = QColor(88, 91, 112);   // #585b70
    m_colorMap[SemanticColor::Shadow]          = QColor(0, 0, 0, 77);   // rgba(0,0,0,0.3)

    // ---- 终端专用颜色（Catppuccin Mocha）----
    m_colorMap[SemanticColor::TermBackground]      = QColor(30, 30, 46);    // #1e1e2e
    m_colorMap[SemanticColor::TermRxText]          = QColor(205, 214, 244); // #cdd6f4 接收文本
    m_colorMap[SemanticColor::TermTxText]          = QColor(166, 227, 161); // #a6e3a1 发送文本
    m_colorMap[SemanticColor::TermTimestamp]       = QColor(147, 153, 178); // #9399b2 时间戳
    m_colorMap[SemanticColor::TermSelection]       = QColor(69, 71, 90);    // #45475a 选中背景
    m_colorMap[SemanticColor::TermSearchHighlight] = QColor(249, 226, 175, 80);  // #f9e2af 半透明
    m_colorMap[SemanticColor::TermCurrentMatch]    = QColor(249, 226, 175, 180); // #f9e2af 高不透明度
}

/** @brief 加载指定内置主题(查找QSS→读取→带动画应用→解析语义色板) @param themeName 主题名称 @return true加载成功 */
bool ThemeManager::loadTheme(const QString& themeName)
{
    if (!m_themes.contains(themeName)) {
        qWarning() << "Theme not found:" << themeName;
        return false;
    }

    QFile file(m_themes[themeName]);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open theme file:" << m_themes[themeName];
        return false;
    }

    QString qss = QString::fromUtf8(file.readAll());
    m_currentTheme = themeName;

    // 首次加载不带动画（避免启动时闪烁），后续切换带淡入淡出
    if (m_transitionWidget && qApp->styleSheet().length() > 0) {
        applyStylesheetWithAnimation(qss);
    } else {
        applyStylesheetDirect(qss);
    }

    // 从QSS中解析语义色板覆盖默认值
    parseColorsFromQss(qss);

    // 统计：累计主题切换计数
    ++m_totalThemeSwitches;

    emit themeChanged();
    qDebug() << "Theme loaded:" << themeName;
    return true;
}

/** @brief 从外部QSS文件加载自定义主题 @param filePath QSS文件路径 @return true加载成功 */
bool ThemeManager::loadThemeFromFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open theme file:" << filePath;
        return false;
    }

    QString qss = QString::fromUtf8(file.readAll());

    QFileInfo info(filePath);
    m_currentTheme = info.completeBaseName();

    if (m_transitionWidget && qApp->styleSheet().length() > 0) {
        applyStylesheetWithAnimation(qss);
    } else {
        applyStylesheetDirect(qss);
    }

    parseColorsFromQss(qss);

    // 统计：累计自定义主题加载计数
    ++m_totalCustomThemesLoaded;
    // 统计：累计主题切换计数（自定义加载也算一次切换）
    ++m_totalThemeSwitches;

    emit themeChanged();
    qDebug() << "Theme loaded from file:" << filePath;
    return true;
}

/** @brief 获取所有可用主题名称列表 @return 主题名QStringList */
QStringList ThemeManager::availableThemes() const
{
    return m_themes.keys();
}

/** @brief 获取当前激活主题名称 @return 主题名 */
QString ThemeManager::currentTheme() const
{
    return m_currentTheme;
}

/** @brief 按语义色枚举获取对应QColor(未映射时返回灰色并警告) @param color 语义色枚举 @return QColor */
QColor ThemeManager::color(SemanticColor color) const
{
    // 统计：累计语义色查询计数
    ++m_totalColorQueries;

    auto it = m_colorMap.constFind(color);
    if (it != m_colorMap.constEnd()) {
        ++m_totalCacheHits;  // 统计: 语义色缓存命中
        return it.value();
    }
    // 兜底: 返回深灰色，避免程序崩溃
    ++m_totalCacheMisses;  // 统计: 语义色缓存未命中
    qWarning() << "ThemeManager: unmapped SemanticColor" << static_cast<int>(color);
    return QColor(128, 128, 128);
}

/** @brief 检测Windows系统当前是否为暗色模式(读取注册表AppsUseLightTheme) @return true系统为暗色模式 */
bool ThemeManager::isSystemDarkMode() const
{
#ifdef Q_OS_WIN
    QSettings registry(
        "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        QSettings::NativeFormat
    );
    // AppsUseLightTheme: 0 = 暗色, 1 = 亮色
    bool isLight = registry.value("AppsUseLightTheme", 1).toInt() != 0;
    return !isLight;
#else
    // 非Windows平台默认暗色
    return true;
#endif
}

/** @brief 根据系统主题自动选择并加载对应主题(暗色→dark_terminal/亮色→light) @return 实际加载的主题名称 */
QString ThemeManager::loadSystemTheme()
{
    QString themeName = isSystemDarkMode() ? "dark_terminal" : "light";
    loadTheme(themeName);
    return themeName;
}

/** @brief 持久化当前主题到SettingsManager(写入theme/name配置项) */
void ThemeManager::saveTheme() const
{
    if (!m_currentTheme.isEmpty()) {
        SettingsManager::instance().saveTheme(m_currentTheme);
    }
}

/** @brief 从SettingsManager加载上次保存的主题(无记录时回退到系统主题) @return true成功加载已保存的主题 */
bool ThemeManager::loadSavedTheme()
{
    auto& settings = SettingsManager::instance();
    QString savedTheme = settings.loadTheme();

    // loadTheme() 返回默认值 App::DEFAULT_THEME 表示无保存记录
    // 尝试用保存的主题名加载，若失败则回退到系统主题
    if (m_themes.contains(savedTheme)) {
        return loadTheme(savedTheme);
    }

    // 无保存记录或主题不存在，使用系统主题
    loadSystemTheme();
    return false;
}

/** @brief 设置主题切换动画目标widget(通常是centralWidget) @param widget 目标widget */
void ThemeManager::setTransitionWidget(QWidget* widget)
{
    m_transitionWidget = widget;
    if (widget) {
        // 创建透明度特效（初始完全可见）
        m_opacityEffect = new QGraphicsOpacityEffect(widget);
        m_opacityEffect->setOpacity(1.0);
        widget->setGraphicsEffect(m_opacityEffect);
    }
}

/** @brief 带淡入淡出动画应用样式表(300ms InOutCubic) @param qss 新的QSS样式表内容 */
void ThemeManager::applyStylesheetWithAnimation(const QString& qss)
{
    if (!m_transitionWidget || !m_opacityEffect) {
        applyStylesheetDirect(qss);
        return;
    }

    // 淡出动画: InOutCubic
    QPropertyAnimation* fadeOut = new QPropertyAnimation(m_opacityEffect, "opacity");
    fadeOut->setDuration(Animations::kThemeFadeMs);
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.0);
    fadeOut->setEasingCurve(QEasingCurve::InOutCubic);

    // 清理上一次未完成的淡入动画(防止快速切换主题时fadeIn泄漏)
    // 查找ThemeManager的所有QPropertyAnimation子对象并停止/删除
    const auto fadeChildren = findChildren<QPropertyAnimation*>();
    for (auto* oldAnim : fadeChildren) {
        if (oldAnim->state() == QAbstractAnimation::Running) oldAnim->stop();
        delete oldAnim;
    }

    // 淡入动画: InOutCubic
    QPropertyAnimation* fadeIn = new QPropertyAnimation(m_opacityEffect, "opacity");
    fadeIn->setParent(this);  // 父对象设为ThemeManager，生命周期独立于fadeOut
    fadeIn->setDuration(Animations::kThemeFadeMs);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    fadeIn->setEasingCurve(QEasingCurve::InOutCubic);

    // 淡出完成后: 应用新样式表并启动淡入
    connect(fadeOut, &QPropertyAnimation::finished, this, [this, qss, fadeIn]() {
        qApp->setStyleSheet(qss);
        // 统计：累计样式表应用计数（带动画）
        ++m_totalStyleApplications;
        fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
    });

    fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
}

/** @brief 不带动画直接应用样式表(首次加载使用) @param qss QSS样式表内容 */
void ThemeManager::applyStylesheetDirect(const QString& qss)
{
    qApp->setStyleSheet(qss);
    // 统计：累计样式表应用计数（直接应用）
    ++m_totalStyleApplications;
}

/** @brief 从QSS内容中解析语义色板(--semantic-XXX格式覆盖默认值) @param qssContent QSS文件内容 */
void ThemeManager::parseColorsFromQss(const QString& qssContent)
{
    static const QRegularExpression regex(
        QLatin1String(R"(--semantic-(\w+)\s*:\s*([^;]+);)")
    );

    // 枚举名 -> 枚举值映射表
    static const QMap<QString, SemanticColor> nameMap = {
        {"BgPrimary",       SemanticColor::BgPrimary},
        {"BgSecondary",     SemanticColor::BgSecondary},
        {"BgTertiary",      SemanticColor::BgTertiary},
        {"BgHover",         SemanticColor::BgHover},
        {"TextPrimary",     SemanticColor::TextPrimary},
        {"TextSecondary",   SemanticColor::TextSecondary},
        {"TextMuted",       SemanticColor::TextMuted},
        {"Accent",          SemanticColor::Accent},
        {"AccentHover",     SemanticColor::AccentHover},
        {"AccentPressed",   SemanticColor::AccentPressed},
        {"Border",          SemanticColor::Border},
        {"BorderFocus",     SemanticColor::BorderFocus},
        {"Success",         SemanticColor::Success},
        {"Warning",         SemanticColor::Warning},
        {"Error",           SemanticColor::Error},
        {"Scrollbar",       SemanticColor::Scrollbar},
        {"ScrollbarHover",  SemanticColor::ScrollbarHover},
        {"Shadow",          SemanticColor::Shadow},
        {"TermBackground",      SemanticColor::TermBackground},
        {"TermRxText",          SemanticColor::TermRxText},
        {"TermTxText",          SemanticColor::TermTxText},
        {"TermTimestamp",       SemanticColor::TermTimestamp},
        {"TermSelection",       SemanticColor::TermSelection},
        {"TermSearchHighlight", SemanticColor::TermSearchHighlight},
        {"TermCurrentMatch",    SemanticColor::TermCurrentMatch},
    };

    QRegularExpressionMatchIterator it = regex.globalMatch(qssContent);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString name = match.captured(1);
        QString value = match.captured(2).trimmed();

        auto enumIt = nameMap.constFind(name);
        if (enumIt != nameMap.constEnd()) {
            QColor color(value);
            if (color.isValid()) {
                m_colorMap[enumIt.value()] = color;
            } else {
                qWarning() << "ThemeManager: invalid color for" << name << ":" << value;
            }
        }
    }
}

// ==================== 统计接口 ====================

/** @brief 获取累计主题切换次数 */
quint64 ThemeManager::totalThemeSwitches() const
{
    return m_totalThemeSwitches;
}

/** @brief 获取累计自定义主题加载次数 */
quint64 ThemeManager::totalCustomThemesLoaded() const
{
    return m_totalCustomThemesLoaded;
}

/** @brief 获取累计语义色查询次数 */
quint64 ThemeManager::totalColorQueries() const
{
    return m_totalColorQueries;
}

/** @brief 获取累计样式表应用次数 */
quint64 ThemeManager::totalStyleApplications() const
{
    return m_totalStyleApplications;
}

/** @brief 获取累计语义色缓存命中次数 @return 缓存命中次数 */
quint64 ThemeManager::totalCacheHits() const
{
    return m_totalCacheHits;
}

/** @brief 获取累计语义色缓存未命中次数 @return 缓存未命中次数 */
quint64 ThemeManager::totalCacheMisses() const
{
    return m_totalCacheMisses;
}

/** @brief 重置所有统计计数器为零 */
void ThemeManager::resetStats()
{
    m_totalThemeSwitches = 0;
    m_totalCustomThemesLoaded = 0;
    m_totalColorQueries = 0;
    m_totalStyleApplications = 0;
    m_totalCacheHits = 0;
    m_totalCacheMisses = 0;
}

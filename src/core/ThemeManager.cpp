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

#include "ThemeManager.h"
#include "utils/SettingsManager.h"
#include "Constants.h"

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

/**
 * @brief 构造函数 - 注册内置主题并加载默认色板
 *
 * 不执行任何QSS加载，首次主题加载由 loadSavedTheme() 或 loadTheme() 触发。
 */
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

/**
 * @brief 加载默认色板（Catppuccin Mocha）
 *
 * 当QSS文件中未定义对应变量时，使用这些兜底颜色。
 * 这保证了即使QSS解析失败，自绘控件也有合理的颜色。
 */
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

    // ---- 终端专用颜色（Catppuccin Mocha）----
    m_colorMap[SemanticColor::TermBackground]      = QColor(30, 30, 46);    // #1e1e2e
    m_colorMap[SemanticColor::TermRxText]          = QColor(205, 214, 244); // #cdd6f4 接收文本
    m_colorMap[SemanticColor::TermTxText]          = QColor(166, 227, 161); // #a6e3a1 发送文本
    m_colorMap[SemanticColor::TermTimestamp]       = QColor(147, 153, 178); // #9399b2 时间戳
    m_colorMap[SemanticColor::TermSelection]       = QColor(69, 71, 90);    // #45475a 选中背景
    m_colorMap[SemanticColor::TermSearchHighlight] = QColor(249, 226, 175, 80);  // #f9e2af 半透明
    m_colorMap[SemanticColor::TermCurrentMatch]    = QColor(249, 226, 175, 180); // #f9e2af 高不透明度
}

/**
 * @brief 加载指定内置主题
 *
 * 流程: 查找主题QSS文件 -> 读取内容 -> 带动画应用样式表 -> 解析语义色板
 *
 * @param themeName 主题名称
 * @return true 加载成功
 */
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

    emit themeChanged();
    qDebug() << "Theme loaded:" << themeName;
    return true;
}

/**
 * @brief 从外部QSS文件加载自定义主题
 * @param filePath QSS文件路径
 * @return true 加载成功
 */
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

    emit themeChanged();
    qDebug() << "Theme loaded from file:" << filePath;
    return true;
}

QStringList ThemeManager::availableThemes() const
{
    return m_themes.keys();
}

QString ThemeManager::currentTheme() const
{
    return m_currentTheme;
}

QColor ThemeManager::color(SemanticColor color) const
{
    auto it = m_colorMap.constFind(color);
    if (it != m_colorMap.constEnd()) {
        return it.value();
    }
    // 兜底: 返回深灰色，避免程序崩溃
    qWarning() << "ThemeManager: unmapped SemanticColor" << static_cast<int>(color);
    return QColor(128, 128, 128);
}

/**
 * @brief 检测 Windows 系统当前是否为暗色模式
 *
 * 读取注册表 AppsUseLightTheme 键值:
 *   - 值为 0: 暗色模式
 *   - 值为 1: 亮色模式
 *   - 读取失败: 默认返回 false（亮色）
 *
 * @return true 系统为暗色模式
 */
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

/**
 * @brief 根据系统主题自动选择并加载对应主题
 *
 * 暗色模式 -> dark_terminal
 * 亮色模式 -> light
 *
 * @return 实际加载的主题名称
 */
QString ThemeManager::loadSystemTheme()
{
    QString themeName = isSystemDarkMode() ? "dark_terminal" : "light";
    loadTheme(themeName);
    return themeName;
}

/**
 * @brief 持久化当前主题到 SettingsManager
 *
 * 将主题名称写入 "theme/name" 配置项。
 * 由 SettingsController 在主题切换和窗口关闭时调用。
 */
void ThemeManager::saveTheme() const
{
    if (!m_currentTheme.isEmpty()) {
        SettingsManager::instance().saveTheme(m_currentTheme);
    }
}

/**
 * @brief 从 SettingsManager 加载上次保存的主题
 *
 * 若无保存记录，回退到系统主题检测。
 *
 * @return true 成功加载已保存的主题
 */
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

/**
 * @brief 设置主题切换动画目标 widget
 *
 * @param widget 通常是 MainWindow 的 centralWidget
 */
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

/**
 * @brief 带淡入淡出动画应用样式表
 *
 * 动画流程:
 *   1. 淡出（opacity 1.0 -> 0.0, 300ms, InOutCubic）
 *   2. 在淡出完成时应用新QSS
 *   3. 淡入（opacity 0.0 -> 1.0, 300ms, InOutCubic）
 *
 * @param qss 新的QSS样式表内容
 */
void ThemeManager::applyStylesheetWithAnimation(const QString& qss)
{
    if (!m_transitionWidget || !m_opacityEffect) {
        applyStylesheetDirect(qss);
        return;
    }

    // 淡出动画: 300ms InOutCubic
    QPropertyAnimation* fadeOut = new QPropertyAnimation(m_opacityEffect, "opacity");
    fadeOut->setDuration(300);
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.0);
    fadeOut->setEasingCurve(QEasingCurve::InOutCubic);

    // 淡入动画: 300ms InOutCubic
    QPropertyAnimation* fadeIn = new QPropertyAnimation(m_opacityEffect, "opacity");
    fadeIn->setDuration(300);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    fadeIn->setEasingCurve(QEasingCurve::InOutCubic);

    // 淡出完成后: 应用新样式表并启动淡入
    connect(fadeOut, &QPropertyAnimation::finished, this, [this, qss, fadeIn]() {
        qApp->setStyleSheet(qss);
        fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
    });

    fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
}

/**
 * @brief 不带动画直接应用样式表（首次加载使用）
 * @param qss QSS样式表内容
 */
void ThemeManager::applyStylesheetDirect(const QString& qss)
{
    qApp->setStyleSheet(qss);
}

/**
 * @brief 从QSS内容中解析语义色板
 *
 * 解析格式: --semantic-<枚举名>: <颜色值>;
 * 例如: --semantic-BgPrimary: #1e1e2e;
 * 解析结果覆盖 loadDefaultColors() 中的默认值。
 *
 * @param qssContent QSS文件内容
 */
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

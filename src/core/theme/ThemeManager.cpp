/**
 * @file ThemeManager.cpp
 * @brief 主题管理器实现 - 单例入口、构造函数、主题加载入口方法
 *
 * 核心功能:
 *   1. 单例 instance() 和构造函数（注册内置主题、加载默认色板）
 *   2. loadTheme() / loadThemeFromFile() 主题加载入口
 *
 * 主题应用/查询/动画/统计方法见 ThemeManagerApply.cpp
 * 语义色板相关方法（loadDefaultColors / color / parseColorsFromQss）见 ThemeManagerColor.cpp
 */

#include "core/theme/ThemeManager.h"

#include <QApplication>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QWidget>

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

/** @brief 加载指定内置主题(查找QSS→读取→带动画应用→解析语义色板) @param themeName 主题名称 @return true加载成功 */
bool ThemeManager::loadTheme(const QString& themeName)
{
    if (!m_themes.contains(themeName)) {
        qWarning() << "Theme not found:" << themeName;
        ++m_totalThemeLoadFailures;
        return false;
    }

    QFile file(m_themes[themeName]);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open theme file:" << m_themes[themeName];
        ++m_totalThemeLoadFailures;
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
        ++m_totalThemeLoadFailures;
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


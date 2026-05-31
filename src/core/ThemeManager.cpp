#include "ThemeManager.h"
#include <QApplication>
#include <QFile>
#include <QFileInfo>
#include <QDebug>

ThemeManager& ThemeManager::instance()
{
    static ThemeManager inst;
    return inst;
}

ThemeManager::ThemeManager(QObject* parent)
    : QObject(parent)
{
    // 注册内置主题 (从Qt资源文件加载)
    m_themes["dark_terminal"] = ":/themes/dark_terminal.qss";
    m_themes["modern_dark"]   = ":/themes/modern_dark.qss";
    m_themes["light"]         = ":/themes/light.qss";
}

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
    qApp->setStyleSheet(qss);
    m_currentTheme = themeName;

    qDebug() << "Theme loaded:" << themeName;
    return true;
}

bool ThemeManager::loadThemeFromFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open theme file:" << filePath;
        return false;
    }

    QString qss = QString::fromUtf8(file.readAll());
    qApp->setStyleSheet(qss);

    // 用文件名（不含路径和后缀）作为当前主题标识
    QFileInfo info(filePath);
    m_currentTheme = info.completeBaseName();

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

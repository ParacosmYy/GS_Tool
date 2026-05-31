#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QObject>
#include <QString>
#include <QMap>

// 主题管理器 - 加载和切换QSS样式表
class ThemeManager : public QObject {
    Q_OBJECT

public:
    static ThemeManager& instance();

    // 加载指定主题
    bool loadTheme(const QString& themeName);

    // 从外部文件加载自定义主题（方便用户自定义QSS）
    bool loadThemeFromFile(const QString& filePath);

    // 获取可用主题列表
    QStringList availableThemes() const;

    // 获取当前主题名称
    QString currentTheme() const;

private:
    ThemeManager(QObject* parent = nullptr);
    QString m_currentTheme;
    QMap<QString, QString> m_themes;   // 主题名 → QSS文件路径
};

#endif // THEMEMANAGER_H

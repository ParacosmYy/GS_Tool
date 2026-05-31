#include "ThemeManager.h"
#include <QApplication>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QRegularExpression>

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

    // 加载默认色板（兜底值，任何主题未定义的颜色都用这里的值）
    loadDefaultColors();
}

void ThemeManager::loadDefaultColors()
{
    // ---- Catppuccin Mocha 色板（dark_terminal 默认值）----
    // 当QSS文件中未定义对应变量时，使用这些兜底颜色
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

    // 从QSS中解析语义色板覆盖默认值
    parseColorsFromQss(qss);

    emit themeChanged();
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

    QFileInfo info(filePath);
    m_currentTheme = info.completeBaseName();

    // 从QSS中解析语义色板覆盖默认值
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

void ThemeManager::parseColorsFromQss(const QString& qssContent)
{
    // 解析QSS中的CSS自定义属性: --semantic-BgPrimary: #1e1e2e;
    // 格式: --semantic-<枚举名>: <颜色值>;
    // 枚举名与SemanticColor枚举的toString()一致
    static const QRegularExpression regex(
        QLatin1String(R"(--semantic-(\w+)\s*:\s*([^;]+);)")
    );

    // 枚举名 → 枚举值映射表
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

/** @file ThemeManager.h @brief 主题管理器 -- 单例，负责QSS加载/系统主题检测/语义色板查询。支持淡入淡出切换动画(300ms InOutCubic) */
#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QColor>
#include <QPointer>

class QWidget;
class QGraphicsOpacityEffect;

/**
 * @brief 主题管理器 -- 单例模式
 * 所有自绘控件通过color(SemanticColor)查询当前主题的语义色值。
 * 协作: MainWindow(加载) / TerminalWidget(自绘) / SettingsController(持久化) / QSS文件(样式)
 */
class ThemeManager : public QObject {
    Q_OBJECT

public:
    /** @brief 语义色板枚举 -- 所有自绘控件通过此枚举查询颜色 */
    enum class SemanticColor {
        BgPrimary, BgSecondary, BgTertiary, BgHover,           ///< 背景系列
        TextPrimary, TextSecondary, TextMuted,                  ///< 文字系列
        Accent, AccentHover, AccentPressed,                     ///< 强调色系列
        Border, BorderFocus,                                    ///< 边框系列
        Success, Warning, Error,                                ///< 语义色
        Scrollbar, ScrollbarHover, Shadow,                      ///< UI元素
        TermBackground, TermRxText, TermTxText, TermTimestamp,  ///< 终端系列
        TermSelection, TermSearchHighlight, TermCurrentMatch,   ///< 终端选择/搜索
    };
    Q_ENUM(SemanticColor)

    static ThemeManager& instance();
    /** @brief 加载内置主题 @param themeName 主题名(如"dark_terminal") @return true成功 */
    bool loadTheme(const QString& themeName);
    /** @brief 从外部QSS文件加载自定义主题 @param filePath QSS绝对路径 @return true成功 */
    bool loadThemeFromFile(const QString& filePath);
    QStringList availableThemes() const;  ///< 可用主题列表
    QString currentTheme() const;         ///< 当前主题名称
    QColor color(SemanticColor color) const; ///< 查询语义色板颜色
    bool isSystemDarkMode() const;        ///< 检测Windows系统暗色模式
    QString loadSystemTheme();            ///< 根据系统主题自动选择并加载
    void saveTheme() const;               ///< 持久化当前主题到SettingsManager
    bool loadSavedTheme();                ///< 从SettingsManager加载上次保存的主题
    void setTransitionWidget(QWidget* widget); ///< 设置淡入淡出动画目标widget
    // ==================== 统计接口 ====================
    quint64 totalThemeSwitches() const;    ///< 累计主题切换次数
    quint64 totalCustomThemesLoaded() const; ///< 累计自定义主题加载次数
    quint64 totalThemeReloads() const;     ///< 累计主题重新加载次数(系统/保存主题)
    quint64 totalColorQueries() const;     ///< 累计语义色查询次数
    quint64 totalStyleApplications() const; ///< 累计样式表应用次数
    quint64 totalCacheHits() const;        ///< 累计语义色缓存命中次数
    quint64 totalCacheMisses() const;      ///< 累计语义色缓存未命中次数
    quint64 totalThemeLoadFailures() const; ///< 获取累计主题加载失败次数
    quint64 totalSystemThemeChecks() const; ///< 获取累计系统暗色模式检测次数
    void resetStats();                     ///< 重置所有统计计数器

signals:
    void themeChanged(); ///< 主题切换后发射，自绘控件应重绘

private:
    ThemeManager(QObject* parent = nullptr);
    void parseColorsFromQss(const QString& qssContent); ///< 从QSS解析语义色板
    void loadDefaultColors();                            ///< 加载默认色板(dark_terminal兜底)
    void applyStylesheetWithAnimation(const QString& qss); ///< 带动画应用QSS
    void applyStylesheetDirect(const QString& qss);        ///< 不带动画直接应用(首次)
    QString m_currentTheme;                 ///< 当前主题名
    QMap<QString, QString> m_themes;        ///< 主题名->QSS文件路径
    QMap<SemanticColor, QColor> m_colorMap; ///< 语义色->颜色值
    QPointer<QWidget> m_transitionWidget;          ///< 主题切换动画目标
    QPointer<QGraphicsOpacityEffect> m_opacityEffect; ///< 透明度特效
    // ---- 统计计数器 ----
    quint64 m_totalThemeSwitches = 0, m_totalCustomThemesLoaded = 0, m_totalThemeReloads = 0;
    mutable quint64 m_totalColorQueries = 0, m_totalCacheHits = 0, m_totalCacheMisses = 0;
    quint64 m_totalStyleApplications = 0;
    quint64 m_totalThemeLoadFailures = 0;       ///< 累计主题加载失败次数
    mutable quint64 m_totalSystemThemeChecks = 0; ///< 累计系统暗色模式检测次数
};

#endif // THEMEMANAGER_H

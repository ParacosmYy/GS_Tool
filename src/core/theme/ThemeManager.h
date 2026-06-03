/**
 * @file ThemeManager.h
 * @brief 主题管理器 -- 单例，负责QSS样式表加载、系统主题检测和语义色板查询
 *
 * 职责:
 *   1. 加载/切换QSS主题文件（应用到全局qApp）
 *   2. 检测 Windows 系统暗色/亮色模式
 *   3. 通过 SettingsManager 持久化用户主题选择
 *   4. 切换主题时带 QPropertyAnimation 淡入淡出过渡（300ms InOutCubic）
 *   5. 为自绘控件（TerminalWidget等使用QPainter的控件）提供语义色板查询
 *   6. 主题切换时发射 themeChanged() 信号，通知自绘控件重绘
 *
 * 协作关系:
 *   - MainWindow: 启动时加载默认主题，设置菜单切换
 *   - TerminalWidget: 通过 color() 获取自绘颜色，监听 themeChanged 重绘
 *   - SettingsController: 通过 saveTheme/loadTheme 持久化主题选择
 *   - QSS文件: 定义所有QWidget子控件的样式（非自绘部分）
 */

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
 *
 * 提供主题加载、系统主题检测、持久化存储和切换动画功能。
 * 所有自绘控件通过 color(SemanticColor) 查询当前主题的语义色值。
 */
class ThemeManager : public QObject {
    Q_OBJECT

public:
    /** @brief 语义色板枚举 -- 所有自绘控件通过此枚举查询颜色 */
    enum class SemanticColor {
        BgPrimary,       ///< 主背景（最深，整个窗口底色）
        BgSecondary,     ///< 次背景（面板/卡片背景）
        BgTertiary,      ///< 三级背景（输入框/悬浮提示背景）
        BgHover,         ///< 鼠标悬浮背景
        TextPrimary,     ///< 主文字（高对比度）
        TextSecondary,   ///< 次文字（描述、标签）
        TextMuted,       ///< 弱文字（占位符、禁用态）
        Accent,          ///< 强调色（主操作按钮、选中态）
        AccentHover,     ///< 强调色悬停
        AccentPressed,   ///< 强调色按下
        Border,          ///< 边框色
        BorderFocus,     ///< 焦点边框色
        Success,         ///< 成功色
        Warning,         ///< 警告色
        Error,           ///< 错误色
        Scrollbar,       ///< 滚动条默认色
        ScrollbarHover,  ///< 滚动条悬停色
        Shadow,          ///< 阴影色（面板浮起时的阴影）
        // ---- 终端专用颜色 ----
        TermBackground,  ///< 终端背景色
        TermRxText,      ///< 终端接收文本色
        TermTxText,      ///< 终端发送文本色
        TermTimestamp,   ///< 终端时间戳色
        TermSelection,   ///< 终端选中背景色
        TermSearchHighlight, ///< 终端搜索高亮色
        TermCurrentMatch,    ///< 终端当前匹配色
    };
    Q_ENUM(SemanticColor)

    static ThemeManager& instance();

    /**
     * @brief 加载指定内置主题
     * @param themeName 主题名称（如 "dark_terminal"）
     * @return true 加载成功，false 主题不存在或文件无法读取
     */
    bool loadTheme(const QString& themeName);

    /**
     * @brief 从外部QSS文件加载自定义主题
     * @param filePath QSS文件绝对路径
     * @return true 加载成功
     */
    bool loadThemeFromFile(const QString& filePath);

    /** @brief 获取可用主题列表 */
    QStringList availableThemes() const;

    /** @brief 获取当前主题名称 */
    QString currentTheme() const;

    /** @brief 查询语义色板中的颜色值 */
    QColor color(SemanticColor color) const;

    /** @brief 检测 Windows 系统当前是否为暗色模式 */
    bool isSystemDarkMode() const;

    /** @brief 根据系统主题自动选择并加载对应主题 */
    QString loadSystemTheme();

    /** @brief 持久化当前主题到 SettingsManager */
    void saveTheme() const;

    /** @brief 从 SettingsManager 加载上次保存的主题 */
    bool loadSavedTheme();

    /** @brief 设置需要执行淡入淡出动画的目标 widget */
    void setTransitionWidget(QWidget* widget);

    // ==================== 统计接口 ====================

    /** @brief 获取累计主题切换次数 */
    quint64 totalThemeSwitches() const;

    /** @brief 获取累计自定义主题加载次数 */
    quint64 totalCustomThemesLoaded() const;

    /** @brief 获取累计语义色查询次数 */
    quint64 totalColorQueries() const;

    /** @brief 获取累计样式表应用次数 */
    quint64 totalStyleApplications() const;

    /** @brief 重置所有统计计数器 */
    void resetStats();

signals:
    /** @brief 主题切换后发射，自绘控件应监听此信号并重绘 */
    void themeChanged();

private:
    ThemeManager(QObject* parent = nullptr);

    /** @brief 从当前QSS文件中解析语义色板到 m_colorMap */
    void parseColorsFromQss(const QString& qssContent);

    /** @brief 加载默认色板（dark_terminal主题的色值），作为兜底 */
    void loadDefaultColors();

    /** @brief 带动画应用QSS样式表到全局 qApp */
    void applyStylesheetWithAnimation(const QString& qss);

    /** @brief 不带动画直接应用样式表（用于首次加载） */
    void applyStylesheetDirect(const QString& qss);

    QString m_currentTheme;                 ///< 当前激活的主题名称
    QMap<QString, QString> m_themes;        ///< 主题名 -> QSS文件路径
    QMap<SemanticColor, QColor> m_colorMap; ///< 语义色 -> 当前颜色值

    /** @brief 主题切换动画目标 widget */
    QPointer<QWidget> m_transitionWidget;

    /** @brief 透明度特效，用于主题切换淡入淡出动画 */
    QPointer<QGraphicsOpacityEffect> m_opacityEffect;

    // ---- 统计计数器 ----
    quint64 m_totalThemeSwitches = 0;       ///< 累计主题切换次数
    quint64 m_totalCustomThemesLoaded = 0;  ///< 累计自定义主题加载次数
    mutable quint64 m_totalColorQueries = 0;        ///< 累计语义色查询次数
    quint64 m_totalStyleApplications = 0;   ///< 累计样式表应用次数
};

#endif // THEMEMANAGER_H

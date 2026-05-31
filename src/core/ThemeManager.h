#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QColor>

/**
 * @file ThemeManager.h
 * @brief 主题管理器 — 单例，负责QSS样式表加载和语义色板查询
 *
 * 职责:
 *   1. 加载/切换QSS主题文件（应用到全局qApp）
 *   2. 为自绘控件（TerminalWidget等使用QPainter的控件）提供语义色板查询
 *   3. 主题切换时发射 themeChanged() 信号，通知自绘控件重绘
 *
 * 协作关系:
 *   - MainWindow: 启动时加载默认主题，设置菜单切换
 *   - TerminalWidget: 通过 color() 获取自绘颜色，监听 themeChanged 重绘
 *   - QSS文件: 定义所有QWidget子控件的样式（非自绘部分）
 */
class ThemeManager : public QObject {
    Q_OBJECT

public:
    /** @brief 语义色板枚举 — 所有自绘控件通过此枚举查询颜色 */
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

    /** @brief 加载指定内置主题 */
    bool loadTheme(const QString& themeName);

    /** @brief 从外部QSS文件加载自定义主题 */
    bool loadThemeFromFile(const QString& filePath);

    /** @brief 获取可用主题列表 */
    QStringList availableThemes() const;

    /** @brief 获取当前主题名称 */
    QString currentTheme() const;

    /**
     * @brief 查询语义色板中的颜色值
     * @param color 语义色枚举
     * @return 对应的QColor（若主题未定义该色则返回深灰色兜底）
     */
    QColor color(SemanticColor color) const;

signals:
    /** @brief 主题切换后发射，自绘控件应监听此信号并重绘 */
    void themeChanged();

private:
    ThemeManager(QObject* parent = nullptr);

    /** @brief 从当前QSS文件中解析语义色板到 m_colorMap */
    void parseColorsFromQss(const QString& qssContent);

    /** @brief 加载默认色板（dark_terminal主题的色值），作为兜底 */
    void loadDefaultColors();

    QString m_currentTheme;
    QMap<QString, QString> m_themes;   ///< 主题名 → QSS文件路径
    QMap<SemanticColor, QColor> m_colorMap; ///< 语义色 → 当前颜色值
};

#endif // THEMEMANAGER_H

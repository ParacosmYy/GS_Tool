/**
 * @file QssThemeGenerator.h
 * @brief QSS主题生成器 - 从语义色自动生成完整QSS主题
 *
 * 替代手动维护3套共4740行的QSS文件，通过语义色映射
 * 自动生成完整的样式表。支持亮色/暗色/高对比度三种主题。
 *
 * 用法:
 *   QString qss = QssThemeGenerator::generate(ThemeType::Dark);
 *   qApp->setStyleSheet(qss);
 */

#ifndef QSS_THEME_GENERATOR_H
#define QSS_THEME_GENERATOR_H

#include <QString>
#include <QMap>
#include <QColor>

/**
 * @brief 主题类型
 */
enum class ThemeType {
    Dark,            ///< 暗色主题(默认)
    Light,           ///< 亮色主题
    HighContrast     ///< 高对比度主题
};

/**
 * @brief QSS主题生成器
 *
 * 从语义色定义自动生成完整QSS主题，替代手动QSS文件维护。
 * 所有颜色从ThemeManager获取，确保语义一致性。
 */
class QssThemeGenerator {
public:
    /// 语义色映射表(语义名 -> 实际颜色值)
    using ColorMap = QMap<QString, QString>;

    /// 生成完整QSS主题
    static QString generate(ThemeType type);

    /// 获取主题对应的语义色映射
    static ColorMap semanticColors(ThemeType type);

    /// 生成基础控件样式(按钮/输入框/标签/滚动条)
    static QString generateBaseWidgets(const ColorMap& colors);

    /// 生成面板/容器样式(BasePanel/QGroupBox/QTabWidget)
    static QString generatePanels(const ColorMap& colors);

    /// 生成导航栏样式(IconNavBar/侧边栏)
    static QString generateNavigation(const ColorMap& colors);

    /// 生成终端样式(TerminalWidget/搜索栏)
    static QString generateTerminal(const ColorMap& colors);

    /// 生成图表样式(ChartWidget/FFT/直方图)
    static QString generateCharts(const ColorMap& colors);

    /// 生成对话框样式(AppDialog/ToastWidget)
    static QString generateDialogs(const ColorMap& colors);

    /// 生成工具栏样式(Toolbar/快捷命令栏)
    static QString generateToolbar(const ColorMap& colors);

private:
    /// 生成暗色语义色
    static ColorMap darkColors();

    /// 生成亮色语义色
    static ColorMap lightColors();

    /// 生成高对比度语义色
    static ColorMap highContrastColors();

    /// 将QColor转换为hex字符串
    static QString toHex(const QColor& color);
};

#endif // QSS_THEME_GENERATOR_H

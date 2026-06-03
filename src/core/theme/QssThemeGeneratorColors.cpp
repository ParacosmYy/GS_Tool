/**
 * @file QssThemeGeneratorColors.cpp
 * @brief QSS主题生成器 - 语义色定义
 *
 * 从 QssThemeGenerator.cpp 拆分而来，包含三套主题（暗色/亮色/高对比度）
 * 的完整语义色映射表定义。
 */

#include "core/theme/QssThemeGenerator.h"

// ============================================================
// 暗色主题语义色
// ============================================================

/** @brief 返回暗色主题的语义色映射表 @return 暗色主题语义色键值对 */
QssThemeGenerator::ColorMap QssThemeGenerator::darkColors()
{
    return {
        // 背景色系
        {"bg_primary",      "#1e1f26"},
        {"bg_secondary",    "#282a33"},
        {"bg_tertiary",     "#2a2d35"},
        {"bg_elevated",     "#353840"},
        {"bg_hover",        "#3a3d45"},
        {"bg_pressed",      "#42454d"},
        {"bg_input",        "#22242c"},

        // 文本色系
        {"text_primary",    "#e0e0e0"},
        {"text_secondary",  "#a0a0a0"},
        {"text_disabled",   "#5a5a5a"},
        {"text_accent",     "#4a9eff"},

        // 边框色系
        {"border_default",  "#3a3d45"},
        {"border_focus",    "#4a9eff"},
        {"border_hover",    "#4a4d55"},

        // 强调色
        {"accent_primary",  "#4a9eff"},
        {"accent_success",  "#4ade80"},
        {"accent_warning",  "#f59e0b"},
        {"accent_error",    "#ef4444"},
        {"accent_info",     "#3b82f6"},

        // 滚动条
        {"scrollbar_bg",    "#282a33"},
        {"scrollbar_handle","#4a4d55"},
        {"scrollbar_hover", "#5a5d65"},

        // 选项卡
        {"tab_active",      "#4a9eff"},
        {"tab_inactive",    "transparent"},

        // 终端
        {"terminal_bg",     "#1a1b22"},
        {"terminal_fg",     "#c8c8c8"},
        {"terminal_cursor", "#4a9eff"},
        {"terminal_sel",    "#264f78"},

        // 图表
        {"chart_bg",        "#1e1f26"},
        {"chart_grid",      "#2a2d35"},
        {"chart_axis",      "#5a5a5a"},
    };
}

// ============================================================
// 亮色主题语义色
// ============================================================

/** @brief 返回亮色主题的语义色映射表 @return 亮色主题语义色键值对 */
QssThemeGenerator::ColorMap QssThemeGenerator::lightColors()
{
    return {
        {"bg_primary",      "#ffffff"},
        {"bg_secondary",    "#f5f5f5"},
        {"bg_tertiary",     "#ebebeb"},
        {"bg_elevated",     "#ffffff"},
        {"bg_hover",        "#e8e8e8"},
        {"bg_pressed",      "#d8d8d8"},
        {"bg_input",        "#ffffff"},

        {"text_primary",    "#1a1a1a"},
        {"text_secondary",  "#666666"},
        {"text_disabled",   "#b0b0b0"},
        {"text_accent",     "#1a73e8"},

        {"border_default",  "#d0d0d0"},
        {"border_focus",    "#1a73e8"},
        {"border_hover",    "#b0b0b0"},

        {"accent_primary",  "#1a73e8"},
        {"accent_success",  "#0d9f4f"},
        {"accent_warning",  "#e09100"},
        {"accent_error",    "#d93025"},
        {"accent_info",     "#1a73e8"},

        {"scrollbar_bg",    "#f0f0f0"},
        {"scrollbar_handle","#c0c0c0"},
        {"scrollbar_hover", "#a0a0a0"},

        {"tab_active",      "#1a73e8"},
        {"tab_inactive",    "transparent"},

        {"terminal_bg",     "#fafafa"},
        {"terminal_fg",     "#333333"},
        {"terminal_cursor", "#1a73e8"},
        {"terminal_sel",    "#cce0f5"},

        {"chart_bg",        "#ffffff"},
        {"chart_grid",      "#f0f0f0"},
        {"chart_axis",      "#999999"},
    };
}

// ============================================================
// 高对比度主题语义色
// ============================================================

/** @brief 返回高对比度主题的语义色映射表 @return 高对比度主题语义色键值对 */
QssThemeGenerator::ColorMap QssThemeGenerator::highContrastColors()
{
    return {
        {"bg_primary",      "#000000"},
        {"bg_secondary",    "#1a1a1a"},
        {"bg_tertiary",     "#2a2a2a"},
        {"bg_elevated",     "#333333"},
        {"bg_hover",        "#444444"},
        {"bg_pressed",      "#555555"},
        {"bg_input",        "#111111"},

        {"text_primary",    "#ffffff"},
        {"text_secondary",  "#cccccc"},
        {"text_disabled",   "#777777"},
        {"text_accent",     "#00ccff"},

        {"border_default",  "#666666"},
        {"border_focus",    "#00ccff"},
        {"border_hover",    "#888888"},

        {"accent_primary",  "#00ccff"},
        {"accent_success",  "#00ff88"},
        {"accent_warning",  "#ffcc00"},
        {"accent_error",    "#ff4444"},
        {"accent_info",     "#00ccff"},

        {"scrollbar_bg",    "#1a1a1a"},
        {"scrollbar_handle","#888888"},
        {"scrollbar_hover", "#aaaaaa"},

        {"tab_active",      "#00ccff"},
        {"tab_inactive",    "transparent"},

        {"terminal_bg",     "#000000"},
        {"terminal_fg",     "#ffffff"},
        {"terminal_cursor", "#00ccff"},
        {"terminal_sel",    "#003355"},

        {"chart_bg",        "#000000"},
        {"chart_grid",      "#333333"},
        {"chart_axis",      "#cccccc"},
    };
}

// ============================================================
// 色彩选择
// ============================================================

/** @brief 根据主题类型返回对应的语义色映射表 @param type 主题类型枚举 @return 语义色键值对映射 */
QssThemeGenerator::ColorMap QssThemeGenerator::semanticColors(ThemeType type)
{
    switch (type) {
    case ThemeType::Dark:         return darkColors();
    case ThemeType::Light:        return lightColors();
    case ThemeType::HighContrast: return highContrastColors();
    }
    return darkColors();
}

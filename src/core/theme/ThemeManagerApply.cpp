/**
 * @file ThemeManagerApply.cpp
 * @brief ThemeManager 主题应用/查询/动画方法实现
 *
 * 本文件从 ThemeManager.cpp 拆分而来，包含:
 *   1. 主题查询接口（availableThemes / currentTheme / isSystemDarkMode）
 *   2. 主题持久化与恢复（saveTheme / loadSavedTheme / loadSystemTheme）
 *   3. 样式表应用方法（applyStylesheetWithAnimation / applyStylesheetDirect）
 *   4. 主题切换动画设置（setTransitionWidget）
 *
 * 运行时统计接口见 ThemeManagerStats.cpp。
 * 语义色板相关方法见 ThemeManagerColor.cpp
 * 主题加载入口（loadTheme / loadThemeFromFile）见 ThemeManager.cpp
 */

#include "core/theme/ThemeManager.h"
#include "utils/settings/SettingsManager.h"
#include "shared/AnimationConstants.h"

#include <QApplication>
#include <QDebug>
#include <QWidget>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QAbstractAnimation>

#ifdef Q_OS_WIN
#include <QSettings>
#endif

// ==================== 主题查询接口 ====================

/** @brief 获取所有可用主题名称列表 @return 主题名QStringList */
QStringList ThemeManager::availableThemes() const
{
    return m_themes.keys();
}

/** @brief 获取当前激活主题名称 @return 主题名 */
QString ThemeManager::currentTheme() const
{
    return m_currentTheme;
}

/** @brief 检测Windows系统当前是否为暗色模式(读取注册表AppsUseLightTheme) @return true系统为暗色模式 */
bool ThemeManager::isSystemDarkMode() const
{
    ++m_totalSystemThemeChecks;  ///< 累计系统暗色模式检测次数
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

// ==================== 主题持久化与恢复 ====================

/** @brief 根据系统主题自动选择并加载对应主题(暗色→dark_terminal/亮色→light) @return 实际加载的主题名称 */
QString ThemeManager::loadSystemTheme()
{
    QString themeName = isSystemDarkMode() ? "dark_terminal" : "light";
    ++m_totalThemeReloads;  ///< 统计: 系统主题重新加载
    loadTheme(themeName);
    return themeName;
}

/** @brief 持久化当前主题到SettingsManager(写入theme/name配置项) */
void ThemeManager::saveTheme() const
{
    if (!m_currentTheme.isEmpty()) {
        SettingsManager::instance().saveTheme(m_currentTheme);
    }
}

/** @brief 从SettingsManager加载上次保存的主题(无记录时回退到系统主题) @return true成功加载已保存的主题 */
bool ThemeManager::loadSavedTheme()
{
    auto& settings = SettingsManager::instance();
    QString savedTheme = settings.loadTheme();

    ++m_totalThemeReloads;  ///< 统计: 保存主题重新加载

    // loadTheme() 返回默认值 App::DEFAULT_THEME 表示无保存记录
    // 尝试用保存的主题名加载，若失败则回退到系统主题
    if (m_themes.contains(savedTheme)) {
        return loadTheme(savedTheme);
    }

    // 无保存记录或主题不存在，使用系统主题
    loadSystemTheme();
    return false;
}

// ==================== 主题切换动画 ====================

/** @brief 设置主题切换动画目标widget(通常是centralWidget) @param widget 目标widget */
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

/** @brief 带淡入淡出动画应用样式表(300ms InOutCubic) @param qss 新的QSS样式表内容 */
void ThemeManager::applyStylesheetWithAnimation(const QString& qss)
{
    if (!m_transitionWidget || !m_opacityEffect) {
        applyStylesheetDirect(qss);
        return;
    }

    // 淡出动画: InOutCubic
    QPropertyAnimation* fadeOut = new QPropertyAnimation(m_opacityEffect, "opacity");
    fadeOut->setDuration(Animations::kThemeFadeMs);
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.0);
    fadeOut->setEasingCurve(QEasingCurve::InOutCubic);

    // 清理上一次未完成的淡入动画(防止快速切换主题时fadeIn泄漏)
    // 查找ThemeManager的所有QPropertyAnimation子对象并停止/删除
    const auto fadeChildren = findChildren<QPropertyAnimation*>();
    for (auto* oldAnim : fadeChildren) {
        if (oldAnim->state() == QAbstractAnimation::Running) oldAnim->stop();
        delete oldAnim;
    }

    // 淡入动画: InOutCubic
    QPropertyAnimation* fadeIn = new QPropertyAnimation(m_opacityEffect, "opacity");
    fadeIn->setParent(this);  // 父对象设为ThemeManager，生命周期独立于fadeOut
    fadeIn->setDuration(Animations::kThemeFadeMs);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    fadeIn->setEasingCurve(QEasingCurve::InOutCubic);

    // 淡出完成后: 应用新样式表并启动淡入
    connect(fadeOut, &QPropertyAnimation::finished, this, [this, qss, fadeIn]() {
        qApp->setStyleSheet(qss);
        // 统计：累计样式表应用计数（带动画）
        ++m_totalStyleApplications;
        fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
    });

    fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
}

/** @brief 不带动画直接应用样式表(首次加载使用) @param qss QSS样式表内容 */
void ThemeManager::applyStylesheetDirect(const QString& qss)
{
    qApp->setStyleSheet(qss);
    // 统计：累计样式表应用计数（直接应用）
    ++m_totalStyleApplications;
}

// 运行时统计接口见 ThemeManagerStats.cpp

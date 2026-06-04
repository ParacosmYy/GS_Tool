/**
 * @file BackgroundWidget.cpp
 * @brief 背景层控件实现 - 属性设置/获取、背景图管理、主题色更新
 *
 * 绘制与动画方法见 BackgroundWidgetPaint.cpp:
 *   paintEvent / mousePressEvent / resizeEvent
 *   generateBlurred / regenerateScaledBackground / advanceRipples
 */

#include "core/background/BackgroundWidget.h"
#include "core/theme/ThemeManager.h"
#include "shared/TimerConstants.h"

/** @brief 构造背景控件，初始化涟漪动画定时器(16ms间隔约60fps)并加载默认背景图 @param parent 父控件 */
BackgroundWidget::BackgroundWidget(QWidget* parent)
    : QWidget(parent)
    , m_rippleTimer(new QTimer(this))
{
    setObjectName("backgroundWidget");

    m_rippleTimer->setInterval(Timers::kRippleFrameMs); // ~60fps
    connect(m_rippleTimer, &QTimer::timeout, this, &BackgroundWidget::advanceRipples);

    // 从 ThemeManager 初始化主题色（遮罩用 BgPrimary，涟漪用 Accent）
    updateThemeColors();
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &BackgroundWidget::updateThemeColors);

    // 加载默认背景图
    setBackgroundImage(":/backgrounds/default_bg.png");
}

/** @brief 设置背景图片，加载原始图并生成模糊版和缩放缓存后触发重绘 @param resourcePath Qt资源路径或本地文件系统路径 */
void BackgroundWidget::setBackgroundImage(const QString& resourcePath)
{
    m_originalImage.load(resourcePath);
    m_currentImagePath = resourcePath;
    m_blurredImage = generateBlurred(m_originalImage, m_blurRadius);
    regenerateScaledBackground();
    ++m_totalImageLoads;
    update();
}

/** @brief 设置磨砂玻璃模糊半径，值变化时重新生成模糊图和缩放缓存 @param radius 模糊半径(0~30) */
void BackgroundWidget::setBlurRadius(qreal radius)
{
    if (qFuzzyCompare(m_blurRadius, radius)) return;
    m_blurRadius = qBound(0.0, radius, 30.0);
    m_blurredImage = generateBlurred(m_originalImage, m_blurRadius);
    regenerateScaledBackground();
    ++m_totalEffectChanges;
    emit blurChanged(m_blurRadius);
    update();
}

/** @brief 获取当前模糊半径 @return 模糊半径值 */
qreal BackgroundWidget::blurRadius() const
{
    return m_blurRadius;
}

/** @brief 设置背景透明度 @param opacity 透明度(0~1) */
void BackgroundWidget::setBgOpacity(qreal opacity)
{
    if (qFuzzyCompare(m_bgOpacity, opacity)) return;
    m_bgOpacity = qBound(0.0, opacity, 1.0);
    ++m_totalEffectChanges;
    emit opacityChanged(m_bgOpacity);
    update();
}

/** @brief 获取当前背景透明度 @return 透明度值 */
qreal BackgroundWidget::bgOpacity() const
{
    return m_bgOpacity;
}

/** @brief 设置涟漪特效开关，禁用时清除所有活跃涟漪并停止定时器 @param enabled true=启用, false=禁用 */
void BackgroundWidget::setRippleEnabled(bool enabled)
{
    m_rippleEnabled = enabled;
    ++m_totalEffectChanges;
    if (!enabled) {
        m_ripples.clear();
        m_rippleTimer->stop();
        update();
    }
}

/** @brief 获取涟漪特效是否启用 @return true=启用 */
bool BackgroundWidget::rippleEnabled() const
{
    return m_rippleEnabled;
}

/** @brief 设置涟漪颜色，用于主题适配 @param color 涟漪颜色 */
void BackgroundWidget::setRippleColor(const QColor& color)
{
    m_rippleColor = color;
}

/** @brief 获取当前涟漪颜色 @return 涟漪颜色 */
QColor BackgroundWidget::rippleColor() const
{
    return m_rippleColor;
}

/** @brief 设置半透明遮罩颜色 @param color 遮罩颜色 */
void BackgroundWidget::setOverlayColor(const QColor& color)
{
    m_overlayColor = color;
    update();
}

/** @brief 获取当前遮罩颜色 @return 遮罩颜色 */
QColor BackgroundWidget::overlayColor() const
{
    return m_overlayColor;
}

/** @brief 设置遮罩透明度 @param opacity 遮罩透明度(0~1) */
void BackgroundWidget::setOverlayOpacity(qreal opacity)
{
    m_overlayOpacity = qBound(0.0, opacity, 1.0);
    update();
}

/** @brief 获取当前遮罩透明度 @return 遮罩透明度值 */
qreal BackgroundWidget::overlayOpacity() const
{
    return m_overlayOpacity;
}

/** @brief 设置模糊迭代次数，次数越多磨砂效果越自然但耗时越长 @param iterations 迭代次数(2~10) */
void BackgroundWidget::setBlurIterations(int iterations)
{
    m_blurIterations = qBound(2, iterations, 10);
    m_blurredImage = generateBlurred(m_originalImage, m_blurRadius);
    regenerateScaledBackground();
    ++m_totalEffectChanges;
    update();
}

/** @brief 获取当前模糊迭代次数 @return 迭代次数 */
int BackgroundWidget::blurIterations() const
{
    return m_blurIterations;
}

/** @brief 恢复为资源中的默认背景图 */
void BackgroundWidget::resetToDefault()
{
    setBackgroundImage(":/backgrounds/default_bg.png");
    emit backgroundImageChanged(m_currentImagePath);
}

/** @brief 获取当前背景图片路径 @return 图片路径字符串 */
QString BackgroundWidget::currentImagePath() const
{
    return m_currentImagePath;
}

// ---- 绘制与动画方法见 BackgroundWidgetPaint.cpp ----
// paintEvent / mousePressEvent / resizeEvent
// generateBlurred / regenerateScaledBackground / advanceRipples

/** @brief 从ThemeManager更新主题色(遮罩色用BgPrimary，涟漪色用Accent)并触发重绘 */
void BackgroundWidget::updateThemeColors()
{
    ++m_totalThemeUpdates;
    auto& tm = ThemeManager::instance();
    m_overlayColor = tm.color(ThemeManager::SemanticColor::BgPrimary);
    m_rippleColor = tm.color(ThemeManager::SemanticColor::Accent);
    update();
}

// ============================================================================
// 统计重置
// ============================================================================

/** @brief 重置所有背景统计计数器为零 */
void BackgroundWidget::resetBackgroundStatistics()
{
    m_totalImageLoads = 0;
    m_totalEffectChanges = 0;
    m_totalRipples = 0;
    m_totalThemeUpdates = 0;
    m_totalPaints = 0;
    m_totalResizes = 0;
}

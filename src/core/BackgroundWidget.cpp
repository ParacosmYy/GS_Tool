/**
 * @file BackgroundWidget.cpp
 * @brief 背景层控件实现 - 自定义背景图、磨砂玻璃模糊、透明度调节、点击涟漪特效
 *
 * 性能关键: paintEvent 不做任何耗时的图片缩放操作，
 * 所有缩放在 resize 时预缓存到 m_scaledBlurredImage。
 */

#include "BackgroundWidget.h"
#include "ThemeManager.h"
#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QRandomGenerator>
#include <QtMath>

/**
 * @brief 构造背景控件
 * 初始化涟漪动画定时器（16ms 间隔 ≈ 60fps）并加载默认背景图
 */
BackgroundWidget::BackgroundWidget(QWidget* parent)
    : QWidget(parent)
    , m_rippleTimer(new QTimer(this))
{
    m_rippleTimer->setInterval(16); // ~60fps
    connect(m_rippleTimer, &QTimer::timeout, this, &BackgroundWidget::advanceRipples);

    // 从 ThemeManager 初始化主题色（遮罩用 BgPrimary，涟漪用 Accent）
    updateThemeColors();
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &BackgroundWidget::updateThemeColors);

    // 加载默认背景图
    setBackgroundImage(":/backgrounds/default_bg.png");
}

/**
 * @brief 设置背景图片
 * 加载原始图 → 生成模糊版 → 缓存缩放版 → 触发重绘
 * @param resourcePath Qt 资源路径或本地文件系统路径
 */
void BackgroundWidget::setBackgroundImage(const QString& resourcePath)
{
    m_originalImage.load(resourcePath);
    m_currentImagePath = resourcePath;
    m_blurredImage = generateBlurred(m_originalImage, m_blurRadius);
    regenerateScaledBackground();
    update();
}

/**
 * @brief 设置磨砂玻璃模糊半径
 * 值变化时重新生成模糊图和缩放缓存
 * @param radius 模糊半径 (0~30)
 */
void BackgroundWidget::setBlurRadius(qreal radius)
{
    if (qFuzzyCompare(m_blurRadius, radius)) return;
    m_blurRadius = qBound(0.0, radius, 30.0);
    m_blurredImage = generateBlurred(m_originalImage, m_blurRadius);
    regenerateScaledBackground();
    emit blurChanged(m_blurRadius);
    update();
}

/** @brief 获取当前模糊半径 */
qreal BackgroundWidget::blurRadius() const
{
    return m_blurRadius;
}

/**
 * @brief 设置背景透明度
 * @param opacity 透明度 (0~1)
 */
void BackgroundWidget::setBgOpacity(qreal opacity)
{
    if (qFuzzyCompare(m_bgOpacity, opacity)) return;
    m_bgOpacity = qBound(0.0, opacity, 1.0);
    emit opacityChanged(m_bgOpacity);
    update();
}

/** @brief 获取当前背景透明度 */
qreal BackgroundWidget::bgOpacity() const
{
    return m_bgOpacity;
}

/**
 * @brief 设置涟漪特效开关
 * 禁用时清除所有活跃涟漪并停止定时器
 * @param enabled true=启用, false=禁用
 */
void BackgroundWidget::setRippleEnabled(bool enabled)
{
    m_rippleEnabled = enabled;
    if (!enabled) {
        m_ripples.clear();
        m_rippleTimer->stop();
        update();
    }
}

/** @brief 获取涟漪特效是否启用 */
bool BackgroundWidget::rippleEnabled() const
{
    return m_rippleEnabled;
}

/** @brief 设置涟漪颜色（用于主题适配） */
void BackgroundWidget::setRippleColor(const QColor& color)
{
    m_rippleColor = color;
}

/** @brief 获取当前涟漪颜色 */
QColor BackgroundWidget::rippleColor() const
{
    return m_rippleColor;
}

/** @brief 设置半透明遮罩颜色 */
void BackgroundWidget::setOverlayColor(const QColor& color)
{
    m_overlayColor = color;
    update();
}

/** @brief 获取当前遮罩颜色 */
QColor BackgroundWidget::overlayColor() const
{
    return m_overlayColor;
}

/** @brief 设置遮罩透明度 */
void BackgroundWidget::setOverlayOpacity(qreal opacity)
{
    m_overlayOpacity = qBound(0.0, opacity, 1.0);
    update();
}

/** @brief 获取当前遮罩透明度 */
qreal BackgroundWidget::overlayOpacity() const
{
    return m_overlayOpacity;
}

/**
 * @brief 设置模糊迭代次数
 * 次数越多磨砂效果越自然，但生成耗时也越长
 * @param iterations 迭代次数 (2~10)
 */
void BackgroundWidget::setBlurIterations(int iterations)
{
    m_blurIterations = qBound(2, iterations, 10);
    m_blurredImage = generateBlurred(m_originalImage, m_blurRadius);
    regenerateScaledBackground();
    update();
}

/** @brief 获取当前模糊迭代次数 */
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

/** @brief 获取当前背景图片路径 */
QString BackgroundWidget::currentImagePath() const
{
    return m_currentImagePath;
}

/**
 * @brief 自绘事件 - 按四层渲染背景
 *
 * 第1层: 纯黑底色（兜底，防止透明区域）
 * 第2层: 模糊背景图（Cover 模式居中裁剪覆盖，使用预缓存避免每帧缩放）
 * 第3层: 半透明遮罩（暗色遮罩提升文字可读性）
 * 第4层: 涟漪特效（跟随主题 accent 色的径向渐变圆环）
 */
void BackgroundWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 第1层：纯黑底色
    painter.fillRect(rect(), Qt::black);

    // 第2层：背景图（模糊版），居中裁剪覆盖，使用预缓存避免每帧缩放
    if (!m_scaledBlurredImage.isNull()) {
        painter.setOpacity(m_bgOpacity);
        painter.drawPixmap(m_scaledOffset, m_scaledBlurredImage);
        painter.setOpacity(1.0);
    }

    // 第3层：半透明遮罩层（暗色遮罩让文字更易读）
    if (m_overlayOpacity > 0.0) {
        QColor overlay = m_overlayColor;
        overlay.setAlphaF(m_overlayOpacity);
        painter.fillRect(rect(), overlay);
    }

    // 第4层：涟漪特效（使用主题accent色）
    if (!m_ripples.isEmpty()) {
        for (const auto& ripple : m_ripples) {
            QRadialGradient gradient(ripple.center, ripple.currentRadius);
            QColor inner = m_rippleColor;
            inner.setAlpha(int(ripple.opacity * 80));
            QColor mid = m_rippleColor;
            mid.setAlpha(int(ripple.opacity * 30));
            QColor outer = m_rippleColor;
            outer.setAlpha(0);
            gradient.setColorAt(0, inner);
            gradient.setColorAt(0.5, mid);
            gradient.setColorAt(1, outer);
            painter.setBrush(gradient);
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(ripple.center, ripple.currentRadius, ripple.currentRadius);
        }
    }
}

/**
 * @brief 鼠标按下事件 - 创建涟漪动画
 * 在点击位置生成一个新的涟漪（随机最大半径 120~180px），启动动画定时器
 */
void BackgroundWidget::mousePressEvent(QMouseEvent* event)
{
    if (!m_rippleEnabled) {
        QWidget::mousePressEvent(event);
        return;
    }

    Ripple ripple;
    ripple.center = event->pos();
    ripple.currentRadius = 1.0;
    // 随机化最大半径，避免所有涟漪看起来完全相同
    ripple.maxRadius = 120.0 + QRandomGenerator::global()->bounded(60);
    ripple.opacity = 0.6;
    m_ripples.append(ripple);

    // 如果定时器未运行则启动（涟漪结束后自动停止）
    if (!m_rippleTimer->isActive()) {
        m_rippleTimer->start();
    }

    update();
}

/**
 * @brief 窗口尺寸变化事件
 * 重新缓存缩放后的背景图，保证 paintEvent 不做缩放
 */
void BackgroundWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    regenerateScaledBackground();
}

/**
 * @brief 从原始图片生成模糊版本（缩放法快速近似高斯模糊）
 *
 * 算法: 多次缩放（缩小→逐步放大→最终恢复原尺寸），
 * 利用 Qt::SmoothTransformation 的双线性插值近似高斯模糊。
 * 迭代次数由 m_blurIterations 控制（2~10），次数越多效果越自然。
 * @param src 原始像素图
 * @param radius 模糊半径
 * @return 模糊后的像素图
 */
QPixmap BackgroundWidget::generateBlurred(const QPixmap& src, qreal radius) const
{
    if (src.isNull() || radius <= 0) return src;

    qreal baseFactor = 1.0 / (1.0 + radius * 0.15);

    QPixmap result = src;

    for (int i = 0; i < m_blurIterations; ++i) {
        // 第一次缩小最多，后续逐步放大回原尺寸
        qreal stepFactor = baseFactor + (1.0 - baseFactor) * (qreal(i) / qreal(m_blurIterations));
        QSize targetSize = src.size() * stepFactor;
        targetSize = targetSize.expandedTo(QSize(1, 1));  // 防止尺寸为零
        result = result.scaled(targetSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    // 最终放大回原尺寸
    result = result.scaled(src.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    return result;
}

/**
 * @brief 缓存当前窗口尺寸下的缩放背景图
 *
 * Cover 模式: 等比缩放填满整个 widget，裁剪多余部分。
 * 在 resize/blur/image 变化时调用，避免 paintEvent 每帧都做缩放。
 */
void BackgroundWidget::regenerateScaledBackground()
{
    if (m_blurredImage.isNull()) {
        m_scaledBlurredImage = QPixmap();
        m_scaledOffset = QPoint();
        return;
    }

    // Cover模式: 等比缩放填满整个widget，裁剪多余部分
    QSize widgetSize = size();
    if (widgetSize.isEmpty()) return;

    QSize imgSize = m_blurredImage.size();
    qreal scaleX = (qreal)widgetSize.width() / imgSize.width();
    qreal scaleY = (qreal)widgetSize.height() / imgSize.height();
    // 取较大缩放比，确保填满（多余部分被裁剪）
    qreal scale = qMax(scaleX, scaleY);

    QSize scaledSize = imgSize * scale;
    // 计算居中偏移量（负值表示裁剪区域）
    m_scaledOffset = QPoint(
        (widgetSize.width() - scaledSize.width()) / 2,
        (widgetSize.height() - scaledSize.height()) / 2
    );

    // 预先缩放并缓存，paintEvent 直接 drawPixmap 不再做 scaled()
    m_scaledBlurredImage = m_blurredImage.scaled(
        scaledSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

/**
 * @brief 涟漪动画帧更新
 * 每帧: 扩大半径(+4px) + 降低透明度(-0.025)，到达最大半径或透明度归零时移除
 * 所有涟漪完成后自动停止定时器以节省 CPU
 */
void BackgroundWidget::advanceRipples()
{
    // 逆序遍历，安全删除已完成涟漪
    for (int i = m_ripples.size() - 1; i >= 0; --i) {
        auto& r = m_ripples[i];
        r.currentRadius += 4.0;
        r.opacity -= 0.025;

        if (r.opacity <= 0 || r.currentRadius >= r.maxRadius) {
            m_ripples.removeAt(i);
        }
    }

    // 所有涟漪完成后停止定时器，避免空转浪费 CPU
    if (m_ripples.isEmpty()) {
        m_rippleTimer->stop();
    }

    update();
}

/**
 * @brief 从 ThemeManager 更新主题色并触发重绘
 * 遮罩颜色使用 BgPrimary（深色背景），涟漪颜色使用 Accent（强调色）
 * 主题切换时自动调用，无需手动更新
 */
void BackgroundWidget::updateThemeColors()
{
    auto& tm = ThemeManager::instance();
    m_overlayColor = tm.color(ThemeManager::SemanticColor::BgPrimary);
    m_rippleColor = tm.color(ThemeManager::SemanticColor::Accent);
    update();
}

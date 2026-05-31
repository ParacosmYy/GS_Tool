#include "BackgroundWidget.h"
#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QRandomGenerator>
#include <QtMath>

BackgroundWidget::BackgroundWidget(QWidget* parent)
    : QWidget(parent)
    , m_rippleTimer(new QTimer(this))
{
    m_rippleTimer->setInterval(16); // ~60fps
    connect(m_rippleTimer, &QTimer::timeout, this, &BackgroundWidget::advanceRipples);

    // 加载默认背景图
    setBackgroundImage(":/backgrounds/default_bg.png");
}

void BackgroundWidget::setBackgroundImage(const QString& resourcePath)
{
    m_originalImage.load(resourcePath);
    m_currentImagePath = resourcePath;
    m_blurredImage = generateBlurred(m_originalImage, m_blurRadius);
    regenerateScaledBackground();
    update();
}

void BackgroundWidget::setBlurRadius(qreal radius)
{
    if (qFuzzyCompare(m_blurRadius, radius)) return;
    m_blurRadius = qBound(0.0, radius, 30.0);
    m_blurredImage = generateBlurred(m_originalImage, m_blurRadius);
    regenerateScaledBackground();
    emit blurChanged(m_blurRadius);
    update();
}

qreal BackgroundWidget::blurRadius() const
{
    return m_blurRadius;
}

void BackgroundWidget::setBgOpacity(qreal opacity)
{
    if (qFuzzyCompare(m_bgOpacity, opacity)) return;
    m_bgOpacity = qBound(0.0, opacity, 1.0);
    emit opacityChanged(m_bgOpacity);
    update();
}

qreal BackgroundWidget::bgOpacity() const
{
    return m_bgOpacity;
}

void BackgroundWidget::setRippleEnabled(bool enabled)
{
    m_rippleEnabled = enabled;
    if (!enabled) {
        m_ripples.clear();
        m_rippleTimer->stop();
        update();
    }
}

bool BackgroundWidget::rippleEnabled() const
{
    return m_rippleEnabled;
}

void BackgroundWidget::setRippleColor(const QColor& color)
{
    m_rippleColor = color;
}

QColor BackgroundWidget::rippleColor() const
{
    return m_rippleColor;
}

void BackgroundWidget::setOverlayColor(const QColor& color)
{
    m_overlayColor = color;
    update();
}

QColor BackgroundWidget::overlayColor() const
{
    return m_overlayColor;
}

void BackgroundWidget::setOverlayOpacity(qreal opacity)
{
    m_overlayOpacity = qBound(0.0, opacity, 1.0);
    update();
}

qreal BackgroundWidget::overlayOpacity() const
{
    return m_overlayOpacity;
}

void BackgroundWidget::setBlurIterations(int iterations)
{
    m_blurIterations = qBound(2, iterations, 10);
    m_blurredImage = generateBlurred(m_originalImage, m_blurRadius);
    regenerateScaledBackground();
    update();
}

int BackgroundWidget::blurIterations() const
{
    return m_blurIterations;
}

void BackgroundWidget::resetToDefault()
{
    setBackgroundImage(":/backgrounds/default_bg.png");
    emit backgroundImageChanged(m_currentImagePath);
}

QString BackgroundWidget::currentImagePath() const
{
    return m_currentImagePath;
}

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

void BackgroundWidget::mousePressEvent(QMouseEvent* event)
{
    if (!m_rippleEnabled) {
        QWidget::mousePressEvent(event);
        return;
    }

    Ripple ripple;
    ripple.center = event->pos();
    ripple.currentRadius = 1.0;
    ripple.maxRadius = 120.0 + QRandomGenerator::global()->bounded(60);
    ripple.opacity = 0.6;
    m_ripples.append(ripple);

    if (!m_rippleTimer->isActive()) {
        m_rippleTimer->start();
    }

    update();
}

void BackgroundWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    regenerateScaledBackground();
}

QPixmap BackgroundWidget::generateBlurred(const QPixmap& src, qreal radius) const
{
    if (src.isNull() || radius <= 0) return src;

    // 快速模糊: 多次缩放增强磨砂效果
    // 每次迭代的缩放系数逐步增大，产生更自然的高斯模糊近似
    qreal baseFactor = 1.0 / (1.0 + radius * 0.15);

    QPixmap result = src;

    for (int i = 0; i < m_blurIterations; ++i) {
        // 第一次缩小最多，后续逐步放大回原尺寸
        qreal stepFactor = baseFactor + (1.0 - baseFactor) * (qreal(i) / qreal(m_blurIterations));
        QSize targetSize = src.size() * stepFactor;
        targetSize = targetSize.expandedTo(QSize(1, 1));
        result = result.scaled(targetSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    // 最终放大回原尺寸
    result = result.scaled(src.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    return result;
}

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
    qreal scale = qMax(scaleX, scaleY);

    QSize scaledSize = imgSize * scale;
    m_scaledOffset = QPoint(
        (widgetSize.width() - scaledSize.width()) / 2,
        (widgetSize.height() - scaledSize.height()) / 2
    );

    // 预先缩放并缓存，paintEvent直接drawPixmap不再做scaled()
    m_scaledBlurredImage = m_blurredImage.scaled(
        scaledSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

void BackgroundWidget::advanceRipples()
{
    for (int i = m_ripples.size() - 1; i >= 0; --i) {
        auto& r = m_ripples[i];
        r.currentRadius += 4.0;
        r.opacity -= 0.025;

        if (r.opacity <= 0 || r.currentRadius >= r.maxRadius) {
            m_ripples.removeAt(i);
        }
    }

    if (m_ripples.isEmpty()) {
        m_rippleTimer->stop();
    }

    update();
}

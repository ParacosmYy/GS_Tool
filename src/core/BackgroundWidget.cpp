#include "BackgroundWidget.h"
#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
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
    m_blurredImage = generateBlurred(m_originalImage, m_blurRadius);
    update();
}

void BackgroundWidget::setBlurRadius(qreal radius)
{
    if (qFuzzyCompare(m_blurRadius, radius)) return;
    m_blurRadius = qBound(0.0, radius, 30.0);
    m_blurredImage = generateBlurred(m_originalImage, m_blurRadius);
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

void BackgroundWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 1. 绘制纯黑底色
    painter.fillRect(rect(), Qt::black);

    // 2. 绘制背景图（模糊版），居中裁剪覆盖
    if (!m_blurredImage.isNull()) {
        painter.setOpacity(m_bgOpacity);

        // Cover模式: 等比缩放填满整个widget，裁剪多余部分
        QSize widgetSize = size();
        QSize imgSize = m_blurredImage.size();
        qreal scaleX = (qreal)widgetSize.width() / imgSize.width();
        qreal scaleY = (qreal)widgetSize.height() / imgSize.height();
        qreal scale = qMax(scaleX, scaleY);

        QSize scaledSize = imgSize * scale;
        QPoint offset(
            (widgetSize.width() - scaledSize.width()) / 2,
            (widgetSize.height() - scaledSize.height()) / 2
        );

        painter.drawPixmap(offset, m_blurredImage.scaled(
            scaledSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        painter.setOpacity(1.0);
    }

    // 3. 绘制涟漪特效
    if (!m_ripples.isEmpty()) {
        for (const auto& ripple : m_ripples) {
            QRadialGradient gradient(ripple.center, ripple.currentRadius);
            gradient.setColorAt(0, QColor(255, 255, 255, int(ripple.opacity * 80)));
            gradient.setColorAt(0.5, QColor(255, 255, 255, int(ripple.opacity * 30)));
            gradient.setColorAt(1, QColor(255, 255, 255, 0));
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

QPixmap BackgroundWidget::generateBlurred(const QPixmap& src, qreal radius) const
{
    if (src.isNull() || radius <= 0) return src;

    // 快速模糊: 先缩小再放大，利用SmoothTransformation的双线性插值近似高斯模糊
    qreal factor = 1.0 / (1.0 + radius * 0.15);
    QSize scaled = src.size() * factor;

    // 多次缩放增强模糊效果
    QPixmap result = src.scaled(scaled, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    // 二次模糊
    QSize mid = src.size() * (factor * 0.7);
    result = result.scaled(mid, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    // 放回原尺寸
    result = result.scaled(src.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    return result;
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

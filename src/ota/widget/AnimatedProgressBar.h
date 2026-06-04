/**
 * @file AnimatedProgressBar.h
 * @brief 带shimmer流动效果的进度条 -- OTA传输视觉反馈
 *
 * header-only实现，QPropertyAnimation驱动shimmerOffset属性，颜色从ThemeManager获取
 */
#ifndef ANIMATEDPROGRESSBAR_H
#define ANIMATEDPROGRESSBAR_H

#include <QProgressBar>
#include <QPropertyAnimation>
#include <QPainter>
#include <QLinearGradient>
#include <QStyleOptionProgressBar>
#include "core/theme/ThemeManager.h"

/** @brief 带shimmer流动效果的进度条控件。传输进行时chunk上显示水平移动亮光带，chunkColor Q_PROPERTY支持颜色插值动画 */
class AnimatedProgressBar : public QProgressBar {
    Q_OBJECT
    Q_PROPERTY(qreal shimmerOffset READ shimmerOffset WRITE setShimmerOffset NOTIFY shimmerOffsetChanged)
    Q_PROPERTY(QColor chunkColor READ chunkColor WRITE setChunkColor NOTIFY chunkColorChanged)

public:
    explicit AnimatedProgressBar(QWidget* parent = nullptr)
        : QProgressBar(parent), m_shimmerOffset(0.0), m_shimmerAnim(nullptr), m_customChunkColor(false)
    { setObjectName("animatedProgressBar"); }
    qreal shimmerOffset() const { return m_shimmerOffset; } ///< 获取shimmer偏移量[0.0, 1.0]
    /** @brief 设置shimmer偏移量，由QPropertyAnimation驱动 @param offset 目标偏移量[0.0, 1.0] */
    void setShimmerOffset(qreal offset) {
        if (!qFuzzyCompare(m_shimmerOffset, offset)) { m_shimmerOffset = offset; emit shimmerOffsetChanged(); update(); }
    }

signals:
    void shimmerOffsetChanged();             ///< shimmerOffset属性变更通知
    void chunkColorChanged();                ///< chunkColor属性变更通知

public:
    QColor chunkColor() const { return m_chunkColor; } ///< 获取chunk自定义颜色
    /** @brief 设置chunk自定义颜色，由QPropertyAnimation驱动 @param color 目标颜色 */
    void setChunkColor(const QColor& color) { ++m_totalChunkColorChanges; m_chunkColor = color; m_customChunkColor = true; emit chunkColorChanged(); update(); }
    void resetChunkColor() { m_customChunkColor = false; m_chunkColor = QColor(); ++m_totalColorResets; emit chunkColorChanged(); update(); } ///< 恢复QSS主题默认chunk颜色
    void setValue(int value) { ++m_totalValueUpdates; QProgressBar::setValue(value); } ///< 重写setValue，增加统计计数
    quint64 totalAnimations() const { return m_totalAnimations; }     ///< 获取动画播放总次数
    quint64 totalValueUpdates() const { return m_totalValueUpdates; } ///< 获取值更新总次数
    quint64 totalColorResets() const { return m_totalColorResets; }   ///< 获取chunk颜色重置总次数
    quint64 totalShimmersStarted() const { return m_totalShimmersStarted; } ///< 获取shimmer动画启动总次数
    quint64 totalShimmersStopped() const { return m_totalShimmersStopped; } ///< 获取shimmer动画停止总次数
    quint64 totalChunkColorChanges() const { return m_totalChunkColorChanges; } ///< 获取chunk颜色变更总次数
    void resetStatistics() { m_totalAnimations = 0; m_totalValueUpdates = 0; m_totalColorResets = 0; m_totalShimmersStarted = 0; m_totalShimmersStopped = 0; m_totalChunkColorChanges = 0; } ///< 重置统计计数器
    /** @brief 启动shimmer流动动画(2000ms循环) */
    void startShimmer() {
        stopShimmer(); ++m_totalAnimations; ++m_totalShimmersStarted;
        m_shimmerAnim = new QPropertyAnimation(this, "shimmerOffset");
        m_shimmerAnim->setStartValue(0.0); m_shimmerAnim->setEndValue(1.0); m_shimmerAnim->setDuration(2000);
        m_shimmerAnim->setEasingCurve(QEasingCurve::Linear); m_shimmerAnim->setLoopCount(-1);
        m_shimmerAnim->start(QAbstractAnimation::DeleteWhenStopped);
    }
    void stopShimmer() { if (m_shimmerAnim) { m_shimmerAnim->stop(); m_shimmerAnim = nullptr; ++m_totalShimmersStopped; } m_shimmerOffset = 0.0; update(); } ///< 停止shimmer动画

protected:
    /** @brief 重绘进度条，在chunk上叠加自定义颜色和shimmer渐变 @param event 绘制事件 */
    void paintEvent(QPaintEvent* event) override {
        QProgressBar::paintEvent(event);
        QStyleOptionProgressBar opt; opt.initFrom(this);
        opt.minimum = minimum(); opt.maximum = maximum(); opt.progress = value();
        opt.textVisible = isTextVisible(); opt.text = text(); opt.textAlignment = alignment();
        QRect chunkRect = style()->subElementRect(QStyle::SE_ProgressBarContents, &opt, this);
        if (!chunkRect.isValid()) return;
        QPainter p(this); p.setClipRect(chunkRect);
        if (m_customChunkColor && m_chunkColor.isValid()) p.fillRect(chunkRect, m_chunkColor);
        if (m_shimmerOffset <= 0.0 || value() <= minimum()) return;
        auto& theme = ThemeManager::instance();
        QColor accent = theme.color(ThemeManager::SemanticColor::Accent);
        QColor lighter = accent.lighter(140);
        qreal bandWidth = 0.4, offset = m_shimmerOffset;
        qreal gradStart = offset - bandWidth, gradEnd = offset + bandWidth;
        QLinearGradient gradient(chunkRect.left(), 0, chunkRect.right(), 0);
        gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
        gradient.setColorAt(0.0, accent);
        gradient.setColorAt(qBound(0.0, gradStart, 1.0), accent);
        gradient.setColorAt(qBound(0.0, offset, 1.0), lighter);
        gradient.setColorAt(qBound(0.0, gradEnd, 1.0), accent);
        gradient.setColorAt(1.0, accent);
        p.fillRect(chunkRect, gradient);
    }

private:
    qreal m_shimmerOffset;              ///< shimmer偏移量 [0.0, 1.0]
    QPropertyAnimation* m_shimmerAnim;  ///< shimmer动画实例
    QColor m_chunkColor;                ///< 自定义chunk颜色（传输完成变色动画）
    bool m_customChunkColor;            ///< 是否使用自定义chunk颜色
    mutable quint64 m_totalAnimations = 0;  ///< 动画播放总次数
    mutable quint64 m_totalValueUpdates = 0; ///< 值更新总次数
    mutable quint64 m_totalColorResets = 0; ///< chunk颜色重置总次数
    mutable quint64 m_totalShimmersStarted = 0; ///< shimmer动画启动总次数
    mutable quint64 m_totalShimmersStopped = 0; ///< shimmer动画停止总次数
    mutable quint64 m_totalChunkColorChanges = 0; ///< chunk颜色变更总次数
};

#endif // ANIMATEDPROGRESSBAR_H

/** @file AnimatedProgressBar.h @brief 带shimmer流动效果的进度条 -- OTA传输视觉反馈。header-only实现，QPropertyAnimation驱动shimmerOffset属性，颜色从ThemeManager获取 */

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
    explicit AnimatedProgressBar(QWidget* parent = nullptr) ///< 构造，初始化进度条并设置objectName
        : QProgressBar(parent), m_shimmerOffset(0.0), m_shimmerAnim(nullptr)
        , m_customChunkColor(false)
    {
        setObjectName("animatedProgressBar");
    }

    qreal shimmerOffset() const { return m_shimmerOffset; } ///< 获取shimmer偏移量[0.0, 1.0]

    /** 设置shimmer偏移量，由QPropertyAnimation驱动 @param offset 目标偏移量[0.0, 1.0] */
    void setShimmerOffset(qreal offset) {
        if (!qFuzzyCompare(m_shimmerOffset, offset)) {
            m_shimmerOffset = offset;
            emit shimmerOffsetChanged();
            update();  // 触发重绘
        }
    }

signals:
    void shimmerOffsetChanged();             ///< shimmerOffset属性变更通知
    void chunkColorChanged();                ///< chunkColor属性变更通知

public:
    QColor chunkColor() const { return m_chunkColor; } ///< 获取chunk自定义颜色(无效QColor=未设置)

    /** 设置chunk自定义颜色，由QPropertyAnimation驱动传输完成变色动画 @param color 目标颜色 */
    void setChunkColor(const QColor& color) {
        m_chunkColor = color;
        m_customChunkColor = true;
        emit chunkColorChanged();
        update();
    }

    void resetChunkColor() {                 ///< 恢复QSS主题默认chunk颜色
        m_customChunkColor = false;
        m_chunkColor = QColor();
        emit chunkColorChanged();
        update();
    }

    void setValue(int value) { ++m_totalValueUpdates; QProgressBar::setValue(value); } ///< 重写setValue，增加统计
    quint64 totalAnimations() const { return m_totalAnimations; }  ///< 动画播放总次数
    quint64 totalValueUpdates() const { return m_totalValueUpdates; } ///< 值更新总次数
    void resetStatistics() { m_totalAnimations = 0; m_totalValueUpdates = 0; } ///< 重置统计计数器

    void startShimmer() {                   ///< 启动shimmer流动动画(2000ms循环)
        stopShimmer();
        ++m_totalAnimations;
        m_shimmerAnim = new QPropertyAnimation(this, "shimmerOffset");
        m_shimmerAnim->setStartValue(0.0);
        m_shimmerAnim->setEndValue(1.0);
        m_shimmerAnim->setDuration(2000);
        m_shimmerAnim->setEasingCurve(QEasingCurve::Linear);
        m_shimmerAnim->setLoopCount(-1);
        m_shimmerAnim->start(QAbstractAnimation::DeleteWhenStopped);
    }

    void stopShimmer() {                    ///< 停止shimmer动画(置nullptr防悬空指针)
        if (m_shimmerAnim) {
            m_shimmerAnim->stop();
            m_shimmerAnim = nullptr;
        }
        m_shimmerOffset = 0.0;
        update();
    }

protected:
    /** @brief 重绘进度条，在chunk上叠加自定义颜色和shimmer渐变 @param event 绘制事件
     *
     * 绘制流程:
     *   1. 先调用基类paintEvent绘制默认进度条（含QSS样式）
     *   2. 若设置了自定义chunk颜色，在chunk区域覆盖绘制
     *   3. 若shimmer激活，在其上叠加水平移动的QLinearGradient
     */
    void paintEvent(QPaintEvent* event) override {
        // 先绘制基类进度条（保留QSS样式）
        QProgressBar::paintEvent(event);

        // 获取chunk区域
        QStyleOptionProgressBar opt;
        opt.initFrom(this);
        opt.minimum = minimum();
        opt.maximum = maximum();
        opt.progress = value();
        opt.textVisible = isTextVisible();
        opt.text = text();
        opt.textAlignment = alignment();
        QRect chunkRect = style()->subElementRect(QStyle::SE_ProgressBarContents, &opt, this);
        if (!chunkRect.isValid())
            return;

        QPainter p(this);
        p.setClipRect(chunkRect);

        // 阶段1: 若设置了自定义chunk颜色，覆盖绘制chunk区域
        if (m_customChunkColor && m_chunkColor.isValid()) {
            p.fillRect(chunkRect, m_chunkColor);
        }

        // 阶段2: shimmer未激活时跳过流动效果
        if (m_shimmerOffset <= 0.0 || value() <= minimum())
            return;

        // 构建shimmer渐变: accent → lighterAccent → accent
        auto& theme = ThemeManager::instance();
        QColor accent = theme.color(ThemeManager::SemanticColor::Accent);
        QColor lighter = accent.lighter(140);  // 亮40%作为shimmer高光

        // shimmer带宽约为chunk宽度的40%
        qreal bandWidth = 0.4;
        qreal offset = m_shimmerOffset;

        // 渐变起点: offset向左偏移半个带宽，实现从左到右的扫过效果
        qreal gradStart = offset - bandWidth;
        qreal gradEnd = offset + bandWidth;

        QLinearGradient gradient(chunkRect.left(), 0, chunkRect.right(), 0);
        gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
        gradient.setColorAt(0.0, accent);

        qreal normalizedStart = qBound(0.0, gradStart, 1.0);
        qreal normalizedMid = qBound(0.0, offset, 1.0);
        qreal normalizedEnd = qBound(0.0, gradEnd, 1.0);

        gradient.setColorAt(normalizedStart, accent);
        gradient.setColorAt(normalizedMid, lighter);
        gradient.setColorAt(normalizedEnd, accent);
        gradient.setColorAt(1.0, accent);

        // 在chunk上叠加半透明渐变
        p.fillRect(chunkRect, gradient);
    }

private:
    qreal m_shimmerOffset;              ///< shimmer偏移量 [0.0, 1.0]
    QPropertyAnimation* m_shimmerAnim;  ///< shimmer动画实例
    QColor m_chunkColor;                ///< 自定义chunk颜色（传输完成变色动画）
    bool m_customChunkColor;            ///< 是否使用自定义chunk颜色（false时使用QSS默认色）

    mutable quint64 m_totalAnimations = 0;  ///< 动画播放总次数
    mutable quint64 m_totalValueUpdates = 0; ///< 值更新总次数
};

#endif // ANIMATEDPROGRESSBAR_H

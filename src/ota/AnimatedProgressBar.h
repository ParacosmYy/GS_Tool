/**
 * @file AnimatedProgressBar.h
 * @brief 带shimmer流动效果的进度条 -- OTA传输中的视觉反馈
 *
 * 设计要点:
 *   1. 继承 QProgressBar，通过 QPropertyAnimation 驱动 shimmerOffset 属性
 *   2. paintEvent() 在chunk区域上叠加一个水平移动的 QLinearGradient（accent→亮accent→accent）
 *   3. shimmerOffset 范围 [0.0, 1.0]，2000ms循环，Linear曲线
 *   4. 颜色全部从 ThemeManager::SemanticColor::Accent 获取，无硬编码
 *   5. header-only 实现，无需额外 .cpp 文件
 *
 * 协作关系:
 *   - ThemeManager: 提供accent强调色
 *   - OtaWidget: 在传输开始/结束时调用 startShimmer()/stopShimmer()
 */

#ifndef ANIMATEDPROGRESSBAR_H
#define ANIMATEDPROGRESSBAR_H

#include <QProgressBar>
#include <QPropertyAnimation>
#include <QPainter>
#include <QLinearGradient>
#include <QStyleOptionProgressBar>
#include "core/ThemeManager.h"

/**
 * @brief 带shimmer流动效果的进度条控件
 *
 * 传输进行时，进度条chunk上显示一条水平移动的亮光带，
 * 视觉上形成"数据正在流动"的反馈效果。
 * 使用 QPropertyAnimation 驱动，符合 CLAUDE.md §6.5 动画规范。
 */
class AnimatedProgressBar : public QProgressBar {
    Q_OBJECT
    Q_PROPERTY(qreal shimmerOffset READ shimmerOffset WRITE setShimmerOffset)

public:
    /**
     * @brief 构造函数
     * @param parent 父控件
     */
    explicit AnimatedProgressBar(QWidget* parent = nullptr)
        : QProgressBar(parent), m_shimmerOffset(0.0), m_shimmerAnim(nullptr)
    {}

    /** @brief 当前shimmer偏移量，范围 [0.0, 1.0] */
    qreal shimmerOffset() const { return m_shimmerOffset; }

    /** @brief 设置shimmer偏移量（由QPropertyAnimation驱动） */
    void setShimmerOffset(qreal offset) {
        if (!qFuzzyCompare(m_shimmerOffset, offset)) {
            m_shimmerOffset = offset;
            update();  // 触发重绘
        }
    }

    /**
     * @brief 启动shimmer流动动画
     *
     * 创建2000ms循环的QPropertyAnimation，驱动shimmerOffset从0→1，
     * 缓动曲线为Linear，无限循环。传输开始时调用。
     */
    void startShimmer() {
        stopShimmer();
        m_shimmerAnim = new QPropertyAnimation(this, "shimmerOffset");
        m_shimmerAnim->setStartValue(0.0);
        m_shimmerAnim->setEndValue(1.0);
        m_shimmerAnim->setDuration(2000);
        m_shimmerAnim->setEasingCurve(QEasingCurve::Linear);
        m_shimmerAnim->setLoopCount(-1);  // 无限循环
        m_shimmerAnim->start(QAbstractAnimation::DeleteWhenStopped);
    }

    /**
     * @brief 停止shimmer流动动画
     *
     * 停止动画并重置偏移量为0，传输结束/出错时调用。
     */
    void stopShimmer() {
        if (m_shimmerAnim) {
            m_shimmerAnim->stop();
            m_shimmerAnim = nullptr;
        }
        m_shimmerOffset = 0.0;
        update();
    }

protected:
    /**
     * @brief 重绘进度条，在chunk上叠加shimmer渐变
     *
     * 绘制流程:
     *   1. 先调用基类paintEvent绘制默认进度条（含QSS样式）
     *   2. 获取chunk区域，在其上叠加水平移动的QLinearGradient
     *   3. 渐变色带: accent → lighterAccent → accent，宽度约chunk的40%
     */
    void paintEvent(QPaintEvent* event) override {
        // 先绘制基类进度条（保留QSS样式）
        QProgressBar::paintEvent(event);

        // shimmer未激活时不叠加效果
        if (m_shimmerOffset <= 0.0 || value() <= minimum())
            return;

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

        // 在gradStart位置开始变亮
        qreal normalizedStart = qBound(0.0, gradStart, 1.0);
        qreal normalizedMid = qBound(0.0, offset, 1.0);
        qreal normalizedEnd = qBound(0.0, gradEnd, 1.0);

        gradient.setColorAt(normalizedStart, accent);
        gradient.setColorAt(normalizedMid, lighter);
        gradient.setColorAt(normalizedEnd, accent);
        gradient.setColorAt(1.0, accent);

        // 在chunk上叠加半透明渐变
        QPainter p(this);
        p.setClipRect(chunkRect);
        p.fillRect(chunkRect, gradient);
    }

private:
    qreal m_shimmerOffset;          ///< shimmer偏移量 [0.0, 1.0]
    QPropertyAnimation* m_shimmerAnim;  ///< shimmer动画实例
};

#endif // ANIMATEDPROGRESSBAR_H

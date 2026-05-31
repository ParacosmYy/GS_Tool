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
 *
 * 额外功能:
 *   - setChunkColor(): 设置chunk区域的背景色（用于传输完成变色动画）
 *   - resetChunkColor(): 恢复QSS主题默认颜色
 *   - 布局属性（border-radius等）由QSS主题文件控制，不在C++中硬编码
 */
class AnimatedProgressBar : public QProgressBar {
    Q_OBJECT
    Q_PROPERTY(qreal shimmerOffset READ shimmerOffset WRITE setShimmerOffset NOTIFY shimmerOffsetChanged)

public:
    /**
     * @brief 构造函数
     * @param parent 父控件
     */
    explicit AnimatedProgressBar(QWidget* parent = nullptr)
        : QProgressBar(parent), m_shimmerOffset(0.0), m_shimmerAnim(nullptr)
        , m_customChunkColor(false)
    {}

    /** @brief 当前shimmer偏移量，范围 [0.0, 1.0] */
    qreal shimmerOffset() const { return m_shimmerOffset; }

    /** @brief 设置shimmer偏移量（由QPropertyAnimation驱动） */
    void setShimmerOffset(qreal offset) {
        if (!qFuzzyCompare(m_shimmerOffset, offset)) {
            m_shimmerOffset = offset;
            emit shimmerOffsetChanged();
            update();  // 触发重绘
        }
    }

signals:
    /** @brief shimmerOffset属性变更通知信号 */
    void shimmerOffsetChanged();

public:
    /** @brief 设置chunk区域自定义颜色(传输完成变色动画) */
    void setChunkColor(const QColor& color) {
        m_chunkColor = color;
        m_customChunkColor = true;
        update();
    }

    /** @brief 恢复QSS主题默认chunk颜色 */
    void resetChunkColor() {
        m_customChunkColor = false;
        m_chunkColor = QColor();
        update();
    }

    /** @brief 启动shimmer流动动画(2000ms循环)
     *
     * 安全处理: 先停止旧动画，再创建新动画。
     * 如果旧动画已停止但尚未被 Qt 删除(DeleteWhenStopped是异步的)，
     * 设置 parent 为 this 确保旧对象在新对象创建前被安全管理。
     */
    void startShimmer() {
        stopShimmer();
        // 安全创建新动画: 父对象为 this，确保生命周期受控
        m_shimmerAnim = new QPropertyAnimation(this, "shimmerOffset");
        m_shimmerAnim->setStartValue(0.0);
        m_shimmerAnim->setEndValue(1.0);
        m_shimmerAnim->setDuration(2000);
        m_shimmerAnim->setEasingCurve(QEasingCurve::Linear);
        m_shimmerAnim->setLoopCount(-1);
        m_shimmerAnim->start(QAbstractAnimation::DeleteWhenStopped);
    }

    /** @brief 停止shimmer流动动画
     *
     * 安全处理: 调用 stop() 后立即置 nullptr。
     * DeleteWhenStopped会在事件循环中异步删除动画对象，
     * 必须立即置nullptr，否则下次startShimmer()调用stopShimmer()时
     * if(m_shimmerAnim)仍为true（悬空指针），导致use-after-free崩溃。
     */
    void stopShimmer() {
        if (m_shimmerAnim) {
            m_shimmerAnim->stop();
            // DeleteWhenStopped会在事件循环中异步删除动画对象
            // 必须立即置nullptr，否则下次startShimmer()调用stopShimmer()时
            // if(m_shimmerAnim)仍为true（悬空指针），导致use-after-free崩溃
            m_shimmerAnim = nullptr;
        }
        m_shimmerOffset = 0.0;
        update();
    }

protected:
    /**
     * @brief 重绘进度条，在chunk上叠加自定义颜色和shimmer渐变
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
};

#endif // ANIMATEDPROGRESSBAR_H

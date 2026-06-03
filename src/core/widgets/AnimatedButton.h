/**
 * @file AnimatedButton.h
 * @brief 通用动画按钮 - 提供hover渐变和press回弹微交互动画
 *
 * 职责:
 *   1. 鼠标进入/离开时opacity渐变动画(200ms OutCubic)
 *   2. 鼠标按下/释放时opacity反馈动画(100ms)
 *
 * 设计参考: Linear.app 的按钮层次体系 + Raycast 的微交互反馈
 * 使用方式: 替换 QPushButton 为 AnimatedButton，QSS样式不受影响
 *
 * 动画规格(CLAUDE.md Section 6.5):
 *   - Hover: 200ms OutCubic opacity渐变(0.85 → 1.0)
 *   - Press: 100ms opacity → 0.75 (按下感)
 *   - Release: 100ms OutCubic opacity → 1.0 (回弹)
 *
 * 协作关系:
 *   - ThemeManager: 通过QSS获取颜色，不直接读取颜色
 *   - QPushButton: 基类，提供按钮功能
 */

#ifndef ANIMATEDBUTTON_H
#define ANIMATEDBUTTON_H

#include <QPushButton>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

/**
 * @brief 通用动画按钮 - 为QPushButton添加hover渐变和press回弹效果
 *
 * 通过QGraphicsOpacityEffect实现opacity动画，不改变按钮几何/布局。
 * 按钮的QSS样式、objectName、信号/槽均不受影响。
 */
class AnimatedButton : public QPushButton {
    Q_OBJECT

    /** @brief 动画opacity属性(0.75~1.0)，用于QPropertyAnimation驱动 */
    Q_PROPERTY(qreal animOpacity READ animOpacity WRITE setAnimOpacity)

public:
    /**
     * @brief 构造动画按钮
     * @param text 按钮文本
     * @param parent 父widget
     */
    explicit AnimatedButton(const QString& text = QString(),
                            QWidget* parent = nullptr);

    /** @brief 获取当前动画opacity值 */
    qreal animOpacity() const;

    /**
     * @brief 设置动画opacity值(由QPropertyAnimation调用)
     * @param opacity 目标opacity(0.0~1.0)
     */
    void setAnimOpacity(qreal opacity);

protected:
    /** @brief 鼠标进入 - 启动hover渐入动画(200ms OutCubic, →1.0) */
    void enterEvent(QEnterEvent* event) override;

    /** @brief 鼠标离开 - 启动hover渐出动画(200ms OutCubic, →0.85) */
    void leaveEvent(QEvent* event) override;

    /** @brief 鼠标按下 - 启动按下动画(100ms Linear, →0.75) */
    void mousePressEvent(QMouseEvent* event) override;

    /** @brief 鼠标释放 - 启动回弹动画(100ms OutCubic, →1.0) */
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    /** @brief 确保opacity特效已创建(惰性初始化) */
    void ensureOpacityEffect();

    /** @brief 启动opacity动画，自动停止前一个动画防止冲突 @param targetOpacity 目标值 @param durationMs 持续时间 @param curve 缓动曲线 */
    void startOpacityAnim(qreal targetOpacity, int durationMs, QEasingCurve curve);

    qreal m_animOpacity = 1.0;                ///< 当前动画opacity(0.75~1.0)
    QGraphicsOpacityEffect* m_opacityEffect = nullptr;  ///< opacity特效(由this拥有)
    QPropertyAnimation* m_activeAnim = nullptr;         ///< 当前活跃的动画(防止并发冲突)

    // ---- 统计计数器(静态，跨所有实例累积) ----
    static inline quint64 s_totalHoverEnters = 0;     ///< 累计鼠标进入次数
    static inline quint64 s_totalClicks = 0;          ///< 累计点击次数
    static inline quint64 s_totalAnimationsStarted = 0; ///< 累计动画启动次数
public:
    /** @brief 获取累计鼠标进入次数 */
    static quint64 totalHoverEnters() { return s_totalHoverEnters; }
    /** @brief 获取累计点击次数 */
    static quint64 totalButtonClicks() { return s_totalClicks; }
    /** @brief 获取累计动画启动次数 */
    static quint64 totalAnimationsStarted() { return s_totalAnimationsStarted; }
    /** @brief 重置按钮统计计数器 */
    static void resetButtonStatistics() { s_totalHoverEnters = 0; s_totalClicks = 0; s_totalAnimationsStarted = 0; }
};

#endif // ANIMATEDBUTTON_H

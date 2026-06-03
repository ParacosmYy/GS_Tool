/**
 * @file SkeletonWidget.h
 * @brief 骨架屏组件 — 灰色占位块+渐变微光动画，用于内容加载占位
 *
 * 显示为指定尺寸的圆角灰色矩形，带有从左到右的微光扫描动画。
 */
#ifndef SKELETON_WIDGET_H
#define SKELETON_WIDGET_H

#include <QWidget>

/**
 * @brief 骨架屏占位组件
 *
 * 使用场景:
 *   - 列表项加载占位
 *   - 文本块加载占位
 *   - 图片加载占位
 */
class SkeletonWidget : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief 构造骨架屏
     * @param width 宽度(px)
     * @param height 高度(px)，默认20
     * @param borderRadius 圆角半径(px)，默认4
     * @param parent 父控件
     */
    explicit SkeletonWidget(int width, int height = 20, int borderRadius = 4,
                             QWidget* parent = nullptr);

protected:
    /** @brief 绘制骨架块+微光动画 */
    void paintEvent(QPaintEvent* event) override;

private:
    int m_borderRadius = 4;     ///< 圆角半径
    int m_shimmerOffset = 0;    ///< 微光偏移(0~2*width)
    class QTimer* m_timer = nullptr;
};

#endif // SKELETON_WIDGET_H

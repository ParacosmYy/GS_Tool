#ifndef BACKGROUNDWIDGET_H
#define BACKGROUNDWIDGET_H

#include <QWidget>
#include <QPixmap>
#include <QVector>
#include <QTimer>
#include <QColor>

/**
 * @brief 背景层控件 - 提供自定义背景图、磨砂玻璃模糊、透明度调节和点击涟漪特效
 *
 * 作为 MainWindow 的中央部件（centralWidget），承载所有面板的底层背景。
 * 通过 QSS Q_PROPERTY 暴露模糊半径和透明度，支持样式表动态控制。
 *
 * 绘制层次（从底到顶）:
 *   1. 纯黑底色
 *   2. 模糊背景图（Cover 模式居中裁剪填满）
 *   3. 半透明遮罩层（提升文字可读性）
 *   4. 涟漪特效（跟随主题 accent 色）
 *
 * 性能优化: 模糊图在设置时预生成，缩放到窗口尺寸的图在 resize 时缓存，
 * paintEvent 不做任何耗时的图片缩放操作，保证 60fps 涟漪动画流畅。
 *
 * 协作关系:
 *   - BackgroundSettingsPopup: 提供模糊/透明度/涟漪开关/自定义背景图的 UI 控制
 *   - MainWindow: 作为中央部件承载所有面板
 */
class BackgroundWidget : public QWidget {
    Q_OBJECT
    /** @brief 模糊半径属性，可通过 QSS 设置 (0=无模糊, 1~30) */
    Q_PROPERTY(qreal blurRadius READ blurRadius WRITE setBlurRadius NOTIFY blurChanged)
    /** @brief 背景透明度属性，可通过 QSS 设置 (0=完全透明, 1=完全不透明) */
    Q_PROPERTY(qreal bgOpacity READ bgOpacity WRITE setBgOpacity NOTIFY opacityChanged)

public:
    /**
     * @brief 构造背景控件
     * 初始化涟漪动画定时器（~60fps）并加载默认背景图
     * @param parent 父 widget
     */
    explicit BackgroundWidget(QWidget* parent = nullptr);

    /**
     * @brief 设置背景图片
     * 加载图片资源并生成模糊版本，触发重绘
     * @param resourcePath Qt 资源路径或本地文件系统路径
     */
    void setBackgroundImage(const QString& resourcePath);

    /**
     * @brief 设置磨砂玻璃模糊半径
     * @param radius 模糊半径 (0=无模糊, 最大30)
     */
    void setBlurRadius(qreal radius);

    /** @brief 获取当前模糊半径 */
    qreal blurRadius() const;

    /**
     * @brief 设置背景透明度
     * @param opacity 透明度 (0=完全透明, 1=完全不透明)
     */
    void setBgOpacity(qreal opacity);

    /** @brief 获取当前背景透明度 */
    qreal bgOpacity() const;

    /**
     * @brief 设置涟漪特效开关
     * @param enabled true=启用点击涟漪, false=禁用并清除已有涟漪
     */
    void setRippleEnabled(bool enabled);

    /** @brief 获取涟漪特效是否启用 */
    bool rippleEnabled() const;

    /**
     * @brief 设置涟漪颜色（用于主题适配）
     * @param color 涟漪颜色，默认跟随 accent 色 #89b4fa
     */
    void setRippleColor(const QColor& color);

    /** @brief 获取当前涟漪颜色 */
    QColor rippleColor() const;

    /**
     * @brief 设置半透明遮罩颜色
     * @param color 遮罩颜色，默认黑色
     */
    void setOverlayColor(const QColor& color);

    /** @brief 获取当前遮罩颜色 */
    QColor overlayColor() const;

    /**
     * @brief 设置遮罩透明度
     * @param opacity 透明度 (0=无遮罩, 1=全黑，默认0.2)
     */
    void setOverlayOpacity(qreal opacity);

    /** @brief 获取当前遮罩透明度 */
    qreal overlayOpacity() const;

    /**
     * @brief 设置模糊迭代次数
     * @param iterations 迭代次数 (2~10)，次数越多磨砂效果越自然，默认5次
     */
    void setBlurIterations(int iterations);

    /** @brief 获取当前模糊迭代次数 */
    int blurIterations() const;

    /** @brief 恢复为资源中的默认背景图 */
    void resetToDefault();

    /** @brief 获取当前背景图片路径 */
    QString currentImagePath() const;

signals:
    /** @brief 模糊半径变化信号 */
    void blurChanged(qreal radius);

    /** @brief 透明度变化信号 */
    void opacityChanged(qreal opacity);

    /** @brief 背景图片路径变化信号 */
    void backgroundImageChanged(const QString& path);

protected:
    /**
     * @brief 自绘事件 - 按层次渲染背景
     * 绘制顺序: 黑色底色 → 模糊背景图(Cover模式) → 半透明遮罩 → 涟漪特效
     * @param event 绘制事件
     */
    void paintEvent(QPaintEvent* event) override;

    /**
     * @brief 鼠标按下事件 - 创建涟漪动画
     * 在点击位置生成一个新的涟漪并启动动画定时器
     * @param event 鼠标事件
     */
    void mousePressEvent(QMouseEvent* event) override;

    /**
     * @brief 窗口尺寸变化事件 - 重新缓存缩放后的背景图
     * 避免在 paintEvent 中每帧重复缩放，保证渲染性能
     * @param event 尺寸变化事件
     */
    void resizeEvent(QResizeEvent* event) override;

private:
    /**
     * @brief 从原始图片生成模糊版本
     * 使用缩放法快速近似高斯模糊: 缩小→放大→放大，利用双线性插值平滑
     * @param src 原始像素图
     * @param radius 模糊半径
     * @return 模糊后的像素图
     */
    QPixmap generateBlurred(const QPixmap& src, qreal radius) const;

    /**
     * @brief 缓存当前窗口尺寸下的缩放背景图
     * 在 resize/blur/image 变化时调用，避免 paintEvent 每帧都做缩放
     */
    void regenerateScaledBackground();

    /**
     * @brief 涟漪动画帧更新
     * 每帧: 扩大涟漪半径 + 降低透明度，移除已完成的涟漪
     * 当所有涟漪完成后自动停止定时器
     */
    void advanceRipples();

    /** @brief 原始背景图（未模糊） */
    QPixmap m_originalImage;

    /** @brief 模糊后的背景图（原始尺寸，设置模糊半径时重新生成） */
    QPixmap m_blurredImage;

    /** @brief 缓存: 已缩放到当前窗口尺寸的模糊图（避免 paintEvent 中重复缩放） */
    QPixmap m_scaledBlurredImage;

    /** @brief 缓存: 缩放图的居中偏移量，避免 paintEvent 中重复计算 */
    QPoint m_scaledOffset;

    /** @brief 模糊半径，范围 [0, 30] */
    qreal m_blurRadius = 10.0;

    /** @brief 背景透明度，范围 [0, 1] */
    qreal m_bgOpacity = 0.35;

    /** @brief 涟漪特效开关 */
    bool m_rippleEnabled = true;

    /** @brief 模糊缩放迭代次数 (2~10)，次数越多磨砂效果越自然 */
    int m_blurIterations = 5;

    /** @brief 遮罩颜色，默认黑色 */
    QColor m_overlayColor = QColor(0, 0, 0);

    /** @brief 遮罩透明度 (0=无遮罩, 1=全黑) */
    qreal m_overlayOpacity = 0.2;

    /** @brief 涟漪颜色，默认 accent 色 #89b4fa */
    QColor m_rippleColor = QColor(137, 180, 250);

    /** @brief 当前背景图路径（资源路径或文件系统路径） */
    QString m_currentImagePath;

    /**
     * @brief 涟漪特效数据结构
     * 每次鼠标点击创建一个 Ripple，定时器驱动其扩散和衰减
     */
    struct Ripple {
        QPointF center;             ///< 涟漪圆心（点击位置）
        qreal currentRadius = 0.0;  ///< 当前扩散半径
        qreal maxRadius = 150.0;    ///< 最大扩散半径（到达后移除）
        qreal opacity = 0.6;        ///< 当前不透明度（递减至0时移除）
    };

    /** @brief 活跃涟漪列表 */
    QVector<Ripple> m_ripples;

    /** @brief 涟漪动画定时器，16ms 间隔（~60fps） */
    QTimer* m_rippleTimer;
};

#endif // BACKGROUNDWIDGET_H

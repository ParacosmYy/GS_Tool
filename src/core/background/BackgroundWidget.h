/**
 * @file BackgroundWidget.h
 * @brief 背景层控件 — 提供自定义背景图、磨砂玻璃模糊、透明度调节和点击涟漪特效
 *
 * 作为MainWindow的中央部件(centralWidget)，承载所有面板的底层背景。
 */
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
 *   - ThemeManager: 提供涟漪和遮罩的主题色，监听 themeChanged 自动更新
 */
class BackgroundWidget : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal blurRadius READ blurRadius WRITE setBlurRadius NOTIFY blurChanged)
    Q_PROPERTY(qreal bgOpacity READ bgOpacity WRITE setBgOpacity NOTIFY opacityChanged)

public:
    /** @brief 构造背景控件，初始化涟漪定时器（~60fps）并加载默认背景图 */
    explicit BackgroundWidget(QWidget* parent = nullptr);

    /** @brief 设置背景图片（Qt 资源路径或本地文件系统路径） */
    void setBackgroundImage(const QString& resourcePath);

    /** @brief 设置磨砂玻璃模糊半径 (0=无模糊, 最大30) */
    void setBlurRadius(qreal radius);
    qreal blurRadius() const;                       ///< 当前模糊半径

    /** @brief 设置背景透明度 (0=完全透明, 1=完全不透明) */
    void setBgOpacity(qreal opacity);
    qreal bgOpacity() const;                        ///< 当前背景透明度

    /** @brief 设置涟漪特效开关（禁用时清除已有涟漪） */
    void setRippleEnabled(bool enabled);
    bool rippleEnabled() const;                     ///< 涟漪是否启用

    /** @brief 设置涟漪颜色（默认跟随 ThemeManager Accent 色） */
    void setRippleColor(const QColor& color);
    QColor rippleColor() const;                     ///< 当前涟漪颜色

    /** @brief 设置半透明遮罩颜色（默认跟随 ThemeManager BgPrimary） */
    void setOverlayColor(const QColor& color);
    QColor overlayColor() const;                    ///< 当前遮罩颜色

    /** @brief 设置遮罩透明度 (0=无遮罩, 1=全黑) */
    void setOverlayOpacity(qreal opacity);
    qreal overlayOpacity() const;                   ///< 当前遮罩透明度

    /** @brief 设置模糊迭代次数 (2~10)，越多磨砂效果越自然 */
    void setBlurIterations(int iterations);
    int blurIterations() const;                     ///< 当前模糊迭代次数

    /** @brief 恢复默认背景图，清除自定义图片设置 */
    void resetToDefault();
    /** @brief 获取当前背景图文件路径 @return 图片路径字符串 */
    QString currentImagePath() const;

    // ── 统计计数器 ──

    /** @brief 获取图片加载总次数 @return 累计图片加载次数 */
    quint64 totalImageLoads() const { return m_totalImageLoads; }
    /** @brief 获取效果变更总次数(模糊/透明度/涟漪等) @return 累计效果变更次数 */
    quint64 totalEffectChanges() const { return m_totalEffectChanges; }
    /** @brief 获取涟漪创建总次数 @return 累计涟漪动画创建次数 */
    quint64 totalRipples() const { return m_totalRipples; }
    /** @brief 获取主题更新总次数 @return 累计主题色变更次数 */
    quint64 totalThemeUpdates() const { return m_totalThemeUpdates; }
    /** @brief 获取重绘总次数 @return 累计paintEvent调用次数 */
    quint64 totalPaints() const { return m_totalPaints; }
    /** @brief 获取窗口尺寸变更总次数 @return 累计resizeEvent调用次数 */
    quint64 totalResizes() const { return m_totalResizes; }
    /** @brief 重置所有统计计数器 */
    void resetBackgroundStatistics();

signals:
    void blurChanged(qreal radius);                 ///< 模糊半径变化
    void opacityChanged(qreal opacity);             ///< 透明度变化
    void backgroundImageChanged(const QString& path); ///< 背景图路径变化

protected:
    void paintEvent(QPaintEvent* event) override;   ///< 四层渲染: 黑底→模糊图→遮罩→涟漪
    void mousePressEvent(QMouseEvent* event) override; ///< 点击生成涟漪
    void resizeEvent(QResizeEvent* event) override; ///< 重新缓存缩放背景图

private:
    /** @brief 缩放法快速近似高斯模糊（缩小→放大利用双线性插值平滑） */
    QPixmap generateBlurred(const QPixmap& src, qreal radius) const;
    /** @brief 缓存当前窗口尺寸的缩放背景图，避免 paintEvent 中重复缩放 */
    void regenerateScaledBackground();
    /** @brief 涟漪动画帧更新（扩散半径递增 + 不透明度衰减） */
    void advanceRipples();
    /** @brief 从 ThemeManager 加载主题色（涟漪色/遮罩色）并触发重绘 */
    void updateThemeColors();

    QPixmap m_originalImage;            ///< 原始背景图（未模糊）
    QPixmap m_blurredImage;             ///< 模糊后的背景图（原始尺寸）
    QPixmap m_scaledBlurredImage;       ///< 缓存: 已缩放到当前窗口尺寸的模糊图
    QPoint m_scaledOffset;              ///< 缓存: 缩放图的居中偏移量

    qreal m_blurRadius = 10.0;          ///< 模糊半径 [0, 30]
    qreal m_bgOpacity = 0.35;           ///< 背景透明度 [0, 1]
    bool m_rippleEnabled = true;        ///< 涟漪特效开关
    int m_blurIterations = 5;           ///< 模糊迭代次数 (2~10)
    QColor m_overlayColor;              ///< 遮罩颜色（从 ThemeManager 初始化）
    qreal m_overlayOpacity = 0.2;       ///< 遮罩透明度 [0, 1]
    QColor m_rippleColor;               ///< 涟漪颜色（从 ThemeManager 初始化）
    QString m_currentImagePath;         ///< 当前背景图路径

    /// @brief 涟漪数据: 每次点击创建一个，定时器驱动扩散和衰减
    struct Ripple {
        QPointF center;                 ///< 涟漪圆心
        qreal currentRadius = 0.0;      ///< 当前扩散半径
        qreal maxRadius = 150.0;        ///< 最大扩散半径
        qreal opacity = 0.6;            ///< 不透明度（递减至0时移除）
    };

    QVector<Ripple> m_ripples;          ///< 活跃涟漪列表
    QTimer* m_rippleTimer;              ///< 涟漪动画定时器（~60fps）

    // ── 统计计数器 ──
    quint64 m_totalImageLoads = 0;      ///< 图片加载次数
    quint64 m_totalEffectChanges = 0;   ///< 效果变更次数(模糊/透明度/涟漪等)
    quint64 m_totalRipples = 0;         ///< 涟漪创建次数
    quint64 m_totalThemeUpdates = 0;    ///< 主题色更新次数
    mutable quint64 m_totalPaints = 0;  ///< 重绘次数(paintEvent中递增，需mutable)
    quint64 m_totalResizes = 0;         ///< 窗口尺寸变更次数
};

#endif // BACKGROUNDWIDGET_H

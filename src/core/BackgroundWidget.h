#ifndef BACKGROUNDWIDGET_H
#define BACKGROUNDWIDGET_H

#include <QWidget>
#include <QPixmap>
#include <QVector>
#include <QTimer>

// 背景组件 - 支持自定义背景图、磨砂玻璃(模糊)、透明度调节、点击涟漪特效
// 作为MainWindow的底层widget，承载所有面板
class BackgroundWidget : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal blurRadius READ blurRadius WRITE setBlurRadius NOTIFY blurChanged)
    Q_PROPERTY(qreal bgOpacity READ bgOpacity WRITE setBgOpacity NOTIFY opacityChanged)

public:
    explicit BackgroundWidget(QWidget* parent = nullptr);

    // 设置背景图片路径
    void setBackgroundImage(const QString& resourcePath);

    // 磨砂玻璃模糊半径 (0=无模糊, 1~30)
    void setBlurRadius(qreal radius);
    qreal blurRadius() const;

    // 背景透明度 (0=完全透明, 1=完全不透明)
    void setBgOpacity(qreal opacity);
    qreal bgOpacity() const;

    // 启用/禁用点击涟漪特效
    void setRippleEnabled(bool enabled);
    bool rippleEnabled() const;

signals:
    void blurChanged(qreal radius);
    void opacityChanged(qreal opacity);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    // 从原始图片生成模糊版本 (缩放法快速近似)
    QPixmap generateBlurred(const QPixmap& src, qreal radius) const;

    // 涟漪动画帧更新
    void advanceRipples();

    QPixmap m_originalImage;     // 原始背景图
    QPixmap m_blurredImage;      // 模糊后的背景图
    qreal m_blurRadius = 10.0;   // 模糊半径
    qreal m_bgOpacity = 0.35;    // 背景透明度
    bool m_rippleEnabled = true;

    // 涟漪特效数据
    struct Ripple {
        QPointF center;
        qreal currentRadius = 0.0;
        qreal maxRadius = 150.0;
        qreal opacity = 0.6;
    };
    QVector<Ripple> m_ripples;
    QTimer* m_rippleTimer;
};

#endif // BACKGROUNDWIDGET_H

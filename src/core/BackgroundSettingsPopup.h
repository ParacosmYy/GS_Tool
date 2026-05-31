#ifndef BACKGROUNDSETTINGSDOCK_H
#define BACKGROUNDSETTINGSDOCK_H

#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QPushButton>

class BackgroundWidget;

// 背景设置弹出面板 - 磨砂玻璃/透明度/涟漪开关
// 从工具栏按钮弹出，浮动在主窗口上方
class BackgroundSettingsPopup : public QWidget {
    Q_OBJECT

public:
    explicit BackgroundSettingsPopup(BackgroundWidget* bgWidget, QWidget* parent = nullptr);

    // 从BackgroundWidget同步当前值到控件
    void syncFromWidget();

signals:
    void hidden();

protected:
    void hideEvent(QHideEvent* event) override;

private:
    BackgroundWidget* m_bgWidget;

    QSlider* m_blurSlider;
    QLabel* m_blurValueLbl;
    QSlider* m_opacitySlider;
    QLabel* m_opacityValueLbl;
    QPushButton* m_rippleBtn;
    QPushButton* m_closeBtn;
};

#endif // BACKGROUNDSETTINGSDOCK_H

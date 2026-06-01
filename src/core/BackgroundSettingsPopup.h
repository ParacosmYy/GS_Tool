#ifndef BACKGROUNDSETTINGSPOPUP_H
#define BACKGROUNDSETTINGSPOPUP_H

#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QPushButton>

class BackgroundWidget;

/**
 * @brief 背景设置弹出面板 - 提供磨砂玻璃/透明度/涟漪开关/自定义背景图的实时调节
 *
 * 作为浮动弹出窗口（Qt::Popup），从工具栏"背景"按钮弹出。
 * 点击外部区域自动关闭。所有调节实时反映到 BackgroundWidget。
 *
 * 协作关系:
 *   - BackgroundWidget: 被控对象，所有滑块和开关直接操作其属性
 *   - MainWindow: 管理弹出面板的显示/隐藏和定位
 */
class BackgroundSettingsPopup : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief 构造背景设置弹出面板
     * 创建模糊半径滑块、透明度滑块、背景图选择按钮，并连接到 BackgroundWidget
     * @param bgWidget 被控的背景控件实例
     * @param parent 父 widget
     */
    explicit BackgroundSettingsPopup(BackgroundWidget* bgWidget, QWidget* parent = nullptr);

    /**
     * @brief 从 BackgroundWidget 同步当前值到 UI 控件
     * 每次显示弹出面板前调用，确保滑块位置与实际值一致
     */
    void syncFromWidget();

signals:
    /** @brief 面板隐藏信号，通知 MainWindow 更新按钮状态 */
    void hidden();

    /**
     * @brief 用户选择了自定义背景图
     * @param filePath 选择的图片文件路径
     */
    void backgroundImageSelected(const QString& filePath);

    /** @brief 用户请求恢复默认背景图 */
    void resetToDefaultRequested();

protected:
    /**
     * @brief 隐藏事件处理
     * 在面板隐藏时发出 hidden() 信号
     * @param event 隐藏事件
     */
    void hideEvent(QHideEvent* event) override;

private:
    /**
     * @brief 打开文件对话框选择背景图
     * 用户选择图片后通过 backgroundImageSelected 信号通知 MainWindow
     */
    void onSelectBackground();

    /** @brief 创建模糊/透明度滑块、涟漪开关、分隔线并添加到布局 */
    void createControls(class QVBoxLayout* mainLayout);

    /** @brief 被控的背景控件实例 */
    BackgroundWidget* m_bgWidget;

    /** @brief 模糊半径滑块 (范围 0~30) */
    QSlider* m_blurSlider;

    /** @brief 模糊半径当前值标签 */
    QLabel* m_blurValueLbl;

    /** @brief 背景透明度滑块 (范围 0~100，映射到 0.0~1.0) */
    QSlider* m_opacitySlider;

    /** @brief 透明度当前值标签（显示百分比） */
    QLabel* m_opacityValueLbl;

    /** @brief 选择自定义背景图按钮 */
    QPushButton* m_selectImageBtn;

    /** @brief 恢复默认背景图按钮 */
    QPushButton* m_resetBtn;
};

#endif // BACKGROUNDSETTINGSPOPUP_H

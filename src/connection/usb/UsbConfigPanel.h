/**
 * @file UsbConfigPanel.h
 * @brief USB配置面板 — 配置USB连接参数
 *
 * 提供设备选择、VID/PID输入、接口编号和连接按钮。
 */
#ifndef USB_CONFIG_PANEL_H
#define USB_CONFIG_PANEL_H

#include <QWidget>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>

/**
 * @brief USB连接配置面板控件
 * 配置USB设备的VID、PID、接口编号等连接参数。
 */
class UsbConfigPanel : public QWidget {
    Q_OBJECT

public:
    explicit UsbConfigPanel(QWidget* parent = nullptr);

private:
    QComboBox*  m_deviceCombo    = nullptr; ///< 设备选择下拉框
    QSpinBox*   m_vidSpin        = nullptr; ///< VID输入 (十六进制)
    QSpinBox*   m_pidSpin        = nullptr; ///< PID输入 (十六进制)
    QSpinBox*   m_interfaceSpin  = nullptr; ///< 接口编号
    QPushButton* m_connectBtn    = nullptr; ///< 连接/断开按钮
};

#endif // USB_CONFIG_PANEL_H

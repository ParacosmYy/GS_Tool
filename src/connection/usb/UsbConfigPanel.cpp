/**
 * @file UsbConfigPanel.cpp
 * @brief USB配置面板实现
 */
#include "connection/usb/UsbConfigPanel.h"
#include <QFormLayout>
#include <QLabel>

UsbConfigPanel::UsbConfigPanel(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("UsbConfigPanel");

    auto* layout = new QFormLayout(this);

    // 设备选择
    m_deviceCombo = new QComboBox(this);
    m_deviceCombo->setObjectName("usbDeviceCombo");
    layout->addRow(tr("USB设备:"), m_deviceCombo);

    // VID输入
    m_vidSpin = new QSpinBox(this);
    m_vidSpin->setObjectName("vidSpin");
    m_vidSpin->setRange(0x0000, 0xFFFF);
    m_vidSpin->setDisplayIntegerBase(16);
    m_vidSpin->setPrefix("0x");
    layout->addRow(tr("VID:"), m_vidSpin);

    // PID输入
    m_pidSpin = new QSpinBox(this);
    m_pidSpin->setObjectName("pidSpin");
    m_pidSpin->setRange(0x0000, 0xFFFF);
    m_pidSpin->setDisplayIntegerBase(16);
    m_pidSpin->setPrefix("0x");
    layout->addRow(tr("PID:"), m_pidSpin);

    // 接口编号
    m_interfaceSpin = new QSpinBox(this);
    m_interfaceSpin->setObjectName("interfaceSpin");
    m_interfaceSpin->setRange(0, 255);
    layout->addRow(tr("接口编号:"), m_interfaceSpin);

    // 连接按钮
    m_connectBtn = new QPushButton(tr("连接"), this);
    m_connectBtn->setObjectName("usbConnectBtn");
    layout->addRow(m_connectBtn);

    // 设备选择联动VID/PID
    connect(m_deviceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
        Q_UNUSED(index)
        // TODO: 从设备列表中提取VID/PID填入
    });
}

#ifndef SERIALCONFIGPANEL_H
#define SERIALCONFIGPANEL_H

#include <QWidget>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QSerialPort>
#include <QLabel>
#include "connection/SerialConnection.h"

// 串口配置面板 - 选择端口、波特率、数据位等参数
class SerialConfigPanel : public QWidget {
    Q_OBJECT

public:
    explicit SerialConfigPanel(QWidget* parent = nullptr);

    // 从界面读取当前配置
    void applyConfigToConnection(SerialConnection* conn);

    // 从连接对象读取配置到界面
    void loadConfigFromConnection(SerialConnection* conn);

    // 刷新可用端口列表
    void refreshPorts();

signals:
    // 用户点击连接/断开按钮
    void connectRequested();
    void disconnectRequested();

private:
    void setupUI();

    QComboBox* m_portCombo;        // 端口选择
    QPushButton* m_refreshBtn;     // 刷新端口列表
    QComboBox* m_baudCombo;        // 波特率
    QComboBox* m_dataBitsCombo;    // 数据位
    QComboBox* m_parityCombo;      // 校验
    QComboBox* m_stopBitsCombo;    // 停止位
    QComboBox* m_flowControlCombo; // 流控
    QCheckBox* m_dtrCheck;         // DTR控制
    QCheckBox* m_rtsCheck;         // RTS控制
    QPushButton* m_connectBtn;     // 连接/断开按钮
};

#endif // SERIALCONFIGPANEL_H

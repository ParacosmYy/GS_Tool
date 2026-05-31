#ifndef SERIALCONFIGPANEL_H
#define SERIALCONFIGPANEL_H

#include <QWidget>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QSerialPort>
#include <QLabel>
#include <QVariantMap>

// 串口配置面板 - 选择端口、波特率、数据位等参数
class SerialConfigPanel : public QWidget {
    Q_OBJECT

public:
    explicit SerialConfigPanel(QWidget* parent = nullptr);

    // 刷新可用端口列表
    void refreshPorts();

    // 获取当前配置值（供 IConnection::configure() 使用，无需强转）
    QString currentPortData() const;
    int currentBaudRate() const;
    int currentDataBitsIndex() const;
    int currentParityIndex() const;
    int currentStopBitsIndex() const;
    int currentFlowControlIndex() const;

    // DTR/RTS状态（连接时读取初始值）
    bool dtrEnabled() const;
    bool rtsEnabled() const;

    // 设置连接状态(更新按钮文字和可用性)
    void setConnected(bool connected);

    // 当前是否处于连接状态
    bool isConnected() const;

    // 从保存的配置恢复到界面
    void restoreConfig(const QVariantMap& config);

signals:
    // 用户点击连接/断开按钮
    void connectRequested();
    void disconnectRequested();

    // DTR/RTS运行时控制
    void dtrChanged(bool enabled);
    void rtsChanged(bool enabled);

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
    bool m_connected = false;       // 当前连接状态
};

#endif // SERIALCONFIGPANEL_H

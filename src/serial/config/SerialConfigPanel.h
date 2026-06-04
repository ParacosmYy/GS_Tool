/**
 * @file SerialConfigPanel.h
 * @brief 串口配置面板 - 端口选择、参数配置、DTR/RTS控制、连接按钮、状态指示器、驱动检测、配置持久化
 * 协作: ConnectionController/MainWindow
 */
#ifndef SERIALCONFIGPANEL_H
#define SERIALCONFIGPANEL_H

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QSpinBox>
#include <QVariantMap>
#include <QSerialPortInfo>

#include "connection/interface/IConnection.h"

class QHBoxLayout; class QAbstractAnimation; class QVBoxLayout; class QGroupBox;
class AnimatedButton;

/** @brief 串口配置面板。状态指示器: 绿(已连接)/黄+呼吸(连接中)/灰(断开)/红(错误) */
class SerialConfigPanel : public QWidget {
    Q_OBJECT

public:
    explicit SerialConfigPanel(QWidget* parent = nullptr);
    void refreshPorts();                   ///< 刷新可用端口列表
    QString currentPortData() const;       ///< 获取当前端口名
    int currentBaudRate() const;           ///< 获取当前波特率
    int currentDataBitsIndex() const;      ///< 获取数据位索引(0=5,1=6,2=7,3=8)
    int currentParityIndex() const;        ///< 获取校验位索引(0=无,1=偶,2=奇)
    int currentStopBitsIndex() const;      ///< 获取停止位索引(0=1,1=1.5,2=2)
    int currentFlowControlIndex() const;   ///< 获取流控索引(0=无,1=硬,2=软)
    bool dtrEnabled() const;               ///< DTR信号状态
    bool rtsEnabled() const;               ///< RTS信号状态
    void setConnected(bool connected);     ///< 设置连接状态(更新按钮+指示器)
    bool isConnected() const;              ///< 查询连接状态
    void restoreConfig(const QVariantMap& config); ///< 从持久化配置恢复
    void setError(const QString& errorMsg); ///< 设置错误状态
    void setConnecting();                  ///< 设置连接中状态

    // ---- 操作统计 ----
    quint64 totalConfigChanges() const { return m_totalConfigChanges; }
    quint64 totalBaudChanges() const { return m_totalBaudChanges; }
    quint64 totalPortSwitches() const { return m_totalPortSwitches; }
    quint64 totalFlowControlToggles() const { return m_totalFlowControlToggles; }
    quint64 totalRefreshPorts() const { return m_totalRefreshPorts; }
    quint64 totalConnectAttempts() const { return m_totalConnectAttempts; }
    quint64 totalConfigLoads() const { return m_totalConfigLoads; }
    quint64 totalConfigSaves() const { return m_totalConfigSaves; }
    void resetStats();

public slots:
    void updatePinoutLeds(const PinoutSignals& pinSignals); ///< 更新信号线LED

signals:
    void connectRequested();
    void disconnectRequested();
    void dtrChanged(bool enabled);
    void rtsChanged(bool enabled);
    void baudRateChanged(qint32 baud);
    void breakRequested(int duration = 100);
    void autoReconnectToggled(bool enabled, int intervalMs);

private slots:
    void onPortComboChanged();

private:
    void setupUI();
    QGroupBox* createPortGroup();
    QGroupBox* createParamGroup();
    void setupSignalAndConnectControls(QVBoxLayout* mainLayout);
    QGroupBox* createControlSignalsGroup();
    void updateDriverInfo();
    void updateConnectButtonState();
    void updateStatusIndicator(const QString& state);
    void stopBreathAnimation();
    void refreshSignalStyle(QPushButton* btn, bool high);
    QString buildPortTooltip(const QSerialPortInfo& info) const;
    QHBoxLayout* createAutoReconnectLayout();

    QComboBox* m_portCombo;       AnimatedButton* m_refreshBtn;
    QComboBox* m_baudCombo, *m_dataBitsCombo, *m_parityCombo, *m_stopBitsCombo, *m_flowControlCombo;
    QPushButton* m_dtrBtn, *m_rtsBtn, *m_breakBtn;
    AnimatedButton* m_connectBtn;
    QLabel* m_driverInfoLbl, *m_statusIndicator;
    QLabel* m_ctsLed, *m_dsrLed, *m_dcdLed, *m_riLed;
    QCheckBox* m_autoReconnectCheck;
    QSpinBox* m_reconnectIntervalSpin;
    bool m_connected = false, m_connecting = false, m_dtrState = true, m_rtsState = true;
    quint64 m_totalConfigChanges = 0, m_totalBaudChanges = 0, m_totalPortSwitches = 0;
    quint64 m_totalFlowControlToggles = 0, m_totalRefreshPorts = 0, m_totalConnectAttempts = 0;
    quint64 m_totalConfigLoads = 0, m_totalConfigSaves = 0;
    QAbstractAnimation* m_breathAnim = nullptr;
};

#endif // SERIALCONFIGPANEL_H

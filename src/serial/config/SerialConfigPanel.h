/** @file SerialConfigPanel.h @brief 串口配置面板 - 端口选择、参数配置、DTR/RTS控制、连接按钮、状态指示器(彩色圆点+呼吸动画)、驱动检测信息、配置持久化。协作: ConnectionController/MainWindow */

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

#include "connection/interface/IConnection.h"  // PinoutSignals

class QHBoxLayout;
class QAbstractAnimation;
class QVBoxLayout;
class QGroupBox;
class QCheckBox;
class QSpinBox;
class AnimatedButton;

/** @brief 串口配置面板。状态指示器: 绿(已连接)/黄+呼吸(连接中)/灰(断开)/红(错误)。DTR/RTS: 连接后可用，绿=HIGH，灰=LOW */
class SerialConfigPanel : public QWidget {
    Q_OBJECT

public:
    explicit SerialConfigPanel(QWidget* parent = nullptr); ///< 构造串口配置面板
    void refreshPorts();                      ///< 刷新可用端口列表
    QString currentPortData() const;          ///< 获取当前端口名(如"COM3")
    int currentBaudRate() const;              ///< 获取当前波特率
    int currentDataBitsIndex() const;         ///< 获取数据位索引(0=5,1=6,2=7,3=8)
    int currentParityIndex() const;           ///< 获取校验位索引(0=无,1=偶,2=奇)
    int currentStopBitsIndex() const;         ///< 获取停止位索引(0=1,1=1.5,2=2)
    int currentFlowControlIndex() const;      ///< 获取流控索引(0=无,1=硬,2=软)
    bool dtrEnabled() const;                  ///< DTR信号状态(true=HIGH)
    bool rtsEnabled() const;                  ///< RTS信号状态(true=HIGH)
    void setConnected(bool connected);        ///< 设置连接状态(更新按钮+指示器)
    bool isConnected() const;                 ///< 查询当前连接状态
    void restoreConfig(const QVariantMap& config); ///< 从持久化配置恢复串口参数
    void setError(const QString& errorMsg);   ///< 设置连接错误状态
    void setConnecting();                     ///< 设置连接中状态(外部调用)


    // ---- Operation statistics ----
    quint64 totalConfigChanges() const { return m_totalConfigChanges; } ///< 累计配置变更次数
    quint64 totalBaudChanges() const { return m_totalBaudChanges; }     ///< 累计波特率变更次数
    quint64 totalPortSwitches() const { return m_totalPortSwitches; }   ///< 累计端口切换次数
    quint64 totalFlowControlToggles() const { return m_totalFlowControlToggles; } ///< 累计流控切换次数
    quint64 totalRefreshPorts() const { return m_totalRefreshPorts; }   ///< 累计端口刷新次数
    quint64 totalConnectAttempts() const { return m_totalConnectAttempts; } ///< 累计连接尝试次数
    void resetStats();                        ///< 重置所有操作统计计数器

public slots:
    void updatePinoutLeds(const PinoutSignals& pinSignals); ///< 更新信号线状态LED指示灯

signals:
    void connectRequested();                  ///< 用户点击连接按钮
    void disconnectRequested();               ///< 用户点击断开按钮
    void dtrChanged(bool enabled);            ///< DTR状态变化(true=HIGH, false=LOW)
    void rtsChanged(bool enabled);            ///< RTS状态变化(true=HIGH, false=LOW)
    void baudRateChanged(qint32 baud);        ///< 运行时波特率变化信号
    void breakRequested(int duration = 100);  ///< Break信号请求(STM32/ESP32进Bootloader)
    void autoReconnectToggled(bool enabled, int intervalMs); ///< 自动重连开关切换

private slots:
    void onPortComboChanged();                ///< 端口下拉框选择变化时更新连接按钮状态

private:
    void setupUI();                           ///< 初始化UI布局(端口+参数+控制信号+连接按钮)
    QGroupBox* createPortGroup();             ///< 创建端口选择区域
    QGroupBox* createParamGroup();            ///< 创建串口参数区域(波特率/数据位/校验/停止位/流控)
    void setupSignalAndConnectControls(QVBoxLayout* mainLayout); ///< 控制信号+驱动检测+连接按钮
    QGroupBox* createControlSignalsGroup();   ///< 创建DTR/RTS控制信号分组
    void updateDriverInfo();                  ///< 根据选中端口更新驱动检测信息(VID/PID/制造商)
    void updateConnectButtonState();          ///< 根据当前状态更新连接按钮文字/样式/可用性
    void updateStatusIndicator(const QString& state); ///< 更新状态指示器颜色property
    void stopBreathAnimation();               ///< 停止呼吸动画并重置透明度特效
    void refreshSignalStyle(QPushButton* btn, bool high); ///< 刷新信号按钮视觉(HIGH=绿, LOW=灰)
    QString buildPortTooltip(const QSerialPortInfo& info) const; ///< 端口详情tooltip(VID/PID/制造商/序列号)
    QHBoxLayout* createAutoReconnectLayout(); ///< 创建自动重连控件布局

    // ---- 控件指针 ----
    QComboBox* m_portCombo;        ///< 端口选择下拉框
    AnimatedButton* m_refreshBtn;   ///< 刷新端口列表按钮(带hover/press动画)
    QComboBox* m_baudCombo;        ///< 波特率选择
    QComboBox* m_dataBitsCombo;    ///< 数据位选择
    QComboBox* m_parityCombo;      ///< 校验位选择
    QComboBox* m_stopBitsCombo;    ///< 停止位选择
    QComboBox* m_flowControlCombo; ///< 流控模式选择
    QPushButton* m_dtrBtn;         ///< DTR信号切换按钮(HIGH/LOW)
    QPushButton* m_rtsBtn;         ///< RTS信号切换按钮(HIGH/LOW)
    QPushButton* m_breakBtn;       ///< Break信号按钮(用于STM32/ESP32进入Bootloader)
    AnimatedButton* m_connectBtn;  ///< 连接/断开按钮(带hover/press动画)
    QLabel* m_driverInfoLbl;       ///< 驱动检测信息标签
    QLabel* m_statusIndicator;     ///< 连接状态指示器(彩色圆点)
    QLabel* m_ctsLed;              ///< CTS信号指示灯
    QLabel* m_dsrLed;              ///< DSR信号指示灯
    QLabel* m_dcdLed;              ///< DCD信号指示灯
    QLabel* m_riLed;               ///< RI信号指示灯
    QCheckBox* m_autoReconnectCheck;  ///< 自动重连开关
    QSpinBox* m_reconnectIntervalSpin;///< 重连间隔(毫秒)

    // ---- 状态标志 ----
    bool m_connected = false;       ///< 当前是否已连接
    bool m_connecting = false;      ///< 正在连接中(防重复点击)
    bool m_dtrState = true;         ///< DTR信号状态(true=HIGH, 默认HIGH)
    bool m_rtsState = true;         ///< RTS信号状态(true=HIGH, 默认HIGH)

    // ---- 操作统计计数器 ----
    quint64 m_totalConfigChanges = 0;       ///< 累计配置变更次数(波特率/数据位/校验/停止位/流控)
    quint64 m_totalBaudChanges = 0;         ///< 累计波特率变更次数
    quint64 m_totalPortSwitches = 0;        ///< 累计端口切换次数
    quint64 m_totalFlowControlToggles = 0;  ///< 累计流控模式切换次数
    quint64 m_totalRefreshPorts = 0;        ///< 累计端口刷新次数
    quint64 m_totalConnectAttempts = 0;     ///< 累计连接尝试次数

    // ---- 呼吸动画 ----
    QAbstractAnimation* m_breathAnim = nullptr;  ///< 连接中状态的呼吸动画(0.3↔1.0循环)
};

#endif // SERIALCONFIGPANEL_H

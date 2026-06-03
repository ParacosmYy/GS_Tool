/**
 * @file SerialConfigPanel.h
 * @brief 串口配置面板 - 串口参数配置、连接控制和状态指示
 *
 * 职责: 端口选择、参数配置、DTR/RTS控制、连接按钮(含连接中中间状态)、
 * 状态指示器(彩色圆点+呼吸动画)、驱动检测信息、配置持久化
 *
 * 协作关系:
 *   - ConnectionController: 接收 connectRequested/disconnectRequested 信号
 *   - MainWindow: 调用 setConnected() 同步连接状态
 *   - setError() / setConnecting() 更新状态指示器颜色
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

#include "connection/interface/IConnection.h"  // PinoutSignals

class QHBoxLayout;
class QAbstractAnimation;
class QVBoxLayout;
class QGroupBox;
class QCheckBox;
class QSpinBox;
class AnimatedButton;

/**
 * @brief 串口配置面板 - 端口选择、参数配置、连接控制和状态指示
 *
 * 状态指示器: 绿色圆点(已连接)、黄色圆点+呼吸动画(连接中)、灰色圆点(断开)、红色圆点(错误)
 * 圆点大小8px，通过QSS的statusIndicator控制，不硬编码颜色。
 * DTR/RTS切换按钮: 连接后可用，绿色=HIGH，灰色=LOW，点击切换电平
 */
class SerialConfigPanel : public QWidget {
    Q_OBJECT

public:
    /** @brief 构造串口配置面板 @param parent 父窗口 */
    explicit SerialConfigPanel(QWidget* parent = nullptr);

    /** @brief 刷新可用端口列表，枚举系统所有串口设备 */
    void refreshPorts();
    /** @brief 获取当前选中的端口系统名 @return 端口名称（如"COM3"） */
    QString currentPortData() const;
    /** @brief 获取当前选中的波特率 @return 波特率值 */
    int currentBaudRate() const;
    /** @brief 获取当前数据位索引 @return 索引（0=5位, 1=6位, 2=7位, 3=8位） */
    int currentDataBitsIndex() const;
    /** @brief 获取当前校验位索引 @return 索引（0=无校验, 1=偶校验, 2=奇校验） */
    int currentParityIndex() const;
    /** @brief 获取当前停止位索引 @return 索引（0=1位, 1=1.5位, 2=2位） */
    int currentStopBitsIndex() const;
    /** @brief 获取当前流控模式索引 @return 索引（0=无流控, 1=硬件流控, 2=软件流控） */
    int currentFlowControlIndex() const;
    /** @brief 查询DTR信号状态 @return true=HIGH, false=LOW */
    bool dtrEnabled() const;
    /** @brief 查询RTS信号状态 @return true=HIGH, false=LOW */
    bool rtsEnabled() const;
    /** @brief 设置连接状态，更新按钮文字和状态指示器 @param connected 是否已连接 */
    void setConnected(bool connected);
    /** @brief 查询当前连接状态 @return true=已连接, false=已断开 */
    bool isConnected() const;
    /** @brief 从持久化配置恢复所有串口参数 @param config 配置键值对 */
    void restoreConfig(const QVariantMap& config);

    /**
     * @brief 设置连接错误状态
     * @param errorMsg 错误信息，显示在状态指示器tooltip中
     */
    void setError(const QString& errorMsg);

    /** @brief 设置连接中状态(由外部连接流程调用) */
    void setConnecting();


    // ---- Operation statistics ----

    /** @brief 获取累计配置变更次数(波特率/数据位/校验/停止位/流控) @return 变更总次数 */
    quint64 totalConfigChanges() const { return m_totalConfigChanges; }

    /** @brief 获取累计端口切换次数 @return 端口切换总次数 */
    quint64 totalPortSwitches() const { return m_totalPortSwitches; }

    /** @brief 获取累计端口刷新次数(refreshPorts调用) @return 端口刷新总次数 */
    quint64 totalRefreshPorts() const { return m_totalRefreshPorts; }

    /** @brief 获取累计连接尝试次数(用户点击连接按钮) @return 连接尝试总次数 */
    quint64 totalConnectAttempts() const { return m_totalConnectAttempts; }

    /** @brief 重置所有操作统计计数器(配置变更/端口切换/端口刷新/连接尝试) */
    void resetStats();

public slots:
    /** @brief 更新信号线状态LED指示灯
     * @param signals 当前信号线电平状态
     */
    void updatePinoutLeds(const PinoutSignals& pinSignals);

signals:
    void connectRequested();      ///< 用户点击连接按钮
    void disconnectRequested();   ///< 用户点击断开按钮
    void dtrChanged(bool enabled);///< DTR状态变化(true=HIGH, false=LOW)
    void rtsChanged(bool enabled);///< RTS状态变化(true=HIGH, false=LOW)

    /** @brief 运行时波特率变化信号(连接后用户更改波特率时发射)
     * @param baud 新的波特率值
     */
    void baudRateChanged(qint32 baud);

    /** @brief Break信号请求(用于STM32/ESP32进入Bootloader)
     * @param duration Break持续时间(毫秒)
     */
    void breakRequested(int duration = 100);

    /** @brief 自动重连开关切换
     * @param enabled 是否启用自动重连
     * @param intervalMs 重连间隔(毫秒)
     */
    void autoReconnectToggled(bool enabled, int intervalMs);

private slots:
    /** @brief 端口下拉框选择变化时更新连接按钮可用状态和驱动信息 */
    void onPortComboChanged();

private:
    /** @brief 初始化UI布局: 端口选择区+参数区+控制信号区+连接按钮 */
    void setupUI();
    /** @brief 创建端口选择区域(端口下拉框+刷新按钮) */
    QGroupBox* createPortGroup();
    /** @brief 创建串口参数区域(波特率/数据位/校验位/停止位/流控) */
    QGroupBox* createParamGroup();
    /** @brief 构建控制信号+驱动检测+连接按钮区域(从setupUI拆分) */
    void setupSignalAndConnectControls(QVBoxLayout* mainLayout);
    /** @brief 创建DTR/RTS控制信号分组(含按钮+信号连接) */
    QGroupBox* createControlSignalsGroup();
    /** @brief 根据选中端口更新驱动检测信息标签（VID/PID/制造商） */
    void updateDriverInfo();
    /** @brief 根据当前状态更新连接按钮的文字、样式和可用性 */
    void updateConnectButtonState();
    /** @brief 更新状态指示器的颜色状态property并刷新样式 */
    void updateStatusIndicator(const QString& state);
    /** @brief 停止呼吸动画并重置透明度特效 */
    void stopBreathAnimation();
    /** @brief 刷新信号按钮视觉状态(HIGH=绿, LOW=灰) */
    void refreshSignalStyle(QPushButton* btn, bool high);
    /** @brief 构建端口详情tooltip(VID/PID/制造商/序列号) */
    QString buildPortTooltip(const QSerialPortInfo& info) const;
    /** @brief 创建自动重连控件布局(复选框+间隔微调框) */
    QHBoxLayout* createAutoReconnectLayout();

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
    quint64 m_totalConfigChanges = 0;  ///< 累计配置变更次数(波特率/数据位/校验/停止位/流控)
    quint64 m_totalPortSwitches = 0;    ///< 累计端口切换次数
    quint64 m_totalRefreshPorts = 0;    ///< 累计端口刷新次数
    quint64 m_totalConnectAttempts = 0; ///< 累计连接尝试次数

    // ---- 呼吸动画 ----
    QAbstractAnimation* m_breathAnim = nullptr;  ///< 连接中状态的呼吸动画(0.3↔1.0循环)
};

#endif // SERIALCONFIGPANEL_H

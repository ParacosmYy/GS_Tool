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
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QVariantMap>

class QAbstractAnimation;

/**
 * @brief 串口配置面板 - 端口选择、参数配置、连接控制和状态指示
 *
 * 状态指示器: 绿色圆点(已连接)、黄色圆点+呼吸动画(连接中)、灰色圆点(断开)、红色圆点(错误)
 * 圆点大小8px，通过QSS的statusIndicator控制，不硬编码颜色。
 */
class SerialConfigPanel : public QWidget {
    Q_OBJECT

public:
    explicit SerialConfigPanel(QWidget* parent = nullptr);

    void refreshPorts();                     ///< 刷新可用端口列表
    QString currentPortData() const;         ///< 获取端口系统名(如COM3)
    int currentBaudRate() const;             ///< 获取波特率
    int currentDataBitsIndex() const;        ///< 数据位索引(0=5,3=8)
    int currentParityIndex() const;          ///< 校验位索引
    int currentStopBitsIndex() const;        ///< 停止位索引
    int currentFlowControlIndex() const;     ///< 流控索引
    bool dtrEnabled() const;                 ///< DTR是否启用
    bool rtsEnabled() const;                 ///< RTS是否启用
    void setConnected(bool connected);       ///< 设置连接状态
    bool isConnected() const;                ///< 当前是否已连接
    void restoreConfig(const QVariantMap& config); ///< 恢复配置

    /**
     * @brief 设置连接错误状态
     * @param errorMsg 错误信息，显示在状态指示器tooltip中
     */
    void setError(const QString& errorMsg);

    /** @brief 设置连接中状态(由外部连接流程调用) */
    void setConnecting();

signals:
    void connectRequested();      ///< 用户点击连接按钮
    void disconnectRequested();   ///< 用户点击断开按钮
    void dtrChanged(bool enabled);///< DTR状态变化
    void rtsChanged(bool enabled);///< RTS状态变化

private slots:
    void onPortComboChanged();    ///< 端口变化时更新按钮状态

private:
    void setupUI();
    void updateDriverInfo();
    void updateConnectButtonState();
    /** @brief 更新状态指示器的颜色状态property并刷新样式 */
    void updateStatusIndicator(const QString& state);
    /** @brief 停止呼吸动画并重置透明度特效 */
    void stopBreathAnimation();

    // ---- 控件指针 ----
    QComboBox* m_portCombo;        ///< 端口选择下拉框
    QPushButton* m_refreshBtn;     ///< 刷新端口列表按钮
    QComboBox* m_baudCombo;        ///< 波特率选择
    QComboBox* m_dataBitsCombo;    ///< 数据位选择
    QComboBox* m_parityCombo;      ///< 校验位选择
    QComboBox* m_stopBitsCombo;    ///< 停止位选择
    QComboBox* m_flowControlCombo; ///< 流控模式选择
    QCheckBox* m_dtrCheck;         ///< DTR信号控制
    QCheckBox* m_rtsCheck;         ///< RTS信号控制
    QPushButton* m_connectBtn;     ///< 连接/断开按钮
    QLabel* m_driverInfoLbl;       ///< 驱动检测信息标签
    QLabel* m_statusIndicator;     ///< 连接状态指示器(彩色圆点)

    // ---- 状态标志 ----
    bool m_connected = false;       ///< 当前是否已连接
    bool m_connecting = false;      ///< 正在连接中(防重复点击)

    // ---- 呼吸动画 ----
    QAbstractAnimation* m_breathAnim = nullptr;  ///< 连接中状态的呼吸动画(0.3↔1.0循环)
};

#endif // SERIALCONFIGPANEL_H

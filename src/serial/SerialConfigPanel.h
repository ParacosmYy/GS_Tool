/**
 * @file SerialConfigPanel.h
 * @brief 串口配置面板 - 提供完整的串口参数配置和连接控制
 *
 * 职责:
 *   1. 端口选择（自动刷新 + 手动刷新）
 *   2. 串口参数配置（波特率/数据位/校验/停止位/流控）
 *   3. DTR/RTS 线路信号控制
 *   4. 连接/断开按钮（带"连接中"中间状态和防重复点击保护）
 *   5. 驱动检测信息展示
 *   6. 配置持久化（保存/恢复全部参数含 DTR/RTS）
 *
 * 协作关系:
 *   - ConnectionController: 接收 connectRequested/disconnectRequested 信号
 *   - MainWindow: 调用 setConnected() 同步连接状态
 */

#ifndef SERIALCONFIGPANEL_H
#define SERIALCONFIGPANEL_H

#include <QWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QVariantMap>

/**
 * @brief 串口配置面板 - 选择端口、波特率、数据位等参数并控制连接
 *
 * 连接按钮有三种视觉状态:
 *   - 默认: "连接" 绿色背景
 *   - 连接中: "连接中..." 黄色背景 + 按钮禁用（防重复点击）
 *   - 已连接: "断开" 红色背景
 * 空端口时自动禁用连接按钮。
 */
class SerialConfigPanel : public QWidget {
    Q_OBJECT

public:
    explicit SerialConfigPanel(QWidget* parent = nullptr);

    /** @brief 刷新可用端口列表，尝试保持当前选择 */
    void refreshPorts();

    // ---- 配置读取接口（供 ConnectionController 构建 QVariantMap） ----

    /** @brief 获取当前选中的端口系统名（如 "COM3"） */
    QString currentPortData() const;

    /** @brief 获取当前波特率值 */
    int currentBaudRate() const;

    /** @brief 获取数据位索引（0=5位, 1=6位, 2=7位, 3=8位） */
    int currentDataBitsIndex() const;

    /** @brief 获取校验位索引（0=无, 1=偶, 2=奇, 3=Mark, 4=Space） */
    int currentParityIndex() const;

    /** @brief 获取停止位索引（0=1, 1=1.5, 2=2） */
    int currentStopBitsIndex() const;

    /** @brief 获取流控索引（0=无, 1=RTS/CTS, 2=XON/XOFF） */
    int currentFlowControlIndex() const;

    /** @brief DTR 信号是否启用 */
    bool dtrEnabled() const;

    /** @brief RTS 信号是否启用 */
    bool rtsEnabled() const;

    /**
     * @brief 设置连接状态（由 MainWindow 在连接成功/失败/断开时调用）
     *
     * 更新按钮文字/颜色、锁定/解锁配置控件。
     * 同时清除"连接中"中间状态。
     * @param connected true=已连接, false=已断开
     */
    void setConnected(bool connected);

    /** @brief 当前是否处于已连接状态 */
    bool isConnected() const;

    /**
     * @brief 从保存的配置恢复到界面
     *
     * 恢复所有参数包括 DTR/RTS 复选框状态。
     * @param config 配置映射表
     */
    void restoreConfig(const QVariantMap& config);

signals:
    /** @brief 用户点击"连接"按钮（已通过空端口检查和防重复点击保护） */
    void connectRequested();

    /** @brief 用户点击"断开"按钮 */
    void disconnectRequested();

    /** @brief DTR 复选框状态变化（运行时实时控制） */
    void dtrChanged(bool enabled);

    /** @brief RTS 复选框状态变化（运行时实时控制） */
    void rtsChanged(bool enabled);

private slots:
    /** @brief 端口列表变化时更新连接按钮可用性 */
    void onPortComboChanged();

private:
    /** @brief 构建 UI 布局和控件 */
    void setupUI();

    /** @brief 更新驱动检测信息标签 */
    void updateDriverInfo();

    /** @brief 根据端口列表是否为空，启用/禁用连接按钮 */
    void updateConnectButtonState();

    // ---- 控件指针 ----

    QComboBox* m_portCombo;        ///< 端口选择下拉框
    QPushButton* m_refreshBtn;     ///< 刷新端口列表按钮
    QComboBox* m_baudCombo;        ///< 波特率选择
    QComboBox* m_dataBitsCombo;    ///< 数据位选择
    QComboBox* m_parityCombo;      ///< 校验位选择
    QComboBox* m_stopBitsCombo;    ///< 停止位选择
    QComboBox* m_flowControlCombo; ///< 流控模式选择
    QCheckBox* m_dtrCheck;         ///< DTR 信号控制
    QCheckBox* m_rtsCheck;         ///< RTS 信号控制
    QPushButton* m_connectBtn;     ///< 连接/断开按钮
    QLabel* m_driverInfoLbl;       ///< 驱动检测信息标签

    // ---- 状态标志 ----

    bool m_connected = false;       ///< 当前是否已连接
    bool m_connecting = false;      ///< 正在连接中（防重复点击保护）
};

#endif // SERIALCONFIGPANEL_H

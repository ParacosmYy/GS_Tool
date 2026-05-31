/**
 * @file SerialConfigPanel.cpp
 * @brief 串口配置面板实现 - 提供完整的串口参数配置和连接控制
 *
 * 面板布局从上到下:
 *   1. 端口选择区（下拉框 + 刷新按钮）
 *   2. 参数配置区（波特率/数据位/校验/停止位/流控）
 *   3. 控制信号区（DTR/RTS 复选框）
 *   4. 驱动检测信息
 *   5. 连接/断开按钮
 */

#include "SerialConfigPanel.h"
#include "SerialDriverDetector.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QFormLayout>
#include <QGroupBox>
#include <QSerialPortInfo>
#include <QIntValidator>

/**
 * @brief 构造串口配置面板
 *
 * 初始化所有 UI 控件并加载当前系统可用端口。
 * 启动时自动调用 refreshPorts() 和 updateDriverInfo()。
 */
SerialConfigPanel::SerialConfigPanel(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    refreshPorts();
    updateDriverInfo();
}

/**
 * @brief 构建 UI 布局
 *
 * 面板使用垂直布局，各功能区用 QGroupBox 分组。
 * 所有控件都设置了 objectName 以支持 QSS 样式选择器。
 */
void SerialConfigPanel::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(12);

    // ---- 端口选择区域 ----
    auto* portGroup = new QGroupBox(tr("端口"));
    portGroup->setObjectName("portGroup");
    auto* portLayout = new QHBoxLayout(portGroup);

    m_portCombo = new QComboBox;
    m_portCombo->setObjectName("portCombo");
    m_portCombo->setMinimumWidth(150);
    // 端口变化时更新连接按钮状态
    connect(m_portCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SerialConfigPanel::onPortComboChanged);

    m_refreshBtn = new QPushButton(tr("刷新"));
    m_refreshBtn->setObjectName("refreshBtn");
    m_refreshBtn->setToolTip(tr("重新扫描系统中的串口设备"));
    connect(m_refreshBtn, &QPushButton::clicked, this, &SerialConfigPanel::refreshPorts);

    portLayout->addWidget(m_portCombo, 1);
    portLayout->addWidget(m_refreshBtn);
    mainLayout->addWidget(portGroup);

    // ---- 串口参数区域 ----
    auto* paramGroup = new QGroupBox(tr("参数"));
    paramGroup->setObjectName("paramGroup");
    auto* formLayout = new QFormLayout(paramGroup);

    // 波特率（可编辑，允许输入自定义值）
    m_baudCombo = new QComboBox;
    m_baudCombo->setObjectName("baudCombo");
    m_baudCombo->setEditable(true);
    m_baudCombo->setToolTip(tr("通信速率(比特/秒)，常用值: 9600, 115200\n可直接输入自定义波特率"));
    QStringList baudRates = {"1200", "2400", "4800", "9600", "19200",
                             "38400", "57600", "115200", "230400",
                             "460800", "921600", "1000000"};
    m_baudCombo->addItems(baudRates);
    m_baudCombo->setCurrentText("115200");
    // 为波特率输入框安装整数校验器，限制范围 300 ~ 10000000
    m_baudCombo->lineEdit()->setValidator(new QIntValidator(300, 10000000, this));
    formLayout->addRow(tr("波特率:"), m_baudCombo);

    // 数据位
    m_dataBitsCombo = new QComboBox;
    m_dataBitsCombo->setObjectName("dataBitsCombo");
    m_dataBitsCombo->setToolTip(tr("每个数据帧的数据位数，绝大多数设备使用 8 位"));
    m_dataBitsCombo->addItems({"5", "6", "7", "8"});
    m_dataBitsCombo->setCurrentIndex(3);  // 默认8
    formLayout->addRow(tr("数据位:"), m_dataBitsCombo);

    // 校验位
    m_parityCombo = new QComboBox;
    m_parityCombo->setObjectName("parityCombo");
    m_parityCombo->setToolTip(tr("校验方式:\n无 - 不校验(最常用)\n偶校验/奇校验 - 简单错误检测"));
    m_parityCombo->addItems({tr("无"), tr("偶校验"), tr("奇校验"),
                              tr("Mark"), tr("Space")});
    formLayout->addRow(tr("校验位:"), m_parityCombo);

    // 停止位
    m_stopBitsCombo = new QComboBox;
    m_stopBitsCombo->setObjectName("stopBitsCombo");
    m_stopBitsCombo->setToolTip(tr("帧结束的停止位数，绝大多数设备使用 1 位"));
    m_stopBitsCombo->addItems({"1", "1.5", "2"});
    formLayout->addRow(tr("停止位:"), m_stopBitsCombo);

    // 流控
    m_flowControlCombo = new QComboBox;
    m_flowControlCombo->setObjectName("flowControlCombo");
    m_flowControlCombo->setToolTip(tr("流量控制:\n无 - 不使用流控(最常用)\nRTS/CTS - 硬件流控\nXON/XOFF - 软件流控"));
    m_flowControlCombo->addItems({tr("无"), tr("RTS/CTS"), tr("XON/XOFF")});
    formLayout->addRow(tr("流控:"), m_flowControlCombo);

    mainLayout->addWidget(paramGroup);

    // ---- 控制信号 ----
    auto* signalGroup = new QGroupBox(tr("控制信号"));
    signalGroup->setObjectName("signalGroup");
    auto* signalLayout = new QHBoxLayout(signalGroup);

    m_dtrCheck = new QCheckBox("DTR");
    m_dtrCheck->setChecked(true);
    m_dtrCheck->setObjectName("dtrCheck");
    m_dtrCheck->setToolTip(tr("数据终端就绪信号，部分设备需要 DTR 拉低才能复位"));
    m_rtsCheck = new QCheckBox("RTS");
    m_rtsCheck->setChecked(true);
    m_rtsCheck->setObjectName("rtsCheck");
    m_rtsCheck->setToolTip(tr("请求发送信号，部分设备需要 RTS 拉低进入 bootloader"));

    signalLayout->addWidget(m_dtrCheck);
    signalLayout->addWidget(m_rtsCheck);
    signalLayout->addStretch();

    // DTR/RTS 运行时控制信号
    connect(m_dtrCheck, &QCheckBox::toggled, this, &SerialConfigPanel::dtrChanged);
    connect(m_rtsCheck, &QCheckBox::toggled, this, &SerialConfigPanel::rtsChanged);
    mainLayout->addWidget(signalGroup);

    // ---- 驱动检测信息 ----
    m_driverInfoLbl = new QLabel;
    m_driverInfoLbl->setObjectName("driverInfoLbl");
    m_driverInfoLbl->setWordWrap(true);
    mainLayout->addWidget(m_driverInfoLbl);

    // ---- 连接按钮 ----
    m_connectBtn = new QPushButton(tr("连接"));
    m_connectBtn->setObjectName("connectBtn");
    m_connectBtn->setMinimumHeight(36);
    m_connectBtn->setToolTip(tr("建立串口连接，快捷键: 无"));
    connect(m_connectBtn, &QPushButton::clicked, this, [this]() {
        if (m_connecting) {
            // 连接中，忽略重复点击
            return;
        }
        if (m_connected) {
            emit disconnectRequested();
        } else {
            // 防重复点击保护: 设置连接中状态
            m_connecting = true;
            m_connectBtn->setEnabled(false);
            m_connectBtn->setText(tr("连接中..."));
            m_connectBtn->setProperty("state", "connecting");
            m_connectBtn->style()->unpolish(m_connectBtn);
            m_connectBtn->style()->polish(m_connectBtn);
            emit connectRequested();
        }
    });
    mainLayout->addWidget(m_connectBtn);

    mainLayout->addStretch();
}

/**
 * @brief 刷新可用端口列表
 *
 * 保留当前选择（如果设备仍在系统中），避免刷新后丢失用户选择。
 * 刷新后更新驱动检测信息和连接按钮状态。
 */
void SerialConfigPanel::refreshPorts()
{
    // 保存当前选择
    QString currentPort = m_portCombo->currentData().toString();
    m_portCombo->clear();

    // 枚举系统中的所有串口
    auto ports = QSerialPortInfo::availablePorts();
    for (const auto& port : ports) {
        QString name = port.portName();
        QString desc = port.description();
        if (!desc.isEmpty()) {
            m_portCombo->addItem(name + " - " + desc, name);
        } else {
            m_portCombo->addItem(name, name);
        }
    }

    // 恢复之前的选择
    if (!currentPort.isEmpty()) {
        int idx = m_portCombo->findData(currentPort);
        if (idx >= 0) {
            m_portCombo->setCurrentIndex(idx);
        }
    }

    updateDriverInfo();
    updateConnectButtonState();
}

/**
 * @brief 设置连接状态
 *
 * 清除"连接中"中间状态，更新按钮文字/颜色和控件可用性。
 * 连接成功后锁定所有配置控件（DTR/RTS 保持可用）。
 *
 * @param connected true=已连接, false=已断开
 */
void SerialConfigPanel::setConnected(bool connected)
{
    m_connected = connected;
    m_connecting = false;  // 清除连接中状态

    m_connectBtn->setEnabled(true);
    m_connectBtn->setText(connected ? tr("断开") : tr("连接"));
    m_connectBtn->setProperty("state", connected ? "connected" : "");
    m_connectBtn->style()->unpolish(m_connectBtn);
    m_connectBtn->style()->polish(m_connectBtn);

    // 锁定/解锁配置控件
    m_portCombo->setEnabled(!connected);
    m_baudCombo->setEnabled(!connected);
    m_dataBitsCombo->setEnabled(!connected);
    m_parityCombo->setEnabled(!connected);
    m_stopBitsCombo->setEnabled(!connected);
    m_flowControlCombo->setEnabled(!connected);
    m_refreshBtn->setEnabled(!connected);
    // DTR/RTS 保持可用，允许连接后实时切换
    m_dtrCheck->setEnabled(true);
    m_rtsCheck->setEnabled(true);

    // 断开后重新检查端口列表，可能需要禁用按钮
    if (!connected) {
        updateConnectButtonState();
    }
}

/** @brief 当前是否处于已连接状态 */
bool SerialConfigPanel::isConnected() const
{
    return m_connected;
}

/** @brief 获取当前选中的端口系统名 */
QString SerialConfigPanel::currentPortData() const
{
    return m_portCombo->currentData().toString();
}

/** @brief 获取当前波特率值 */
int SerialConfigPanel::currentBaudRate() const
{
    return m_baudCombo->currentText().toInt();
}

/** @brief 获取数据位索引 */
int SerialConfigPanel::currentDataBitsIndex() const
{
    return m_dataBitsCombo->currentIndex();
}

/** @brief 获取校验位索引 */
int SerialConfigPanel::currentParityIndex() const
{
    return m_parityCombo->currentIndex();
}

/** @brief 获取停止位索引 */
int SerialConfigPanel::currentStopBitsIndex() const
{
    return m_stopBitsCombo->currentIndex();
}

/** @brief 获取流控索引 */
int SerialConfigPanel::currentFlowControlIndex() const
{
    return m_flowControlCombo->currentIndex();
}

/** @brief DTR 信号是否启用 */
bool SerialConfigPanel::dtrEnabled() const
{
    return m_dtrCheck->isChecked();
}

/** @brief RTS 信号是否启用 */
bool SerialConfigPanel::rtsEnabled() const
{
    return m_rtsCheck->isChecked();
}

/**
 * @brief 从保存的配置恢复到界面
 *
 * 恢复所有参数包括 DTR/RTS 复选框状态。
 * 参数值都做了范围检查，无效值会被安全跳过。
 *
 * @param config 配置映射表
 */
void SerialConfigPanel::restoreConfig(const QVariantMap& config)
{
    if (config.contains("portName")) {
        QString portName = config["portName"].toString();
        int idx = m_portCombo->findData(portName);
        if (idx >= 0) {
            m_portCombo->setCurrentIndex(idx);
        }
    }
    if (config.contains("baudRate")) {
        m_baudCombo->setCurrentText(QString::number(config["baudRate"].toInt()));
    }
    if (config.contains("dataBits")) {
        int idx = config["dataBits"].toInt();
        if (idx >= 0 && idx < m_dataBitsCombo->count()) {
            m_dataBitsCombo->setCurrentIndex(idx);
        }
    }
    if (config.contains("parity")) {
        int idx = config["parity"].toInt();
        if (idx >= 0 && idx < m_parityCombo->count()) {
            m_parityCombo->setCurrentIndex(idx);
        }
    }
    if (config.contains("stopBits")) {
        int idx = config["stopBits"].toInt();
        if (idx >= 0 && idx < m_stopBitsCombo->count()) {
            m_stopBitsCombo->setCurrentIndex(idx);
        }
    }
    if (config.contains("flowControl")) {
        int idx = config["flowControl"].toInt();
        if (idx >= 0 && idx < m_flowControlCombo->count()) {
            m_flowControlCombo->setCurrentIndex(idx);
        }
    }
    // 恢复 DTR/RTS 状态（blockSignals 防止恢复时触发 dtrChanged/rtsChanged 信号
    // 导致实际改变已连接设备的线路状态）
    if (config.contains("dtr")) {
        m_dtrCheck->blockSignals(true);
        m_dtrCheck->setChecked(config["dtr"].toBool());
        m_dtrCheck->blockSignals(false);
    }
    if (config.contains("rts")) {
        m_rtsCheck->blockSignals(true);
        m_rtsCheck->setChecked(config["rts"].toBool());
        m_rtsCheck->blockSignals(false);
    }
}

/** @brief 更新驱动检测信息标签 */
void SerialConfigPanel::updateDriverInfo()
{
    QString summary = SerialDriverDetector::driverStatusSummary();
    m_driverInfoLbl->setText(summary);
}

/**
 * @brief 端口列表变化时的处理
 *
 * 更新连接按钮的可用性（空端口时禁用）。
 */
void SerialConfigPanel::onPortComboChanged()
{
    updateConnectButtonState();
}

/**
 * @brief 根据端口列表是否为空，启用/禁用连接按钮
 *
 * 当没有可用端口时，连接按钮被禁用并显示提示文字。
 */
void SerialConfigPanel::updateConnectButtonState()
{
    if (!m_connected && !m_connecting) {
        bool hasPorts = m_portCombo->count() > 0;
        m_connectBtn->setEnabled(hasPorts);
        if (!hasPorts) {
            m_connectBtn->setText(tr("无端口"));
        } else {
            m_connectBtn->setText(tr("连接"));
        }
    }
}

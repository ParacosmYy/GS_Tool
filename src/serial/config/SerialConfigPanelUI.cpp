/**
 * @file SerialConfigPanelUI.cpp
 * @brief 串口配置面板UI构建方法 - 端口选择、参数配置、连接按钮的布局创建
 *
 * 从 SerialConfigPanel.cpp 拆分而来，包含所有UI构建相关方法:
 *   - setupUI(): 主布局入口
 *   - createPortGroup(): 端口选择区域
 *   - createParamGroup(): 参数配置区域(波特率/数据位/校验位/停止位/流控)
 *   - setupSignalAndConnectControls(): 控制信号+驱动检测+连接按钮区域
 *
 * 控制信号按钮和自动重连布局已拆分至 SerialConfigPanelUISignals.cpp
 */

#include "serial/config/SerialConfigPanel.h"
#include "shared/ConnectionConstants.h"
#include "shared/LayoutConstants.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QIntValidator>
#include <QLineEdit>

#include "core/widgets/AnimatedButton.h"

// ---- UI布局 ----

/** @brief 初始化串口配置面板UI(端口/波特率/数据位/校验/停止位/流控/DTR-RTS) */
void SerialConfigPanel::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(Layout::kPanelPadding, Layout::kPanelPadding,
                                   Layout::kPanelPadding, Layout::kPanelPadding);
    mainLayout->setSpacing(Layout::kPanelSpacing);

    mainLayout->addWidget(createPortGroup());
    mainLayout->addWidget(createParamGroup());
    setupSignalAndConnectControls(mainLayout);
}

/** @brief 创建端口选择区域(端口下拉框+刷新按钮) */
QGroupBox* SerialConfigPanel::createPortGroup()
{
    auto* portGroup = new QGroupBox(tr("端口"));
    portGroup->setObjectName("portGroup");
    auto* portLayout = new QHBoxLayout(portGroup);

    m_portCombo = new QComboBox;
    m_portCombo->setObjectName("portCombo");
    m_portCombo->setMinimumWidth(Layout::kPortComboMinWidth);
    connect(m_portCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SerialConfigPanel::onPortComboChanged);

    m_refreshBtn = new AnimatedButton(tr("刷新"));
    m_refreshBtn->setObjectName("refreshBtn");
    m_refreshBtn->setToolTip(tr("重新扫描系统中的串口设备"));
    connect(m_refreshBtn, &QPushButton::clicked, this, &SerialConfigPanel::refreshPorts);

    portLayout->addWidget(m_portCombo, 1);
    portLayout->addWidget(m_refreshBtn);
    return portGroup;
}

/** @brief 创建串口参数区域(波特率/数据位/校验位/停止位/流控) */
QGroupBox* SerialConfigPanel::createParamGroup()
{
    auto* paramGroup = new QGroupBox(tr("参数"));
    paramGroup->setObjectName("paramGroup");
    auto* formLayout = new QFormLayout(paramGroup);

    m_baudCombo = new QComboBox;
    m_baudCombo->setObjectName("baudCombo");
    m_baudCombo->setEditable(true);
    m_baudCombo->setToolTip(tr("通信速率(比特/秒)，常用值: 9600, 115200\n可直接输入自定义波特率"));
    m_baudCombo->addItems(BaudRates::kStandardRates);
    m_baudCombo->setCurrentIndex(BaudRates::kDefaultBaudIndex);
    m_baudCombo->lineEdit()->setValidator(new QIntValidator(110, 10000000, this));
    // 运行时波特率切换: 连接后用户修改波特率时通知上层
    connect(m_baudCombo, &QComboBox::currentTextChanged, this, [this](const QString& text) {
        if (m_connected) {
            bool ok = false;
            qint32 baud = text.toInt(&ok);
            if (ok && baud > 0) {
                ++m_totalConfigChanges;  ///< 统计: 运行时配置变更
                ++m_totalBaudChanges;    ///< 统计: 波特率变更
                emit baudRateChanged(baud);
            }
        }
    });
    formLayout->addRow(tr("波特率:"), m_baudCombo);

    m_dataBitsCombo = new QComboBox;
    m_dataBitsCombo->setObjectName("dataBitsCombo");
    m_dataBitsCombo->setToolTip(tr("每个数据帧的数据位数，绝大多数设备使用 8 位"));
    m_dataBitsCombo->addItems({tr("5"), tr("6"), tr("7"), tr("8")});
    m_dataBitsCombo->setCurrentIndex(3);
    formLayout->addRow(tr("数据位:"), m_dataBitsCombo);

    m_parityCombo = new QComboBox;
    m_parityCombo->setObjectName("parityCombo");
    m_parityCombo->setToolTip(tr("校验方式:\n无 - 不校验(最常用，适合短距离稳定通信)\n偶校验 - 数据位+校验位1的个数为偶数\n奇校验 - 数据位+校验位1的个数为奇数"));
    m_parityCombo->addItems({tr("无"), tr("偶校验"), tr("奇校验"), tr("Mark"), tr("Space")});
    formLayout->addRow(tr("校验位:"), m_parityCombo);

    m_stopBitsCombo = new QComboBox;
    m_stopBitsCombo->setObjectName("stopBitsCombo");
    m_stopBitsCombo->setToolTip(tr("停止位数:\n1位 - 标准设置(绝大多数设备)\n1.5位 - 极少见\n2位 - 调制解调器/低速通信"));
    m_stopBitsCombo->addItems({tr("1"), tr("1.5"), tr("2")});
    formLayout->addRow(tr("停止位:"), m_stopBitsCombo);

    m_flowControlCombo = new QComboBox;
    m_flowControlCombo->setObjectName("flowControlCombo");
    m_flowControlCombo->setToolTip(tr("流量控制:\n无 - 不使用流控(最常用，短距离无需流控)\nRTS/CTS - 硬件流控(需额外2根信号线，高速通信推荐)\nXON/XOFF - 软件流控(XOFF=0x13暂停, XON=0x11恢复)"));
    m_flowControlCombo->addItems({tr("无"), tr("RTS/CTS"), tr("XON/XOFF")});
    connect(m_flowControlCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int) { ++m_totalFlowControlToggles; });
    formLayout->addRow(tr("流控:"), m_flowControlCombo);

    return paramGroup;
}

/** @brief 构建控制信号(DTR/RTS)+驱动检测信息+连接按钮区域(从setupUI拆分，避免超过80行) @param mainLayout 主布局 */
void SerialConfigPanel::setupSignalAndConnectControls(QVBoxLayout* mainLayout)
{
    // ---- 控制信号 (DTR/RTS) ----
    mainLayout->addWidget(createControlSignalsGroup());

    // ---- 驱动检测信息 ----
    m_driverInfoLbl = new QLabel;
    m_driverInfoLbl->setObjectName("driverInfoLbl");
    m_driverInfoLbl->setWordWrap(true);
    mainLayout->addWidget(m_driverInfoLbl);

    // ---- 连接按钮 + 状态指示器 ----
    auto* connectLayout = new QHBoxLayout;

    m_statusIndicator = new QLabel;
    m_statusIndicator->setObjectName("statusIndicator");
    m_statusIndicator->setFixedSize(8, 8);
    m_statusIndicator->setToolTip(tr("未连接"));
    m_statusIndicator->setProperty("state", "disconnected");
    m_statusIndicator->style()->unpolish(m_statusIndicator);
    m_statusIndicator->style()->polish(m_statusIndicator);

    m_connectBtn = new AnimatedButton(tr("连接"));
    m_connectBtn->setObjectName("connectBtn");
    m_connectBtn->setMinimumHeight(36);
    connect(m_connectBtn, &QPushButton::clicked, this, [this]() {
        if (m_connecting) return;
        if (m_connected) {
            emit disconnectRequested();
        } else {
            ++m_totalConnectAttempts;
            m_connecting = true;
            m_connectBtn->setEnabled(false);
            m_connectBtn->setText(tr("连接中..."));
            m_connectBtn->setProperty("state", "connecting");
            m_connectBtn->style()->unpolish(m_connectBtn);
            m_connectBtn->style()->polish(m_connectBtn);
            emit connectRequested();
        }
    });

    connectLayout->addWidget(m_statusIndicator);
    connectLayout->addWidget(m_connectBtn, 1);
    mainLayout->addLayout(connectLayout);
    mainLayout->addLayout(createAutoReconnectLayout());
}

// ---- 控制信号按钮+自动重连布局见 SerialConfigPanelUISignals.cpp ----

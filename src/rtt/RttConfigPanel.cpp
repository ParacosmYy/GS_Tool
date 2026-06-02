/**
 * @file RttConfigPanel.cpp
 * @brief RTT 配置面板实现 — J-Link RTT 连接参数配置界面
 *
 * 使用 QFormLayout 排列设备、接口、速度、通道等配置项，
 * 底部放置连接/断开操作按钮。
 */

#include "rtt/RttConfigPanel.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

/**
 * @brief 构造函数
 *
 * 初始化面板，设置对象名称并构建 UI 布局。
 *
 * @param parent 父控件
 */
RttConfigPanel::RttConfigPanel(QWidget* parent)
    : QWidget(parent)
    , m_deviceCombo(nullptr)
    , m_interfaceCombo(nullptr)
    , m_speedSpin(nullptr)
    , m_channelSpin(nullptr)
{
    setObjectName(QStringLiteral("RttConfigPanel"));
    setupUI();
}

/**
 * @brief 获取当前配置参数
 *
 * 从各个控件读取当前值，构建参数映射。
 *
 * @return 包含设备/接口/速度/通道等参数的 QVariantMap
 */
QVariantMap RttConfigPanel::config() const
{
    QVariantMap cfg;
    cfg[QStringLiteral("device")] = m_deviceCombo->currentText();
    cfg[QStringLiteral("interface")] = m_interfaceCombo->currentText();
    cfg[QStringLiteral("speed")] = m_speedSpin->value();
    cfg[QStringLiteral("channel")] = m_channelSpin->value();
    return cfg;
}

/**
 * @brief 初始化 UI 布局和控件
 *
 * 创建 QFormLayout，包含：
 *   - 设备选择下拉框（含常用 MCU 型号）
 *   - 调试接口下拉框（JTAG/SWD/cJTAG）
 *   - 连接速度微调框（1000-50000 kHz，默认 4000）
 *   - RTT 通道号微调框（0-15，默认 0）
 *   - 连接/断开按钮
 *
 * 所有控件均设置 objectName 供 QSS 样式匹配。
 */
void RttConfigPanel::setupUI()
{
    auto* mainLayout = new QFormLayout(this);
    mainLayout->setObjectName(QStringLiteral("rttConfigLayout"));
    mainLayout->setLabelAlignment(Qt::AlignRight);

    // ── 设备选择 ──
    m_deviceCombo = new QComboBox(this);
    m_deviceCombo->setObjectName(QStringLiteral("rttDeviceCombo"));
    m_deviceCombo->addItem(tr("Cortex-M0"));
    m_deviceCombo->addItem(tr("Cortex-M0+"));
    m_deviceCombo->addItem(tr("Cortex-M3"));
    m_deviceCombo->addItem(tr("Cortex-M4"));
    m_deviceCombo->addItem(tr("Cortex-M7"));
    m_deviceCombo->addItem(tr("Cortex-M33"));
    m_deviceCombo->setEditable(true);
    m_deviceCombo->setToolTip(tr("目标 MCU 设备型号"));
    mainLayout->addRow(tr("设备:"), m_deviceCombo);

    // ── 调试接口 ──
    m_interfaceCombo = new QComboBox(this);
    m_interfaceCombo->setObjectName(QStringLiteral("rttInterfaceCombo"));
    m_interfaceCombo->addItem(QStringLiteral("JTAG"));
    m_interfaceCombo->addItem(QStringLiteral("SWD"));
    m_interfaceCombo->addItem(QStringLiteral("cJTAG"));
    m_interfaceCombo->setCurrentText(QStringLiteral("SWD"));
    m_interfaceCombo->setToolTip(tr("调试接口类型"));
    mainLayout->addRow(tr("接口:"), m_interfaceCombo);

    // ── 连接速度 ──
    m_speedSpin = new QSpinBox(this);
    m_speedSpin->setObjectName(QStringLiteral("rttSpeedSpin"));
    m_speedSpin->setRange(1000, 50000);
    m_speedSpin->setSingleStep(1000);
    m_speedSpin->setValue(4000);
    m_speedSpin->setSuffix(QStringLiteral(" kHz"));
    m_speedSpin->setToolTip(tr("J-Link 连接速度 (kHz)"));
    mainLayout->addRow(tr("速度:"), m_speedSpin);

    // ── RTT 通道号 ──
    m_channelSpin = new QSpinBox(this);
    m_channelSpin->setObjectName(QStringLiteral("rttChannelSpin"));
    m_channelSpin->setRange(0, 15);
    m_channelSpin->setValue(0);
    m_channelSpin->setToolTip(tr("RTT 通道号 (0-15)"));
    mainLayout->addRow(tr("通道:"), m_channelSpin);

    // ── 操作按钮 ──
    auto* btnLayout = new QHBoxLayout();
    btnLayout->setObjectName(QStringLiteral("rttBtnLayout"));

    auto* connectBtn = new QPushButton(tr("连接"), this);
    connectBtn->setObjectName(QStringLiteral("rttConnectBtn"));
    connectBtn->setToolTip(tr("连接 J-Link 并启动 RTT"));

    auto* disconnectBtn = new QPushButton(tr("断开"), this);
    disconnectBtn->setObjectName(QStringLiteral("rttDisconnectBtn"));
    disconnectBtn->setToolTip(tr("断开 J-Link RTT 连接"));

    btnLayout->addStretch();
    btnLayout->addWidget(connectBtn);
    btnLayout->addWidget(disconnectBtn);

    mainLayout->addRow(QString(), btnLayout);

    // ── 信号连接 ──
    connect(connectBtn, &QPushButton::clicked,
            this, &RttConfigPanel::connectRequested);

    connect(disconnectBtn, &QPushButton::clicked,
            this, &RttConfigPanel::disconnectRequested);

    // 表单值变化 → 聚合发出 configChanged
    connect(m_deviceCombo, &QComboBox::currentTextChanged,
            this, &RttConfigPanel::onFormValueChanged);
    connect(m_interfaceCombo, &QComboBox::currentTextChanged,
            this, &RttConfigPanel::onFormValueChanged);
    connect(m_speedSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &RttConfigPanel::onFormValueChanged);
    connect(m_channelSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &RttConfigPanel::onFormValueChanged);
}

/**
 * @brief 表单值变化时的统一处理
 *
 * 任意配置项变化时，收集当前值并发出 configChanged 信号。
 */
void RttConfigPanel::onFormValueChanged()
{
    emit configChanged(config());
}

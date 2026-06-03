/**
 * @file CanConfigPanel.cpp
 * @brief CAN配置面板实现 — 适配器选择、波特率组合、位时序、帧过滤、连接切换
 */

#include "connection/can/CanConfigPanel.h"
#include <QFormLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QSerialPortInfo>

/** @brief 构造CAN配置面板，初始化适配器列表/波特率/位时序/过滤器/连接按钮 @param parent 父控件 */
CanConfigPanel::CanConfigPanel(QWidget* parent)
    : QWidget(parent)
    , m_adapterCombo(new QComboBox(this))
    , m_bitrateCombo(new QComboBox(this))
    , m_canFdCheck(new QCheckBox(tr("启用 CAN-FD"), this))
    , m_connectBtn(new QPushButton(tr("连接"), this))
    , m_statusLabel(new QLabel(tr("未连接"), this))
    , m_samplePointSpin(new QDoubleSpinBox(this))
    , m_sjwSpin(new QSpinBox(this))
    , m_filterIdEdit(new QLineEdit(this))
    , m_filterMaskEdit(new QLineEdit(this))
    , m_filterExtCheck(new QCheckBox(tr("扩展帧"), this))
{
    setObjectName("CanConfigPanel");

    /* ── 适配器选择 ── */
    m_adapterCombo->setObjectName("canAdapterCombo");
    const auto ports = QSerialPortInfo::availablePorts();
    for (const auto& info : ports) {
        m_adapterCombo->addItem(info.portName() + " - " + info.description(),
                                info.portName());
    }

    /* ── 波特率: 125k, 250k, 500k(默认), 800k, 1M ── */
    m_bitrateCombo->setObjectName("canBitrateCombo");
    struct BitrateEntry { QString label; int value; };
    const BitrateEntry bitrates[] = {
        {tr("125 kbps"), 125000}, {tr("250 kbps"), 250000},
        {tr("500 kbps"), 500000}, {tr("800 kbps"), 800000},
        {tr("1 Mbps"), 1000000}
    };
    for (const auto& e : bitrates) {
        m_bitrateCombo->addItem(e.label, e.value);
    }
    m_bitrateCombo->setCurrentIndex(2);  // 默认500kbps

    /* ── 位时序配置 ── */
    m_samplePointSpin->setObjectName("canSamplePointSpin");
    m_samplePointSpin->setRange(0.50, 0.95);
    m_samplePointSpin->setSingleStep(0.005);
    m_samplePointSpin->setDecimals(3);
    m_samplePointSpin->setValue(0.875);
    m_samplePointSpin->setToolTip(tr("采样点位置(推荐87.5%)"));

    m_sjwSpin->setObjectName("canSjwSpin");
    m_sjwSpin->setRange(1, 4);
    m_sjwSpin->setValue(1);
    m_sjwSpin->setToolTip(tr("同步跳转宽度(1~4)"));

    /* ── 帧过滤器配置 ── */
    m_filterIdEdit->setObjectName("canFilterIdEdit");
    m_filterIdEdit->setPlaceholderText(tr("如: 0x123"));
    m_filterIdEdit->setMaximumWidth(100);
    m_filterMaskEdit->setObjectName("canFilterMaskEdit");
    m_filterMaskEdit->setPlaceholderText(tr("如: 0x7FF"));
    m_filterMaskEdit->setMaximumWidth(100);
    m_filterExtCheck->setObjectName("canFilterExtCheck");

    /* ── 连接/断开按钮 + 状态 ── */
    m_canFdCheck->setObjectName("canFdCheck");
    m_connectBtn->setObjectName("canConnectBtn");
    m_statusLabel->setObjectName("canStatusLabel");
    auto connectLayout = new QHBoxLayout();
    connectLayout->addWidget(m_canFdCheck);
    connectLayout->addStretch();
    connectLayout->addWidget(m_statusLabel);
    connectLayout->addWidget(m_connectBtn);

    /* ── 主表单布局 ── */
    auto form = new QFormLayout(this);
    form->addRow(tr("CAN适配器:"), m_adapterCombo);
    form->addRow(tr("波特率:"), m_bitrateCombo);
    form->addRow(tr("采样点:"), m_samplePointSpin);
    form->addRow(tr("SJW:"), m_sjwSpin);
    form->addRow(connectLayout);

    /* ── 帧过滤器分组 ── */
    auto filterGroup = new QGroupBox(tr("帧过滤器"));
    filterGroup->setObjectName("canFilterGroup");
    auto filterLayout = new QHBoxLayout(filterGroup);
    filterLayout->addWidget(new QLabel(tr("ID:")));
    filterLayout->addWidget(m_filterIdEdit);
    filterLayout->addWidget(new QLabel(tr("掩码:")));
    filterLayout->addWidget(m_filterMaskEdit);
    filterLayout->addWidget(m_filterExtCheck);
    form->addRow(filterGroup);

    /* ── 按钮点击 → 根据当前状态发送不同信号 ── */
    connect(m_connectBtn, &QPushButton::clicked, this, [this]() {
        if (m_connected) {
            ++m_totalBusResets;
            emit disconnectRequested();
        } else {
            emit connectRequested();
        }
    });

    /* ── 配置变更计数 ── */
    auto countChange = [this]() { ++m_totalConfigChanges; };
    connect(m_adapterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, countChange);
    connect(m_bitrateCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, countChange);
    connect(m_canFdCheck, &QCheckBox::stateChanged, this, countChange);
    connect(m_samplePointSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, countChange);
    connect(m_sjwSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, countChange);
    connect(m_filterIdEdit, &QLineEdit::textChanged, this, countChange);
    connect(m_filterMaskEdit, &QLineEdit::textChanged, this, countChange);
}

/** @brief 获取当前CAN配置参数 @return 包含adapter/bitrate/canFd/samplePoint/sjw/filterId/filterMask字段的QVariantMap */
QVariantMap CanConfigPanel::config() const
{
    QVariantMap cfg;
    cfg["adapter"] = m_adapterCombo->currentData().toString();
    cfg["bitrate"] = m_bitrateCombo->currentData().toUInt();
    cfg["canFd"] = m_canFdCheck->isChecked();
    cfg["samplePoint"] = m_samplePointSpin->value();
    cfg["sjw"] = m_sjwSpin->value();

    /* 帧过滤器: 仅当ID和掩码都填写时才包含 */
    const QString filterId = m_filterIdEdit->text().trimmed();
    const QString filterMask = m_filterMaskEdit->text().trimmed();
    if (!filterId.isEmpty() && !filterMask.isEmpty()) {
        cfg["filterId"] = filterId.toUInt(nullptr, 0);
        cfg["filterMask"] = filterMask.toUInt(nullptr, 0);
        cfg["filterExtended"] = m_filterExtCheck->isChecked();
    }

    return cfg;
}

/** @brief 设置连接状态，更新按钮文本和控件可用性 @param connected true=已连接 */
void CanConfigPanel::setConnected(bool connected)
{
    m_connected = connected;
    m_connectBtn->setText(connected ? tr("断开") : tr("连接"));
    m_statusLabel->setText(connected ? tr("已连接") : tr("未连接"));
    m_adapterCombo->setEnabled(!connected);
    m_bitrateCombo->setEnabled(!connected);
    m_samplePointSpin->setEnabled(!connected);
    m_sjwSpin->setEnabled(!connected);
}

/** @brief 保存CAN配置到QSettings @param settings QSettings对象 */
void CanConfigPanel::saveSettings(QSettings& settings) const
{
    settings.setValue(QStringLiteral("can/adapter"),
                     m_adapterCombo->currentData().toString());
    settings.setValue(QStringLiteral("can/bitrateIndex"),
                     m_bitrateCombo->currentIndex());
    settings.setValue(QStringLiteral("can/canFd"),
                     m_canFdCheck->isChecked());
    settings.setValue(QStringLiteral("can/samplePoint"),
                     m_samplePointSpin->value());
    settings.setValue(QStringLiteral("can/sjw"),
                     m_sjwSpin->value());
    settings.setValue(QStringLiteral("can/filterId"),
                     m_filterIdEdit->text());
    settings.setValue(QStringLiteral("can/filterMask"),
                     m_filterMaskEdit->text());
    settings.setValue(QStringLiteral("can/filterExt"),
                     m_filterExtCheck->isChecked());
}

/** @brief 从QSettings加载CAN配置 @param settings QSettings对象 */
void CanConfigPanel::loadSettings(QSettings& settings)
{
    const QString adapter = settings.value(
        QStringLiteral("can/adapter")).toString();
    if (!adapter.isEmpty()) {
        const int idx = m_adapterCombo->findData(adapter);
        if (idx >= 0) {
            m_adapterCombo->setCurrentIndex(idx);
        }
    }
    m_bitrateCombo->setCurrentIndex(
        settings.value(QStringLiteral("can/bitrateIndex"), 2).toInt());
    m_canFdCheck->setChecked(
        settings.value(QStringLiteral("can/canFd"), false).toBool());
    m_samplePointSpin->setValue(
        settings.value(QStringLiteral("can/samplePoint"), 0.875).toDouble());
    m_sjwSpin->setValue(
        settings.value(QStringLiteral("can/sjw"), 1).toInt());
    m_filterIdEdit->setText(
        settings.value(QStringLiteral("can/filterId")).toString());
    m_filterMaskEdit->setText(
        settings.value(QStringLiteral("can/filterMask")).toString());
    m_filterExtCheck->setChecked(
        settings.value(QStringLiteral("can/filterExt"), false).toBool());
}

/** @brief 重置所有统计计数器 */
void CanConfigPanel::resetStatistics()
{
    m_totalConfigChanges = 0;
    m_totalBusResets = 0;
}

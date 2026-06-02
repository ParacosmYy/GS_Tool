/**
 * @file SpiI2cConfigPanel.cpp
 * @brief SPI/I2C配置面板实现
 */

#include "connection/spi_i2c/SpiI2cConfigPanel.h"
#include <QFormLayout>

/**
 * @brief 构造函数 - 初始化UI
 * @param parent 父控件
 */
SpiI2cConfigPanel::SpiI2cConfigPanel(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("SpiI2cConfigPanel");
    setupUi();
    setupConnections();
    updateModeVisibility();
}

/**
 * @brief 获取当前配置参数
 * @return 配置键值对
 */
QVariantMap SpiI2cConfigPanel::config() const
{
    QVariantMap cfg;
    cfg["busMode"] = m_currentMode;

    if (m_adapterCombo) {
        cfg["adapter"] = m_adapterCombo->currentText();
    }
    if (m_clockSpin) {
        cfg["clockSpeed"] = m_clockSpin->value();
    }

    if (m_currentMode == "spi") {
        if (m_spiModeCombo) {
            cfg["spiMode"] = m_spiModeCombo->currentIndex();
        }
        if (m_csPinSpin) {
            cfg["csPin"] = m_csPinSpin->value();
        }
    } else {
        if (m_deviceAddrSpin) {
            cfg["deviceAddress"] = m_deviceAddrSpin->value();
        }
    }
    return cfg;
}

/**
 * @brief 设置当前模式
 * @param mode "spi" 或 "i2c"
 */
void SpiI2cConfigPanel::setMode(const QString& mode)
{
    m_currentMode = mode;
    /// 同步下拉框(阻止信号防止递归触发onModeChanged)
    if (m_busModeCombo) {
        m_busModeCombo->blockSignals(true);
        int idx = (mode == "i2c") ? 1 : 0;
        m_busModeCombo->setCurrentIndex(idx);
        m_busModeCombo->blockSignals(false);
    }
    updateModeVisibility();
}

/**
 * @brief 设置连接状态(由外部连接管理器调用)
 * @param connected true=已连接
 */
void SpiI2cConfigPanel::setConnected(bool connected)
{
    m_connected = connected;
    if (m_connectBtn) {
        m_connectBtn->setText(connected ? tr("断开") : tr("连接"));
    }
    if (m_statusLabel) {
        m_statusLabel->setText(connected ? tr("已连接") : tr("未连接"));
    }
}

/**
 * @brief 连接按钮点击
 */
void SpiI2cConfigPanel::onConnectClicked()
{
    if (m_connected) {
        /// 断开连接
        m_connected = false;
        if (m_connectBtn) {
            m_connectBtn->setText(tr("连接"));
        }
        if (m_statusLabel) {
            m_statusLabel->setText(tr("已断开"));
        }
        emit disconnectRequested();
    } else {
        /// 发起连接
        emit connectRequested(config());
    }
}

/**
 * @brief 模式切换(SPI/I2C)回调
 * @param index 下拉框当前索引
 */
void SpiI2cConfigPanel::onModeChanged(int index)
{
    m_currentMode = (index == 1) ? "i2c" : "spi";
    updateModeVisibility();
}

/**
 * @brief 初始化UI布局
 */
void SpiI2cConfigPanel::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);

    /// 总线模式选择
    auto* modeLayout = new QHBoxLayout();
    auto* modeLabel = new QLabel(tr("总线模式:"), this);
    modeLabel->setObjectName("busModeLabel");

    m_busModeCombo = new QComboBox(this);
    m_busModeCombo->setObjectName("busModeCombo");
    m_busModeCombo->addItem(tr("SPI"));
    m_busModeCombo->addItem(tr("I2C"));

    modeLayout->addWidget(modeLabel);
    modeLayout->addWidget(m_busModeCombo);
    mainLayout->addLayout(modeLayout);

    /// 适配器选择
    auto* adapterLayout = new QHBoxLayout();
    auto* adapterLabel = new QLabel(tr("适配器:"), this);
    adapterLabel->setObjectName("adapterLabel");

    m_adapterCombo = new QComboBox(this);
    m_adapterCombo->setObjectName("adapterCombo");
    m_adapterCombo->addItem(tr("FT232H"));
    m_adapterCombo->addItem(tr("CH347"));
    m_adapterCombo->addItem(tr("CP2130"));

    adapterLayout->addWidget(adapterLabel);
    adapterLayout->addWidget(m_adapterCombo);
    mainLayout->addLayout(adapterLayout);

    /// 时钟频率
    auto* clockLayout = new QHBoxLayout();
    auto* clockLabel = new QLabel(tr("时钟频率:"), this);
    clockLabel->setObjectName("clockLabel");

    m_clockSpin = new QSpinBox(this);
    m_clockSpin->setObjectName("clockSpin");
    m_clockSpin->setRange(1, 50000000);
    m_clockSpin->setValue(1000000);
    m_clockSpin->setSuffix(tr(" Hz"));
    m_clockSpin->setSingleStep(100000);

    clockLayout->addWidget(clockLabel);
    clockLayout->addWidget(m_clockSpin);
    mainLayout->addLayout(clockLayout);

    /// ---- SPI参数分组 ----
    m_spiGroup = new QGroupBox(tr("SPI参数"), this);
    m_spiGroup->setObjectName("spiGroup");
    auto* spiLayout = new QFormLayout(m_spiGroup);

    m_spiModeCombo = new QComboBox(this);
    m_spiModeCombo->setObjectName("spiModeCombo");
    m_spiModeCombo->addItem(tr("Mode 0 (CPOL=0, CPHA=0)"));
    m_spiModeCombo->addItem(tr("Mode 1 (CPOL=0, CPHA=1)"));
    m_spiModeCombo->addItem(tr("Mode 2 (CPOL=1, CPHA=0)"));
    m_spiModeCombo->addItem(tr("Mode 3 (CPOL=1, CPHA=1)"));
    spiLayout->addRow(tr("SPI模式:"), m_spiModeCombo);

    m_csPinSpin = new QSpinBox(this);
    m_csPinSpin->setObjectName("csPinSpin");
    m_csPinSpin->setRange(0, 15);
    m_csPinSpin->setValue(0);
    spiLayout->addRow(tr("片选引脚:"), m_csPinSpin);

    mainLayout->addWidget(m_spiGroup);

    /// ---- I2C参数分组 ----
    m_i2cGroup = new QGroupBox(tr("I2C参数"), this);
    m_i2cGroup->setObjectName("i2cGroup");
    auto* i2cLayout = new QFormLayout(m_i2cGroup);

    m_deviceAddrSpin = new QSpinBox(this);
    m_deviceAddrSpin->setObjectName("deviceAddrSpin");
    m_deviceAddrSpin->setRange(0x00, 0x7F);
    m_deviceAddrSpin->setValue(0x00);
    m_deviceAddrSpin->setDisplayIntegerBase(16);
    m_deviceAddrSpin->setPrefix("0x");
    i2cLayout->addRow(tr("设备地址:"), m_deviceAddrSpin);

    mainLayout->addWidget(m_i2cGroup);

    /// 连接按钮
    m_connectBtn = new QPushButton(tr("连接"), this);
    m_connectBtn->setObjectName("connectBtn");
    mainLayout->addWidget(m_connectBtn);

    /// 状态标签
    m_statusLabel = new QLabel(tr("未连接"), this);
    m_statusLabel->setObjectName("statusLabel");
    mainLayout->addWidget(m_statusLabel);
}

/**
 * @brief 初始化信号连接
 */
void SpiI2cConfigPanel::setupConnections()
{
    connect(m_connectBtn, &QPushButton::clicked,
            this, &SpiI2cConfigPanel::onConnectClicked);
    connect(m_busModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SpiI2cConfigPanel::onModeChanged);
}

/**
 * @brief 根据模式更新UI可见性
 */
void SpiI2cConfigPanel::updateModeVisibility()
{
    if (m_spiGroup) {
        m_spiGroup->setVisible(m_currentMode == "spi");
    }
    if (m_i2cGroup) {
        m_i2cGroup->setVisible(m_currentMode == "i2c");
    }
}

/**
 * @file SpiI2cConfigPanelUI.cpp
 * @brief SPI/I2C配置面板 — UI构建与信号连接方法
 *
 * 从 SpiI2cConfigPanel.cpp 拆分而来，包含:
 *   - setupUi(): 初始化UI布局(模式选择/适配器/时钟/SPI参数/I2C参数/连接按钮)
 *   - setupConnections(): 初始化信号连接
 *   - updateModeVisibility(): 根据模式动态显示/隐藏参数分组
 *
 * 面板交互逻辑/配置访问器保留在 SpiI2cConfigPanel.cpp。
 * saveSettings/loadSettings/resetStatistics见 SpiI2cConfigPanelPersist.cpp。
 */

#include "connection/spi_i2c/SpiI2cConfigPanel.h"
#include <QFormLayout>

/** @brief 初始化UI布局: 模式选择/适配器/时钟/SPI参数组/I2C参数组/连接按钮/状态标签 */
void SpiI2cConfigPanel::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);

    /// 总线模式选择
    auto* modeLayout = new QHBoxLayout();
    auto* modeLabel = new QLabel(tr("总线模式:"), this);
    modeLabel->setObjectName("spiI2cBusModeLabel");

    m_busModeCombo = new QComboBox(this);
    m_busModeCombo->setObjectName("spiI2cBusModeCombo");
    m_busModeCombo->addItem(tr("SPI"));
    m_busModeCombo->addItem(tr("I2C"));

    modeLayout->addWidget(modeLabel);
    modeLayout->addWidget(m_busModeCombo);
    mainLayout->addLayout(modeLayout);

    /// 适配器选择
    auto* adapterLayout = new QHBoxLayout();
    auto* adapterLabel = new QLabel(tr("适配器:"), this);
    adapterLabel->setObjectName("spiI2cAdapterLabel");

    m_adapterCombo = new QComboBox(this);
    m_adapterCombo->setObjectName("spiI2cAdapterCombo");
    m_adapterCombo->addItem(tr("FT232H"));
    m_adapterCombo->addItem(tr("CH347"));
    m_adapterCombo->addItem(tr("CP2130"));

    adapterLayout->addWidget(adapterLabel);
    adapterLayout->addWidget(m_adapterCombo);
    mainLayout->addLayout(adapterLayout);

    /// 时钟频率
    auto* clockLayout = new QHBoxLayout();
    auto* clockLabel = new QLabel(tr("时钟频率:"), this);
    clockLabel->setObjectName("spiI2cClockLabel");

    m_clockSpin = new QSpinBox(this);
    m_clockSpin->setObjectName("spiI2cClockSpin");
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
    m_spiModeCombo->setObjectName("spiI2cSpiModeCombo");
    m_spiModeCombo->addItem(tr("Mode 0 (CPOL=0, CPHA=0)"));
    m_spiModeCombo->addItem(tr("Mode 1 (CPOL=0, CPHA=1)"));
    m_spiModeCombo->addItem(tr("Mode 2 (CPOL=1, CPHA=0)"));
    m_spiModeCombo->addItem(tr("Mode 3 (CPOL=1, CPHA=1)"));
    spiLayout->addRow(tr("SPI模式:"), m_spiModeCombo);

    m_csPinSpin = new QSpinBox(this);
    m_csPinSpin->setObjectName("spiI2cCsPinSpin");
    m_csPinSpin->setRange(0, 15);
    m_csPinSpin->setValue(0);
    spiLayout->addRow(tr("片选引脚:"), m_csPinSpin);

    mainLayout->addWidget(m_spiGroup);

    /// ---- I2C参数分组 ----
    m_i2cGroup = new QGroupBox(tr("I2C参数"), this);
    m_i2cGroup->setObjectName("i2cGroup");
    auto* i2cLayout = new QFormLayout(m_i2cGroup);

    m_deviceAddrSpin = new QSpinBox(this);
    m_deviceAddrSpin->setObjectName("spiI2cDeviceAddrSpin");
    m_deviceAddrSpin->setRange(0x00, 0x7F);
    m_deviceAddrSpin->setValue(0x00);
    m_deviceAddrSpin->setDisplayIntegerBase(16);
    m_deviceAddrSpin->setPrefix("0x");
    i2cLayout->addRow(tr("设备地址:"), m_deviceAddrSpin);

    mainLayout->addWidget(m_i2cGroup);

    /// 连接按钮
    m_connectBtn = new QPushButton(tr("连接"), this);
    m_connectBtn->setObjectName("spiI2cConnectBtn");
    mainLayout->addWidget(m_connectBtn);

    /// 状态标签
    m_statusLabel = new QLabel(tr("未连接"), this);
    m_statusLabel->setObjectName("spiI2cStatusLabel");
    mainLayout->addWidget(m_statusLabel);
}

/** @brief 初始化信号连接: 连接按钮/模式切换/配置变更 */
void SpiI2cConfigPanel::setupConnections()
{
    connect(m_connectBtn, &QPushButton::clicked,
            this, &SpiI2cConfigPanel::onConnectClicked);
    connect(m_busModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SpiI2cConfigPanel::onModeChanged);
    connect(m_busModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this]() { ++m_totalConfigChanges; });
    connect(m_adapterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this]() { ++m_totalConfigChanges; });
    connect(m_clockSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this]() { ++m_totalConfigChanges; });
}

/** @brief 根据当前模式(SPI/I2C)动态显示/隐藏对应参数分组 */
void SpiI2cConfigPanel::updateModeVisibility()
{
    if (m_spiGroup) {
        m_spiGroup->setVisible(m_currentMode == "spi");
    }
    if (m_i2cGroup) {
        m_i2cGroup->setVisible(m_currentMode == "i2c");
    }
}

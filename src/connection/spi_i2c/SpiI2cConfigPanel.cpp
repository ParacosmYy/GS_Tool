/**
 * @file SpiI2cConfigPanel.cpp
 * @brief SPI/I2C配置面板实现 - 骨架
 */

#include "connection/spi_i2c/SpiI2cConfigPanel.h"

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
}

/**
 * @brief 获取当前配置参数
 * @return 配置键值对
 */
QVariantMap SpiI2cConfigPanel::config() const
{
    QVariantMap cfg;
    cfg["mode"] = m_currentMode;
    if (m_adapterCombo) {
        cfg["adapter"] = m_adapterCombo->currentText();
    }
    if (m_clockSpin) {
        cfg["clockSpeed"] = m_clockSpin->value();
    }
    if (m_modeCombo) {
        cfg["spiMode"] = m_modeCombo->currentIndex();
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
    updateModeVisibility();
}

/**
 * @brief 连接按钮点击
 */
void SpiI2cConfigPanel::onConnectClicked()
{
    // TODO: 根据当前模式创建对应连接并打开
}

/**
 * @brief 初始化UI布局
 */
void SpiI2cConfigPanel::setupUi()
{
    auto* layout = new QVBoxLayout(this);

    // 适配器选择
    m_adapterCombo = new QComboBox(this);
    m_adapterCombo->setObjectName("adapterCombo");
    m_adapterCombo->addItem(tr("FT232H"));
    m_adapterCombo->addItem(tr("CH347"));

    // 时钟频率
    m_clockSpin = new QSpinBox(this);
    m_clockSpin->setObjectName("clockSpin");
    m_clockSpin->setRange(1, 50000000);
    m_clockSpin->setValue(1000000);
    m_clockSpin->setSuffix(tr(" Hz"));
    m_clockSpin->setSingleStep(100000);

    // 模式选择
    m_modeCombo = new QComboBox(this);
    m_modeCombo->setObjectName("modeCombo");
    m_modeCombo->addItem(tr("Mode 0 (CPOL=0, CPHA=0)"));
    m_modeCombo->addItem(tr("Mode 1 (CPOL=0, CPHA=1)"));
    m_modeCombo->addItem(tr("Mode 2 (CPOL=1, CPHA=0)"));
    m_modeCombo->addItem(tr("Mode 3 (CPOL=1, CPHA=1)"));

    // 连接按钮
    m_connectBtn = new QPushButton(tr("连接"), this);
    m_connectBtn->setObjectName("connectBtn");

    layout->addWidget(m_adapterCombo);
    layout->addWidget(m_clockSpin);
    layout->addWidget(m_modeCombo);
    layout->addWidget(m_connectBtn);
}

/**
 * @brief 初始化信号连接
 */
void SpiI2cConfigPanel::setupConnections()
{
    connect(m_connectBtn, &QPushButton::clicked,
            this, &SpiI2cConfigPanel::onConnectClicked);
}

/**
 * @brief 根据模式更新UI可见性
 */
void SpiI2cConfigPanel::updateModeVisibility()
{
    // TODO: I2C模式时隐藏SPI模式选择，显示设备地址等
    if (m_currentMode == "i2c") {
        if (m_modeCombo) m_modeCombo->setVisible(false);
    } else {
        if (m_modeCombo) m_modeCombo->setVisible(true);
    }
}

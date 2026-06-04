/**
 * @file UsbConfigPanel.cpp
 * @brief USB配置面板实现
 */
#include "connection/usb/UsbConfigPanel.h"
#include "connection/usb/UsbDeviceDetector.h"

#include <QFormLayout>

/** @brief 构造USB配置面板，初始化设备选择/VID/PID/接口/连接按钮等UI控件 @param parent 父控件 */
UsbConfigPanel::UsbConfigPanel(QWidget* parent)
    : QWidget(parent)
    , m_connected(false)
{
    setObjectName("UsbConfigPanel");

    auto* layout = new QFormLayout(this);

    // 设备选择 + 扫描按钮
    auto* devLayout = new QHBoxLayout();
    m_deviceCombo = new QComboBox(this);
    m_deviceCombo->setObjectName("usbDeviceCombo");
    m_scanBtn = new QPushButton(tr("扫描"), this);
    m_scanBtn->setObjectName("usbScanBtn");
    m_scanBtn->setFixedWidth(60);
    devLayout->addWidget(m_deviceCombo);
    devLayout->addWidget(m_scanBtn);
    layout->addRow(tr("USB设备:"), devLayout);

    // VID输入
    m_vidSpin = new QSpinBox(this);
    m_vidSpin->setObjectName("vidSpin");
    m_vidSpin->setRange(0x0000, 0xFFFF);
    m_vidSpin->setDisplayIntegerBase(16);
    m_vidSpin->setPrefix("0x");
    layout->addRow(tr("VID:"), m_vidSpin);

    // PID输入
    m_pidSpin = new QSpinBox(this);
    m_pidSpin->setObjectName("pidSpin");
    m_pidSpin->setRange(0x0000, 0xFFFF);
    m_pidSpin->setDisplayIntegerBase(16);
    m_pidSpin->setPrefix("0x");
    layout->addRow(tr("PID:"), m_pidSpin);

    // 接口编号
    m_interfaceSpin = new QSpinBox(this);
    m_interfaceSpin->setObjectName("interfaceSpin");
    m_interfaceSpin->setRange(0, 255);
    layout->addRow(tr("接口编号:"), m_interfaceSpin);

    // 连接按钮
    m_connectBtn = new QPushButton(tr("连接"), this);
    m_connectBtn->setObjectName("usbConnectBtn");
    layout->addRow(m_connectBtn);

    // 状态标签
    m_statusLabel = new QLabel(tr("未连接"), this);
    m_statusLabel->setObjectName("usbStatusLabel");
    layout->addRow(tr("状态:"), m_statusLabel);

    // 信号连接
    connect(m_scanBtn, &QPushButton::clicked,
            this, &UsbConfigPanel::onScanClicked);
    connect(m_deviceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &UsbConfigPanel::onDeviceChanged);
    connect(m_connectBtn, &QPushButton::clicked,
            this, &UsbConfigPanel::onConnectClicked);
    connect(m_vidSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this]() { ++m_totalConfigChanges; });
    connect(m_pidSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this]() { ++m_totalConfigChanges; });
    connect(m_interfaceSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this]() { ++m_totalConfigChanges; });
}

/** @brief 设置设备检测器实例 @param detector 检测器对象指针 */
void UsbConfigPanel::setDetector(UsbDeviceDetector* detector) {
    m_detector = detector;
    if (m_detector) {
        onScanClicked();
    }
}

/** @brief 扫描USB设备并填充下拉框 */
void UsbConfigPanel::onScanClicked() {
    ++m_totalDeviceRefreshes;
    m_deviceCombo->clear();

    QVariantList devices;
    if (m_detector) {
        devices = m_detector->scanDevices();
    }

    for (const QVariant& var : devices) {
        QVariantMap dev = var.toMap();
        QString label = tr("%1 (VID_%2 PID_%3)")
                            .arg(dev["name"].toString(),
                                 dev["vidHex"].toString(),
                                 dev["pidHex"].toString());
        m_deviceCombo->addItem(label, dev);
    }

    if (devices.isEmpty()) {
        m_statusLabel->setText(tr("未发现USB设备"));
    } else {
        m_statusLabel->setText(tr("发现 %1 个设备").arg(devices.size()));
    }
}

/** @brief 设备选择变更时更新VID/PID */
void UsbConfigPanel::onDeviceChanged(int index) {
    if (index < 0) { return; }
    ++m_totalDeviceSelections;
    QVariantMap dev = m_deviceCombo->itemData(index).toMap();
    if (!dev.isEmpty()) {
        m_vidSpin->setValue(dev["vid"].toUInt());
        m_pidSpin->setValue(dev["pid"].toUInt());
    }
}

/** @brief 连接/断开按钮点击处理 */
void UsbConfigPanel::onConnectClicked() {
    if (!m_connected) {
        ++m_totalConnectAttempts;
        quint16 vid = static_cast<quint16>(m_vidSpin->value());
        quint16 pid = static_cast<quint16>(m_pidSpin->value());
        int iface = m_interfaceSpin->value();
        emit connectRequested(vid, pid, iface);
        m_statusLabel->setText(tr("正在连接..."));
    } else {
        emit disconnectRequested();
        m_connected = false;
        m_connectBtn->setText(tr("连接"));
        m_statusLabel->setText(tr("已断开"));
    }
}

/** @brief 设置连接状态(由外部连接管理器调用) @param connected true=已连接 */
void UsbConfigPanel::setConnected(bool connected) {
    m_connected = connected;
    m_connectBtn->setText(connected ? tr("断开") : tr("连接"));
    m_statusLabel->setText(connected ? tr("已连接") : tr("未连接"));
}

/** @brief 保存USB配置到QSettings @param settings QSettings对象 */
void UsbConfigPanel::saveSettings(QSettings& settings) const
{
    settings.setValue(QStringLiteral("usb/vid"), m_vidSpin->value());
    settings.setValue(QStringLiteral("usb/pid"), m_pidSpin->value());
    settings.setValue(QStringLiteral("usb/interface"),
                      m_interfaceSpin->value());
}

/** @brief 从QSettings加载USB配置 @param settings QSettings对象 */
void UsbConfigPanel::loadSettings(QSettings& settings)
{
    m_vidSpin->setValue(
        settings.value(QStringLiteral("usb/vid"), 0).toInt());
    m_pidSpin->setValue(
        settings.value(QStringLiteral("usb/pid"), 0).toInt());
    m_interfaceSpin->setValue(
        settings.value(QStringLiteral("usb/interface"), 0).toInt());
}

/** @brief 重置所有统计计数器 */
void UsbConfigPanel::resetStatistics()
{
    m_totalDeviceRefreshes = 0;
    m_totalConfigChanges = 0;
    m_totalDeviceSelections = 0;
    m_totalConnectAttempts = 0;
}

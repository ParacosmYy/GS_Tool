/**
 * @file BleConfigPanel.cpp
 * @brief BLE配置面板实现
 *
 * 完整UI: 设备下拉框+扫描按钮、地址输入+连接按钮、状态标签。
 * 通过BleScanner集成设备发现流程。
 */

#include "connection/ble/BleConfigPanel.h"
#include "connection/ble/BleScanner.h"
#include <QFormLayout>
#include <QHBoxLayout>
#include <QRegularExpression>

BleConfigPanel::BleConfigPanel(QWidget* parent)
    : QWidget(parent)
    , m_deviceCombo(new QComboBox(this))
    , m_addressEdit(new QLineEdit(this))
    , m_scanBtn(new QPushButton(tr("扫描"), this))
    , m_connectBtn(new QPushButton(tr("连接"), this))
    , m_statusLabel(new QLabel(tr("就绪"), this))
{
    setObjectName("BleConfigPanel");

    // 控件命名（QSS依赖）
    m_deviceCombo->setObjectName("comboBleDevices");
    m_addressEdit->setObjectName("editBleAddress");
    m_scanBtn->setObjectName("btnBleScan");
    m_connectBtn->setObjectName("btnBleConnect");
    m_statusLabel->setObjectName("labelBleStatus");

    // 控件属性
    m_addressEdit->setPlaceholderText(
        tr("例如 00:11:22:33:44:55"));
    m_deviceCombo->setPlaceholderText(tr("点击扫描发现设备"));
    m_statusLabel->setAlignment(Qt::AlignCenter);

    // 地址输入校验: XX:XX:XX:XX:XX:XX 格式
    QRegularExpression addrRegex(
        "([0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2}");
    m_addressEdit->setToolTip(
        tr("蓝牙设备地址格式: XX:XX:XX:XX:XX:XX"));

    // 扫描行
    auto scanLayout = new QHBoxLayout();
    scanLayout->addWidget(m_deviceCombo, 1);
    scanLayout->addWidget(m_scanBtn);

    // 连接行
    auto connectLayout = new QHBoxLayout();
    connectLayout->addWidget(m_addressEdit, 1);
    connectLayout->addWidget(m_connectBtn);

    // 表单布局
    auto form = new QFormLayout(this);
    form->addRow(tr("已发现设备:"), scanLayout);
    form->addRow(tr("设备地址:"), connectLayout);
    form->addRow(tr("状态:"), m_statusLabel);

    // 信号连接
    connect(m_scanBtn, &QPushButton::clicked,
            this, &BleConfigPanel::scanRequested);
    connect(m_connectBtn, &QPushButton::clicked,
            this, &BleConfigPanel::connectRequested);
    connect(m_deviceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &BleConfigPanel::onDeviceSelected);
}

QVariantMap BleConfigPanel::config() const
{
    QVariantMap cfg;
    // 优先使用下拉框选中的设备地址
    const int idx = m_deviceCombo->currentIndex();
    if (idx >= 0 && idx < m_deviceList.size()) {
        const QVariantMap dev = m_deviceList.at(idx).toMap();
        cfg["address"] = dev.value("address").toString();
        cfg["deviceName"] = dev.value("name").toString();
    } else {
        cfg["address"] = m_addressEdit->text().trimmed().toUpper();
        cfg["deviceName"] = QString();
    }
    return cfg;
}

void BleConfigPanel::setScanner(BleScanner* scanner)
{
    m_scanner = scanner;
    if (m_scanner) {
        connect(m_scanner, &BleScanner::deviceFound,
                this, &BleConfigPanel::onDeviceFound);
        connect(m_scanner, &BleScanner::scanFinished,
                this, &BleConfigPanel::onScanFinished);
    }
}

void BleConfigPanel::onDeviceFound(const QVariantMap& device)
{
    const QString name = device.value("name").toString();
    const QString addr = device.value("address").toString();
    const int rssi = device.value("rssi").toInt();

    const QString display = QStringLiteral("%1 (%2) [%3 dBm]")
        .arg(name, addr, QString::number(rssi));

    m_deviceCombo->addItem(display);
    m_deviceList.append(device);
    m_statusLabel->setText(
        tr("发现设备: %1").arg(name));
}

void BleConfigPanel::onScanFinished()
{
    m_statusLabel->setText(
        tr("扫描完成，发现 %1 个设备").arg(m_deviceList.size()));
}

void BleConfigPanel::onDeviceSelected(int index)
{
    if (index >= 0 && index < m_deviceList.size()) {
        const QVariantMap dev = m_deviceList.at(index).toMap();
        m_addressEdit->setText(dev.value("address").toString());
    }
}

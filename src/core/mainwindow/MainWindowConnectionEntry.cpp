/**
 * @file MainWindowConnectionEntry.cpp
 * @brief 主窗口连接入口 - 快速新建连接对话框与入口路由
 *
 * 该文件只负责把主窗口级快捷键和导航入口，转成一次快速连接创建动作。
 * 连接默认参数由 ConnectionPresetBuilder 提供，实际连接创建继续由 ConnectionController 负责。
 */

#include "core/mainwindow/MainWindow.h"

#include "core/connect/ConnectionPresetBuilder.h"

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QStringList>

namespace {

struct QuickConnectionTypeItem {
    ConnectionType type;
    const char* label;
};

constexpr QuickConnectionTypeItem kQuickConnectionTypes[] = {
    {ConnectionType::TcpClient, "TCP客户端"},
    {ConnectionType::TcpServer, "TCP服务端"},
    {ConnectionType::Udp, "UDP"},
    {ConnectionType::WebSocket, "WebSocket"},
    {ConnectionType::Mqtt, "MQTT"},
    {ConnectionType::Tls, "TLS"},
    {ConnectionType::Ble, "BLE"},
    {ConnectionType::Can, "CAN"},
    {ConnectionType::Spi, "SPI"},
    {ConnectionType::I2c, "I2C"},
    {ConnectionType::Usb, "USB"},
};

QString buildPresetSummary(const QVariantMap& params)
{
    if (params.isEmpty()) {
        return QObject::tr("无默认参数");
    }

    QStringList items;
    for (auto it = params.constBegin(); it != params.constEnd(); ++it) {
        items.append(QStringLiteral("%1=%2").arg(it.key(), it.value().toString()));
    }
    return items.join(QStringLiteral(", "));
}

} // namespace

/** @brief 打开快速连接对话框，选择连接类型后使用默认参数创建连接 */
void MainWindow::openQuickConnectionDialog()
{
    QDialog dialog(this);
    dialog.setWindowTitle(tr("新建连接"));
    dialog.setObjectName("quickConnectionDialog");

    auto* layout = new QVBoxLayout(&dialog);
    auto* formLayout = new QFormLayout();
    formLayout->setLabelAlignment(Qt::AlignRight);
    formLayout->setFormAlignment(Qt::AlignTop);
    formLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    auto* typeCombo = new QComboBox(&dialog);
    typeCombo->setObjectName("quickConnectionTypeCombo");
    for (const auto& item : kQuickConnectionTypes) {
        typeCombo->addItem(tr(item.label), static_cast<int>(item.type));
    }
    typeCombo->setCurrentIndex(0);

    auto* presetLabel = new QLabel(&dialog);
    presetLabel->setObjectName("quickConnectionPresetLabel");
    presetLabel->setWordWrap(true);
    presetLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

    formLayout->addRow(tr("连接类型"), typeCombo);
    formLayout->addRow(tr("默认参数"), presetLabel);

    auto updatePreset = [typeCombo, presetLabel]() {
        const auto type = static_cast<ConnectionType>(typeCombo->currentData().toInt());
        presetLabel->setText(buildPresetSummary(ConnectionPresetBuilder::build(type)));
    };

    connect(typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            &dialog, [updatePreset](int) {
        updatePreset();
    });

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    if (auto* connectButton = buttons->button(QDialogButtonBox::Ok)) {
        connectButton->setText(tr("连接"));
    }
    if (auto* cancelButton = buttons->button(QDialogButtonBox::Cancel)) {
        cancelButton->setText(tr("取消"));
    }
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    layout->addLayout(formLayout);
    layout->addStretch(1);
    layout->addWidget(buttons);

    updatePreset();
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    const auto type = static_cast<ConnectionType>(typeCombo->currentData().toInt());
    m_connController->connectNetwork(type, ConnectionPresetBuilder::build(type));
}

/**
 * @file BluetoothAtConsole.cpp
 * @brief 蓝牙AT指令控制台实现
 */

#include "connection/ble/BluetoothAtConsole.h"
#include "connection/interface/IConnection.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

BluetoothAtConsole::BluetoothAtConsole(QWidget* parent)
    : QWidget(parent)
    , m_cmdInput(new QLineEdit(this))
    , m_output(new QTextEdit(this))
    , m_sendBtn(new QPushButton(tr("发送"), this))
{
    setObjectName("BluetoothAtConsole");

    m_cmdInput->setPlaceholderText(tr("输入AT指令，如 AT+VERSION"));
    m_output->setReadOnly(true);
    m_output->setPlaceholderText(tr("AT指令响应将显示在此处"));

    auto inputLayout = new QHBoxLayout();
    inputLayout->addWidget(m_cmdInput, 1);
    inputLayout->addWidget(m_sendBtn);

    auto mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(inputLayout);
    mainLayout->addWidget(m_output, 1);

    connect(m_sendBtn, &QPushButton::clicked,
            this, &BluetoothAtConsole::onSendClicked);
}

void BluetoothAtConsole::sendCommand(const QString& command)
{
    if (!m_connection || command.isEmpty()) {
        return;
    }
    m_output->append(QStringLiteral("> ") + command);
    const QByteArray data = (command + "\r\n").toUtf8();
    m_connection->write(data);
}

void BluetoothAtConsole::setConnection(IConnection* connection)
{
    m_connection = connection;
    if (m_connection) {
        connect(m_connection, &IConnection::dataReceived,
                this, [this](const QByteArray& data) {
                    m_output->append(QString::fromUtf8(data));
                });
    }
}

void BluetoothAtConsole::onSendClicked()
{
    const QString cmd = m_cmdInput->text().trimmed();
    if (!cmd.isEmpty()) {
        sendCommand(cmd);
        m_cmdInput->clear();
    }
}

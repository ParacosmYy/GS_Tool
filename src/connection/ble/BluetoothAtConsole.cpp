/**
 * @file BluetoothAtConsole.cpp
 * @brief 蓝牙AT指令控制台实现
 *
 * 完整UI: 输出区 + 输入/发送栏 + HC-05/HC-06预设AT指令面板。
 * 预设指令包括测试、版本、名称、PIN码、波特率、角色等。
 */

#include "connection/ble/BluetoothAtConsole.h"
#include "connection/interface/IConnection.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFont>
#include <QScrollArea>

/// 预设AT指令定义: { 显示文本, AT指令 }
static const QVector<QPair<QString, QString>> PRESET_COMMANDS = {
    { "AT",                 "AT" },
    { "AT+VERSION",         "AT+VERSION" },
    { "AT+NAME?",           "AT+NAME?" },
    { "AT+NAME=MyDevice",   "AT+NAME=MyDevice" },
    { "AT+PIN?",            "AT+PIN?" },
    { "AT+PIN=1234",        "AT+PIN=1234" },
    { "AT+BAUD?",           "AT+BAUD?" },
    { "AT+BAUD=4",          "AT+BAUD=4" },
    { "AT+ROLE?",           "AT+ROLE?" },
    { "AT+ROLE=0",          "AT+ROLE=0" },
    { "AT+ROLE=1",          "AT+ROLE=1" },
    { "AT+RESET",           "AT+RESET" },
};

BluetoothAtConsole::BluetoothAtConsole(QWidget* parent)
    : QWidget(parent)
    , m_cmdInput(new QLineEdit(this))
    , m_output(new QTextEdit(this))
    , m_sendBtn(new QPushButton(tr("发送"), this))
{
    setObjectName("BluetoothAtConsole");

    // 控件命名
    m_cmdInput->setObjectName("editAtCommand");
    m_output->setObjectName("textAtOutput");
    m_sendBtn->setObjectName("btnAtSend");

    // 输出区配置
    m_output->setReadOnly(true);
    m_output->setPlaceholderText(
        tr("AT指令响应将显示在此处"));
    QFont monoFont("Consolas", 10);
    monoFont.setStyleHint(QFont::Monospace);
    m_output->setFont(monoFont);

    // 输入区配置
    m_cmdInput->setPlaceholderText(
        tr("输入AT指令，如 AT+VERSION"));
    m_cmdInput->setToolTip(
        tr("支持HC-05/HC-06 AT指令集"));

    // ---- 输入栏 ----
    auto inputLayout = new QHBoxLayout();
    inputLayout->addWidget(m_cmdInput, 1);
    inputLayout->addWidget(m_sendBtn);

    // ---- 预设指令面板 ----
    auto* presetLabel = new QLabel(tr("常用AT指令:"), this);
    presetLabel->setObjectName("labelAtPresets");

    auto* presetGrid = new QGridLayout();
    presetGrid->setContentsMargins(0, 0, 0, 0);
    const int cols = 4;
    for (int i = 0; i < PRESET_COMMANDS.size(); ++i) {
        auto* btn = new QPushButton(
            PRESET_COMMANDS.at(i).first, this);
        btn->setObjectName(
            QStringLiteral("btnPreset%1").arg(i));
        btn->setToolTip(PRESET_COMMANDS.at(i).second);
        btn->setMaximumHeight(32);

        // 按钮点击发送对应AT指令
        const QString cmd = PRESET_COMMANDS.at(i).second;
        connect(btn, &QPushButton::clicked, this,
                [this, cmd]() {
                    m_cmdInput->setText(cmd);
                    sendCommand(cmd);
                    m_cmdInput->clear();
                });

        presetGrid->addWidget(btn, i / cols, i % cols);
        m_presetButtons.append(btn);
    }

    // ---- 主布局 ----
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_output, 1);
    mainLayout->addLayout(inputLayout);
    mainLayout->addWidget(presetLabel);
    mainLayout->addLayout(presetGrid);

    // 信号连接
    connect(m_sendBtn, &QPushButton::clicked,
            this, &BluetoothAtConsole::onSendClicked);
    connect(m_cmdInput, &QLineEdit::returnPressed,
            this, &BluetoothAtConsole::onSendClicked);
}

void BluetoothAtConsole::sendCommand(const QString& command)
{
    if (!m_connection) {
        m_output->append(tr("[错误] 未绑定连接"));
        return;
    }
    if (command.isEmpty()) {
        return;
    }

    m_output->append(QStringLiteral(">>> ") + command);
    const QByteArray data = (command + "\r\n").toUtf8();
    const qint64 written = m_connection->write(data);
    if (written < 0) {
        m_output->append(tr("[发送失败] 连接不可用"));
    }
}

void BluetoothAtConsole::setConnection(IConnection* connection)
{
    // 断开旧连接的信号
    if (m_connection) {
        disconnect(m_connection, &IConnection::dataReceived,
                   this, nullptr);
    }

    m_connection = connection;
    if (m_connection) {
        connect(m_connection, &IConnection::dataReceived,
                this, [this](const QByteArray& data) {
                    const QString text = QString::fromUtf8(data);
                    m_output->append(text);
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

/**
 * @brief 预设指令点击处理(由各按钮lambda直接调用sendCommand)
 */
void BluetoothAtConsole::onPresetClicked()
{
    // 预设指令通过构造函数中的lambda直接调用sendCommand
    // 此slot保留用于信号路由兼容
}

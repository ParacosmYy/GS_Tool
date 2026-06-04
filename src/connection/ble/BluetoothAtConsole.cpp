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

/** @brief 构造AT控制台，初始化输出区/输入栏/预设指令面板UI @param parent 父控件 */
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

/** @brief 通过连接发送AT指令并显示在输出区 @param command AT指令字符串(如AT+VERSION) */
void BluetoothAtConsole::sendCommand(const QString& command)
{
    if (!m_connection) {
        m_output->append(tr("[错误] 未绑定连接"));
        ++m_totalSendErrors;
        return;
    }
    if (command.isEmpty()) {
        return;
    }

    m_output->append(tr(">>> %1").arg(command));
    const QByteArray data = (command + "\r\n").toUtf8();
    const qint64 written = m_connection->write(data);
    if (written < 0) {
        m_output->append(tr("[发送失败] 连接不可用"));
        ++m_totalSendErrors;
    } else {
        m_totalBytesSent += static_cast<quint64>(written);
    }
    ++m_totalCommandsSent;
}

/** @brief 绑定IConnection连接实例，连接dataReceived信号用于显示响应 @param connection IConnection连接对象 */
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
                    ++m_totalResponsesReceived;
                });
    }
}

/** @brief 发送按钮点击处理，读取输入框文本并发送AT指令 */
void BluetoothAtConsole::onSendClicked()
{
    const QString cmd = m_cmdInput->text().trimmed();
    if (!cmd.isEmpty()) {
        sendCommand(cmd);
        m_cmdInput->clear();
    }
}

/** @brief 预设指令按钮点击统一处理槽，从sender()提取AT指令并发送 */
void BluetoothAtConsole::onPresetClicked()
{
    auto* btn = qobject_cast<QPushButton*>(sender());
    if (!btn) {
        return;
    }

    /* 从按钮的toolTip中获取完整的AT指令文本 */
    const QString cmd = btn->toolTip().trimmed();
    if (!cmd.isEmpty()) {
        m_cmdInput->setText(cmd);
        sendCommand(cmd);
        m_cmdInput->clear();
    }
}

// 统计getter/resetStatistics已移至 BluetoothAtConsoleStats.cpp

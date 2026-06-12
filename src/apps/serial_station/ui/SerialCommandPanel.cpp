#include "apps/serial_station/ui/SerialCommandPanel.h"

#include <QtWidgets/QComboBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>

namespace serial_station {

SerialCommandPanel::SerialCommandPanel(QWidget* parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("serialCommandPanel"));
    setupUi();
    connectSignals();
    updateSendButtonState();
}

QString SerialCommandPanel::commandText() const
{
    return m_commandEdit->text().trimmed();
}

QString SerialCommandPanel::sendMode() const
{
    return m_modeCombo->currentData().toString();
}

void SerialCommandPanel::setSendEnabled(bool enabled)
{
    m_commandEdit->setEnabled(enabled);
    m_modeCombo->setEnabled(enabled);
    m_sendButton->setEnabled(enabled && !commandText().isEmpty());
    m_readIdButton->setEnabled(enabled);
    m_pingButton->setEnabled(enabled);
    m_resetButton->setEnabled(enabled);
}

void SerialCommandPanel::emitSendRequested()
{
    const QString command = commandText();
    if (command.isEmpty()) {
        updateSendButtonState();
        return;
    }
    emit sendRequested(command, sendMode());
}

void SerialCommandPanel::applyQuickCommand()
{
    auto* button = qobject_cast<QToolButton*>(sender());
    if (!button) {
        return;
    }

    const QString command = button->property("command").toString();
    if (command.isEmpty()) {
        return;
    }

    m_commandEdit->setText(command);
    emit quickCommandSelected(command);
    updateSendButtonState();
}

void SerialCommandPanel::updateSendButtonState()
{
    m_sendButton->setEnabled(m_commandEdit->isEnabled() && !commandText().isEmpty());
}

void SerialCommandPanel::setupUi()
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(12, 12, 12, 12);
    root->setSpacing(10);

    auto* headerRow = new QHBoxLayout();
    headerRow->setContentsMargins(0, 0, 0, 0);
    headerRow->setSpacing(8);

    auto* title = new QLabel(tr("命令发送"), this);
    title->setObjectName(QStringLiteral("serialCommandTitle"));

    auto* hint = new QLabel(tr("输入 ASCII 或协议命令，由控制器交给协议层构建发送帧"), this);
    hint->setObjectName(QStringLiteral("serialCommandHint"));
    hint->setWordWrap(true);

    headerRow->addWidget(title);
    headerRow->addStretch();
    headerRow->addWidget(hint, 1);

    auto* inputRow = new QHBoxLayout();
    inputRow->setContentsMargins(0, 0, 0, 0);
    inputRow->setSpacing(8);

    m_commandEdit = new QLineEdit(this);
    m_commandEdit->setObjectName(QStringLiteral("serialCommandEdit"));
    m_commandEdit->setPlaceholderText(tr("例如: AT+GMR 或 01 03 00 00 00 02"));
    m_commandEdit->setClearButtonEnabled(true);

    m_modeCombo = new QComboBox(this);
    m_modeCombo->setObjectName(QStringLiteral("serialCommandModeCombo"));
    m_modeCombo->addItem(tr("ASCII"), QStringLiteral("ascii"));
    m_modeCombo->addItem(tr("协议命令"), QStringLiteral("protocol"));

    m_sendButton = new QPushButton(tr("发送"), this);
    m_sendButton->setObjectName(QStringLiteral("serialCommandSendButton"));

    inputRow->addWidget(m_commandEdit, 1);
    inputRow->addWidget(m_modeCombo);
    inputRow->addWidget(m_sendButton);

    auto* quickRow = new QHBoxLayout();
    quickRow->setContentsMargins(0, 0, 0, 0);
    quickRow->setSpacing(8);

    auto* quickLabel = new QLabel(tr("快捷"), this);
    quickLabel->setObjectName(QStringLiteral("serialQuickLabel"));

    m_readIdButton = createQuickButton(tr("读版本"), QStringLiteral("AT+GMR"));
    m_pingButton = createQuickButton(tr("Ping"), QStringLiteral("PING"));
    m_resetButton = createQuickButton(tr("复位"), QStringLiteral("RESET"));

    quickRow->addWidget(quickLabel);
    quickRow->addWidget(m_readIdButton);
    quickRow->addWidget(m_pingButton);
    quickRow->addWidget(m_resetButton);
    quickRow->addStretch();

    auto* divider = new QFrame(this);
    divider->setObjectName(QStringLiteral("serialCommandDivider"));
    divider->setFrameShape(QFrame::HLine);

    root->addLayout(headerRow);
    root->addLayout(inputRow);
    root->addWidget(divider);
    root->addLayout(quickRow);
}

void SerialCommandPanel::connectSignals()
{
    connect(m_sendButton, &QPushButton::clicked,
            this, &SerialCommandPanel::emitSendRequested);
    connect(m_commandEdit, &QLineEdit::returnPressed,
            this, &SerialCommandPanel::emitSendRequested);
    connect(m_commandEdit, &QLineEdit::textChanged,
            this, &SerialCommandPanel::updateSendButtonState);
    connect(m_readIdButton, &QToolButton::clicked,
            this, &SerialCommandPanel::applyQuickCommand);
    connect(m_pingButton, &QToolButton::clicked,
            this, &SerialCommandPanel::applyQuickCommand);
    connect(m_resetButton, &QToolButton::clicked,
            this, &SerialCommandPanel::applyQuickCommand);
}

QToolButton* SerialCommandPanel::createQuickButton(const QString& text, const QString& command)
{
    auto* button = new QToolButton(this);
    button->setObjectName(QStringLiteral("serialQuickCommandButton"));
    button->setText(text);
    button->setProperty("command", command);
    button->setToolButtonStyle(Qt::ToolButtonTextOnly);
    return button;
}

} // namespace serial_station

#include "apps/serial_station/ui/SerialCommandPanel.h"

#include <QtCore/QSignalBlocker>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>

#include "apps/serial_station/SerialStationModels.h"

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
    m_historyCombo->setEnabled(enabled && m_history.count() > 0);
    m_sendButton->setEnabled(enabled && !commandText().isEmpty());
    m_clearHistoryButton->setEnabled(m_history.count() > 0);
    m_readIdButton->setEnabled(enabled);
    m_pingButton->setEnabled(enabled);
    m_resetButton->setEnabled(enabled);
}

int SerialCommandPanel::historyCount() const
{
    return m_history.count();
}

QStringList SerialCommandPanel::historyCommands() const
{
    return m_history.commands();
}

void SerialCommandPanel::applyProfileCommands(const QVector<SerialProfileCommand>& commands,
                                              const QString& defaultMode)
{
    m_history.clear();
    setModeById(defaultMode);
    m_commandEdit->clear();

    for (auto it = commands.crbegin(); it != commands.crend(); ++it) {
        m_history.recordCommand(it->payload, it->mode);
    }

    refreshHistoryUi();
    if (!commands.isEmpty()) {
        const SerialProfileCommand& firstCommand = commands.first();
        m_commandEdit->setText(firstCommand.payload);
        setModeById(firstCommand.mode);
    }
    updateSendButtonState();
}

void SerialCommandPanel::confirmLastSentCommand()
{
    if (m_pendingCommand.trimmed().isEmpty()) {
        return;
    }

    recordSentCommand(m_pendingCommand, m_pendingMode);
    m_commandEdit->setText(QString());
    m_pendingCommand.clear();
    m_pendingMode.clear();
}

void SerialCommandPanel::recordSentCommand(const QString& command, const QString& mode)
{
    if (m_history.recordCommand(command, mode)) {
        refreshHistoryUi();
    }
    refreshStatus(tr("发送成功"), QStringLiteral("ok"));
}

void SerialCommandPanel::notifyCommandFailed(const QString& message)
{
    notifyCommandFailed(QStringLiteral("unknown"), message);
}

void SerialCommandPanel::notifyCommandFailed(const QString& reason, const QString& message)
{
    const QString safeReason = failureReasonLabel(reason);
    const QString safeMessage = message.trimmed();
    const QString text = safeMessage.isEmpty()
                            ? safeReason
                            : tr("%1：%2").arg(safeReason, safeMessage);
    refreshStatus(text, QStringLiteral("error"));
}

QString SerialCommandPanel::failureReasonLabel(const QString& reason) const
{
    const QString normalizedReason = reason.trimmed().toLower();
    if (normalizedReason == QStringLiteral("encode")) {
        return tr("发送失败（编码）");
    }
    if (normalizedReason == QStringLiteral("not_connected")) {
        return tr("发送失败（未连接）");
    }
    if (normalizedReason == QStringLiteral("write_failed")) {
        return tr("发送失败（写入）");
    }
    return tr("发送失败");
}

void SerialCommandPanel::emitSendRequested()
{
    const QString command = commandText();
    if (command.isEmpty()) {
        updateSendButtonState();
        return;
    }

    if (!m_sendButton->isEnabled() || !m_commandEdit->isEnabled()) {
        refreshStatus(tr("发送未启用，请先连接串口"), QStringLiteral("error"));
        return;
    }

    const QDateTime now = QDateTime::currentDateTimeUtc();
    if (m_lastSendTime.isValid()) {
        const qint64 deltaMs = m_lastSendTime.msecsTo(now);
        if (deltaMs >= 0 && deltaMs < kSendIntervalMs) {
            refreshStatus(tr("发送频率过快，请稍后重试"), QStringLiteral("warning"));
            return;
        }
    }
    m_lastSendTime = now;

    m_pendingCommand = command;
    m_pendingMode = sendMode();
    emit sendRequested(command, sendMode());
    refreshStatus(tr("发送中"), QStringLiteral("ok"));
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

void SerialCommandPanel::applyHistoryCommand(int index)
{
    const int historyIndex = m_historyCombo->itemData(index).toInt();
    const QString command = m_history.commandAt(historyIndex);
    if (command.isEmpty()) {
        return;
    }

    m_commandEdit->setText(command);
    setModeById(m_history.modeAt(historyIndex));
    updateSendButtonState();
}

void SerialCommandPanel::clearHistory()
{
    m_history.clear();
    refreshHistoryUi();
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
    m_statusLabel = new QLabel(tr("就绪"), this);
    m_statusLabel->setObjectName(QStringLiteral("serialCommandStatus"));
    headerRow->addWidget(m_statusLabel);
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
    m_modeCombo->addItem(tr("HEX"), QStringLiteral("hex"));
    m_modeCombo->addItem(tr("协议命令"), QStringLiteral("protocol"));

    m_sendButton = new QPushButton(tr("发送"), this);
    m_sendButton->setObjectName(QStringLiteral("serialCommandSendButton"));

    inputRow->addWidget(m_commandEdit, 1);
    inputRow->addWidget(m_modeCombo);
    inputRow->addWidget(m_sendButton);

    auto* historyRow = new QHBoxLayout();
    historyRow->setContentsMargins(0, 0, 0, 0);
    historyRow->setSpacing(8);

    auto* historyLabel = new QLabel(tr("最近"), this);
    historyLabel->setObjectName(QStringLiteral("serialHistoryLabel"));

    m_historyCombo = new QComboBox(this);
    m_historyCombo->setObjectName(QStringLiteral("serialCommandHistoryCombo"));
    m_historyCombo->setPlaceholderText(tr("暂无最近命令"));
    m_historyCombo->setEnabled(false);

    m_clearHistoryButton = new QPushButton(tr("清空最近"), this);
    m_clearHistoryButton->setObjectName(QStringLiteral("serialHistoryClearButton"));
    m_clearHistoryButton->setEnabled(false);

    historyRow->addWidget(historyLabel);
    historyRow->addWidget(m_historyCombo, 1);
    historyRow->addWidget(m_clearHistoryButton);

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
    root->addLayout(historyRow);
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
    connect(m_historyCombo, QOverload<int>::of(&QComboBox::activated),
            this, &SerialCommandPanel::applyHistoryCommand);
    connect(m_clearHistoryButton, &QPushButton::clicked,
            this, &SerialCommandPanel::clearHistory);
    connect(m_readIdButton, &QToolButton::clicked,
            this, &SerialCommandPanel::applyQuickCommand);
    connect(m_pingButton, &QToolButton::clicked,
            this, &SerialCommandPanel::applyQuickCommand);
    connect(m_resetButton, &QToolButton::clicked,
            this, &SerialCommandPanel::applyQuickCommand);
}

void SerialCommandPanel::refreshStatus(const QString& status, const QString& state)
{
    if (!m_statusLabel) {
        return;
    }

    const QString safeStatus = status.trimmed();
    m_statusLabel->setText(safeStatus.isEmpty() ? tr("就绪") : safeStatus);
    const QString safeState = state.trimmed().toLower();
    m_statusLabel->setProperty(QStringLiteral("state"), safeState);
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

void SerialCommandPanel::refreshHistoryUi()
{
    const QSignalBlocker blocker(m_historyCombo);
    m_historyCombo->clear();
    for (int i = 0; i < m_history.count(); ++i) {
        m_historyCombo->addItem(m_history.displayTextAt(i), i);
    }

    const bool hasHistory = m_history.count() > 0;
    m_historyCombo->setEnabled(m_commandEdit->isEnabled() && hasHistory);
    m_clearHistoryButton->setEnabled(hasHistory);
}

void SerialCommandPanel::setModeById(const QString& mode)
{
    const int index = m_modeCombo->findData(mode);
    if (index >= 0) {
        m_modeCombo->setCurrentIndex(index);
    }
}

} // namespace serial_station

#include "core/SendController.h"
#include "terminal/TerminalModel.h"
#include "utils/DataLogger.h"
#include "serial/SendHistory.h"
#include "serial/TimedSender.h"
#include "connection/IConnection.h"
#include "utils/HexConverter.h"
#include "core/Constants.h"

#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QHBoxLayout>
#include <QFrame>
#include <QStyle>

SendController::SendController(TerminalModel* model, DataLogger* logger,
                               SendHistory* history, QObject* parent)
    : QObject(parent)
    , m_terminalModel(model)
    , m_dataLogger(logger)
    , m_sendHistory(history)
    , m_timedSender(new TimedSender(this))
{
}

SendController::~SendController()
{
}

QWidget* SendController::createSendBar(QWidget* parent)
{
    auto* sendFrame = new QFrame(parent);
    sendFrame->setObjectName("sendBarFrame");
    auto* sendLayout = new QHBoxLayout(sendFrame);
    sendLayout->setContentsMargins(8, 4, 8, 4);

    // 发送模式切换: 文本 / HEX
    m_sendModeCombo = new QComboBox;
    m_sendModeCombo->setObjectName("sendModeCombo");
    m_sendModeCombo->addItems({tr("文本"), tr("HEX")});
    m_sendModeCombo->setFixedWidth(60);

    // 发送输入框
    m_sendInput = new QLineEdit;
    m_sendInput->setObjectName("sendInput");
    m_sendInput->setPlaceholderText(tr("输入要发送的数据..."));

    // 发送历史自动补全（复用同一个QStringListModel，避免每次new泄漏）
    m_sendCompleterModel = new QStringListModel(m_sendHistory->recentTexts(), this);
    m_sendCompleter = new QCompleter(m_sendCompleterModel, this);
    m_sendCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    m_sendCompleter->setCompletionMode(QCompleter::PopupCompletion);
    m_sendInput->setCompleter(m_sendCompleter);

    // 发送按钮
    m_sendBtn = new QPushButton(tr("发送"));
    m_sendBtn->setObjectName("sendButton");
    m_sendBtn->setFixedWidth(70);

    // 自动追加换行符选择
    m_newlineCombo = new QComboBox;
    m_newlineCombo->setObjectName("newlineCombo");
    m_newlineCombo->addItems({tr("无"), "\\r\\n", "\\n", "\\r"});
    m_newlineCombo->setFixedWidth(70);
    m_newlineCombo->setToolTip(tr("自动追加换行符"));

    sendLayout->addWidget(m_sendModeCombo);
    sendLayout->addWidget(m_newlineCombo);
    sendLayout->addWidget(m_sendInput, 1);
    sendLayout->addWidget(m_sendBtn);

    // ---- 信号连接 ----

    // 发送按钮 / 回车触发发送
    connect(m_sendBtn, &QPushButton::clicked, this, &SendController::onSendData);
    connect(m_sendInput, &QLineEdit::returnPressed, this, &SendController::onSendData);

    // 发送历史变化时更新自动补全（复用模型，不泄漏QStringListModel）
    connect(m_sendHistory, &SendHistory::historyChanged, this, [this]() {
        m_sendCompleterModel->setStringList(m_sendHistory->recentTexts());
    });

    // 定时发送器的数据通过 sendAndRecord 发出
    connect(m_timedSender, &TimedSender::sendData, this, [this](const QByteArray& data) {
        sendAndRecord(data);
    });

    return sendFrame;
}

void SendController::setConnection(IConnection* conn)
{
    m_currentConn = conn;
}

TimedSender* SendController::timedSender() const
{
    return m_timedSender;
}

bool SendController::sendAndRecord(const QByteArray& data)
{
    if (!m_currentConn || m_currentConn->state() != ConnectionState::Connected) {
        emit statusMessage(tr("发送失败: 未连接"));
        return false;
    }
    qint64 written = m_currentConn->write(data);
    if (written > 0) {
        m_terminalModel->appendSent(data);
        m_dataLogger->logData(data, DataLogger::Direction::Sent);
        emit dataSent(written);
        return true;
    }
    // 写入失败时发出状态消息（修复: 原MainWindow缺少此反馈）
    emit statusMessage(tr("发送失败: 写入返回 %1").arg(written));
    return false;
}

void SendController::onSendData()
{
    if (!m_currentConn || m_currentConn->state() != ConnectionState::Connected) {
        return;
    }

    QString text = m_sendInput->text();
    if (text.isEmpty()) return;

    bool isHex = (m_sendModeCombo->currentIndex() == 1);
    QByteArray data;
    if (isHex) {
        data = HexConverter::fromHexString(text);
        if (data.isEmpty()) {
            m_sendInput->setProperty("hasError", true);
            m_sendInput->style()->unpolish(m_sendInput);
            m_sendInput->style()->polish(m_sendInput);
            return;
        }
    } else {
        data = text.toUtf8();
    }

    // 追加换行符（仅文本模式下生效）
    if (!isHex && m_newlineCombo && m_newlineCombo->currentIndex() > 0) {
        switch (m_newlineCombo->currentIndex()) {
        case 1: data.append("\r\n"); break;
        case 2: data.append("\n"); break;
        case 3: data.append("\r"); break;
        }
    }

    if (sendAndRecord(data)) {
        m_sendHistory->addEntry(text, isHex);
        m_sendInput->clear();
        m_sendInput->setProperty("hasError", false);
        m_sendInput->style()->unpolish(m_sendInput);
        m_sendInput->style()->polish(m_sendInput);
    }
}

void SendController::onQuickCommand(const QByteArray& data)
{
    sendAndRecord(data);
}

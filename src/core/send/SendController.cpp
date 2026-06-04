/**
 * @file SendController.cpp
 * @brief 发送控制器实现 - 管理发送栏 UI 创建、输入解析、数据发送和历史记录
 */

#include "core/send/SendController.h"
#include "core/send/SendHistoryManager.h"
#include "terminal/model/TerminalModel.h"
#include "utils/log/DataLogger.h"
#include "serial/commands/SendHistory.h"
#include "serial/commands/TimedSender.h"
#include "connection/interface/IConnection.h"
#include "utils/crypto/HexConverter.h"
#include "shared/AppConstants.h"
#include "shared/LayoutConstants.h"
#include "core/widgets/AnimatedButton.h"

#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QHBoxLayout>
#include <QFrame>
#include <QStyle>

/**
 * @brief 构造发送控制器
 * @param model 终端数据模型，发送的数据追加到此模型
 * @param logger 数据日志记录器
 * @param history 发送历史存储
 * @param parent 父对象
 */
SendController::SendController(TerminalModel* model, DataLogger* logger,
                                SendHistory* history, QObject* parent)
    : QObject(parent)
    , m_terminalModel(model)
    , m_dataLogger(logger)
    , m_historyManager(new SendHistoryManager(history, this))
    , m_timedSender(new TimedSender(this))
{
}

/**
 * @brief 创建发送输入区域并返回容器 widget
 *
 * 控件布局: [模式切换(文本/HEX)] [换行符选择] [输入框(带自动补全)] [发送按钮]
 * 自动补全委托给 SendHistoryManager 管理
 * @param parent 父 widget
 * @return 发送栏容器 widget
 */
QWidget* SendController::createSendBar(QWidget* parent)
{
    auto* sendFrame = new QFrame(parent);
    sendFrame->setObjectName("sendBarFrame");
    auto* sendLayout = new QHBoxLayout(sendFrame);
    sendLayout->setContentsMargins(Layout::kToolbarPadding, Layout::kToolbarSpacing,
                                   Layout::kToolbarPadding, Layout::kToolbarSpacing);

    // 发送模式切换: 文本 / HEX
    m_sendModeCombo = new QComboBox;
    m_sendModeCombo->setObjectName("sendModeCombo");
    m_sendModeCombo->addItems({tr("文本"), tr("HEX")});
    m_sendModeCombo->setFixedWidth(Layout::kMinButtonWidth);

    // 发送输入框
    m_sendInput = new QLineEdit;
    m_sendInput->setObjectName("sendInput");
    m_sendInput->setPlaceholderText(tr("输入要发送的数据..."));

    // 委托 SendHistoryManager 绑定自动补全（QCompleter + SmartAutoComplete + 信号链路）
    m_historyManager->setupAutoComplete(m_sendInput, parent);

    // 发送按钮
    m_sendBtn = new AnimatedButton(tr("发送"));
    m_sendBtn->setObjectName("sendButton");
    m_sendBtn->setFixedWidth(Layout::kSendBtnWidth);

    // 自动追加换行符选择
    m_newlineCombo = new QComboBox;
    m_newlineCombo->setObjectName("newlineCombo");
    m_newlineCombo->addItems({tr("无"), "\\r\\n", "\\n", "\\r"});
    m_newlineCombo->setFixedWidth(Layout::kNewlineComboWidth);
    m_newlineCombo->setToolTip(tr("自动追加换行符"));

    sendLayout->addWidget(m_sendModeCombo);
    sendLayout->addWidget(m_newlineCombo);
    sendLayout->addWidget(m_sendInput, 1);
    sendLayout->addWidget(m_sendBtn);

    // ---- 信号连接 ----

    // 发送按钮 / 回车触发发送
    connect(m_sendBtn, &QPushButton::clicked, this, &SendController::onSendData);
    connect(m_sendInput, &QLineEdit::returnPressed, this, &SendController::onSendData);

    // 定时发送器的数据通过 sendAndRecord 发出
    connect(m_timedSender, &TimedSender::sendData, this, [this](const QByteArray& data) {
        sendAndRecord(data);
    });

    return sendFrame;
}

/**
 * @brief 设置当前连接
 * @param conn 新的连接实例，断开时传 nullptr
 */
void SendController::setConnection(IConnection* conn)
{
    m_currentConn = conn;
    // 断开连接时停止定时发送，防止 use-after-free
    if (!conn && m_timedSender) {
        m_timedSender->stop();
    }
}

/** @brief 获取定时发送器实例 */
TimedSender* SendController::timedSender() const
{
    return m_timedSender;
}

// sendAndRecord/onSendData/onQuickCommand见 SendControllerSend.cpp

// 统计计数器接口实现已拆分至 SendControllerStats.cpp

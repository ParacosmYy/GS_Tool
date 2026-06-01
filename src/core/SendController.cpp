/**
 * @file SendController.cpp
 * @brief 发送控制器实现 - 管理发送栏 UI 创建、输入解析、数据发送和历史记录
 */

#include "core/SendController.h"
#include "terminal/TerminalModel.h"
#include "utils/DataLogger.h"
#include "serial/SendHistory.h"
#include "serial/TimedSender.h"
#include "connection/IConnection.h"
#include "utils/HexConverter.h"
#include "core/Constants.h"
#include "core/AnimatedButton.h"

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
 * @param history 发送历史管理器
 * @param parent 父对象
 */
SendController::SendController(TerminalModel* model, DataLogger* logger,
                               SendHistory* history, QObject* parent)
    : QObject(parent)
    , m_terminalModel(model)
    , m_dataLogger(logger)
    , m_sendHistory(history)
    , m_timedSender(new TimedSender(this))
{
}

/**
 * @brief 创建发送输入区域并返回容器 widget
 *
 * 控件布局: [模式切换(文本/HEX)] [换行符选择] [输入框(带自动补全)] [发送按钮]
 * 自动补全数据源为 SendHistory 的最近发送记录
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

    // 发送历史自动补全（复用同一个 QStringListModel，避免每次 new 造成内存泄漏）
    m_sendCompleterModel = new QStringListModel(m_sendHistory->recentTexts(), this);
    m_sendCompleter = new QCompleter(m_sendCompleterModel, this);
    m_sendCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    m_sendCompleter->setCompletionMode(QCompleter::PopupCompletion);
    m_sendInput->setCompleter(m_sendCompleter);

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

    // 发送历史变化时更新自动补全数据源
    connect(m_sendHistory, &SendHistory::historyChanged, this, [this]() {
        m_sendCompleterModel->setStringList(m_sendHistory->recentTexts());
    });

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

/**
 * @brief 统一发送方法 — 含部分写入重试策略
 *
 * 完整流程:
 *   1. 检查连接是否存在且处于已连接状态
 *   2. 循环写入数据到连接，最多重试 3 次部分写入
 *   3. 成功时追加到终端模型和日志
 *   4. 失败时通过 statusMessage 通知用户具体原因
 *
 * 部分写入重试策略:
 *   串口/网络 write() 不保证一次写入全部数据。当返回值小于请求字节数时，
 *   对剩余字节发起最多 kMaxPartialRetries(3) 次重试，避免未发送字节被静默丢弃。
 *   每次重试仅发送尚未写入的剩余部分 (data.mid(totalWritten))。
 *   若重试耗尽仍有剩余字节，记录警告但继续记录已发送部分。
 *
 * @param data 待发送的原始字节数据
 * @return true=至少写入了部分数据, false=写入完全失败或未连接
 */
bool SendController::sendAndRecord(const QByteArray& data)
{
    // 前置检查1: 连接是否存在
    if (!m_currentConn) {
        emit statusMessage(tr("发送失败: 未建立连接，请先连接串口"));
        return false;
    }

    // 前置检查2: 连接是否处于已连接状态
    ConnectionState state = m_currentConn->state();
    if (state != ConnectionState::Connected) {
        QString stateText;
        switch (state) {
        case ConnectionState::Disconnected:
            stateText = tr("已断开");
            break;
        case ConnectionState::Connecting:
            stateText = tr("连接中");
            break;
        case ConnectionState::Error:
            stateText = tr("错误");
            break;
        default:
            stateText = tr("未知");
            break;
        }
        emit statusMessage(tr("发送失败: 连接状态为 %1，无法发送数据").arg(stateText));
        return false;
    }

    // ---- 部分写入重试循环 ----
    // 最多重试 kMaxPartialRetries 次，确保剩余字节不被静默丢弃
    qint64 totalWritten = 0;
    int retryCount = 0;
    constexpr int kMaxPartialRetries = 3;

    while (totalWritten < data.size() && retryCount < kMaxPartialRetries) {
        int remaining = static_cast<int>(data.size() - totalWritten);
        qint64 written = m_currentConn->write(data.mid(static_cast<int>(totalWritten), remaining));
        if (written <= 0) {
            // 写入返回 0 或负值: 连接可能已断开，立即终止
            emit statusMessage(tr("发送失败: 写入返回 %1，已发送 %2/%3 字节")
                                   .arg(written).arg(totalWritten).arg(data.size()));
            return false;
        }
        totalWritten += written;
        if (totalWritten < data.size()) {
            ++retryCount;
        }
    }

    // 重试耗尽后仍有未发送字节: 记录警告，但不丢弃已发送部分
    if (totalWritten < data.size()) {
        emit statusMessage(tr("部分写入: 请求 %1 字节，实际发送 %2 字节（重试 %3 次）")
                               .arg(data.size()).arg(totalWritten).arg(kMaxPartialRetries));
    }

    // 记录实际发送的字节到终端模型和日志
    QByteArray sentData = data.left(static_cast<int>(totalWritten));
    m_terminalModel->appendSent(sentData);
    m_dataLogger->logData(sentData, DataLogger::Direction::Sent);
    emit dataSent(totalWritten);
    return true;
}

/**
 * @brief 发送按钮/回车触发的发送逻辑
 *
 * 流程:
 *   1. 前置检查（连接状态、输入非空）
 *   2. 根据发送模式解析输入:
 *      - 文本模式: toUtf8() 编码
 *      - HEX 模式: HexConverter::fromHexString() 解析
 *   3. HEX 解析失败时设置输入框错误样式（红色边框）并通过 statusMessage 提示
 *   4. 文本模式下追加换行符（\r\n/\n/\r）
 *   5. 调用 sendAndRecord() 写入数据
 *   6. 成功后记录历史、清空输入框、清除错误状态
 *
 * HEX 模式支持的格式示例: "AA 55 01 00 FE", "AA,55,01,00,FE", "AA550100FE", "0xAA 0x55"
 */
void SendController::onSendData()
{
    // 前置检查: 连接状态
    if (!m_currentConn || m_currentConn->state() != ConnectionState::Connected) {
        emit statusMessage(tr("发送失败: 未连接"));
        return;
    }

    QString text = m_sendInput->text();
    if (text.isEmpty()) return;

    // 根据发送模式解析输入
    bool isHex = (m_sendModeCombo->currentIndex() == 1);
    QByteArray data;
    if (isHex) {
        data = HexConverter::fromHexString(text);
        if (data.isEmpty()) {
            // HEX 解析失败: 设置错误属性触发 QSS 错误样式（红色边框）
            m_sendInput->setProperty("hasError", true);
            m_sendInput->style()->unpolish(m_sendInput);
            m_sendInput->style()->polish(m_sendInput);
            emit statusMessage(tr("HEX 格式错误: 请输入有效的十六进制数据，如 \"AA 55 01 00 FE\""));
            return;
        }
    } else {
        data = text.toUtf8();
    }

    // 追加换行符（仅文本模式下生效，HEX 模式用户需要手动输入）
    if (!isHex && m_newlineCombo && m_newlineCombo->currentIndex() > 0) {
        switch (m_newlineCombo->currentIndex()) {
        case 1: data.append("\r\n"); break;
        case 2: data.append("\n"); break;
        case 3: data.append("\r"); break;
        }
    }

    if (sendAndRecord(data)) {
        // 发送成功: 记录到历史 → 清空输入框 → 清除错误状态
        m_sendHistory->addEntry(text, isHex);
        m_sendInput->clear();
        m_sendInput->setProperty("hasError", false);
        m_sendInput->style()->unpolish(m_sendInput);
        m_sendInput->style()->polish(m_sendInput);
    }
}

/**
 * @brief 快捷指令触发处理
 * @param data 预编码的原始字节数据（已是 HEX 或 UTF-8 编码后的结果）
 */
void SendController::onQuickCommand(const QByteArray& data)
{
    sendAndRecord(data);
}

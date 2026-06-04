/**
 * @file SendControllerSend.cpp
 * @brief 发送控制器发送逻辑实现 — 数据发送、输入解析和快捷指令
 *
 * 从 SendController.cpp 拆分而来，包含sendAndRecord部分写入重试、
 * onSendData输入解析(文本/HEX)和onQuickCommand快捷指令处理。
 */

#include "core/send/SendController.h"
#include "core/send/SendHistoryManager.h"
#include "terminal/model/TerminalModel.h"
#include "utils/log/DataLogger.h"
#include "connection/interface/IConnection.h"
#include "utils/crypto/HexConverter.h"

#include <QLineEdit>
#include <QComboBox>

/**
 * @brief 统一发送方法 — 含部分写入重试策略
 *
 * 完整流程:
 *   1. 检查连接是否存在且处于已连接状态
 *   2. 循环写入数据到连接，最多重试 3 次部分写入
 *   3. 成功时追加到终端模型和日志
 *   4. 失败时通过 statusMessage 通知用户具体原因
 *
 * @param data 待发送的原始字节数据
 * @return true=至少写入了部分数据, false=写入完全失败或未连接
 */
bool SendController::sendAndRecord(const QByteArray& data)
{
    // 前置检查1: 连接是否存在
    if (!m_currentConn) {
        ++m_totalErrors;
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
        ++m_totalErrors;
        return false;
    }

    // ---- 部分写入重试循环 ----
    qint64 totalWritten = 0;
    int retryCount = 0;
    constexpr int kMaxPartialRetries = 3;

    while (totalWritten < data.size() && retryCount < kMaxPartialRetries) {
        int remaining = static_cast<int>(data.size() - totalWritten);
        qint64 written = m_currentConn->write(data.mid(static_cast<int>(totalWritten), remaining));
        if (written <= 0) {
            ++m_totalErrors;
            emit statusMessage(tr("发送失败: 写入返回 %1，已发送 %2/%3 字节")
                                   .arg(written).arg(totalWritten).arg(data.size()));
            return false;
        }
        totalWritten += written;
        if (totalWritten < data.size()) {
            ++retryCount;
        }
    }

    if (totalWritten < data.size()) {
        emit statusMessage(tr("部分写入: 请求 %1 字节，实际发送 %2 字节（重试 %3 次）")
                               .arg(data.size()).arg(totalWritten).arg(kMaxPartialRetries));
    }

    QByteArray sentData = data.left(static_cast<int>(totalWritten));
    m_terminalModel->appendSent(sentData);
    m_dataLogger->logData(sentData, DataLogger::Direction::Sent);
    ++m_totalSends;
    m_totalBytesSent += static_cast<quint64>(totalWritten);
    emit dataSent(totalWritten);
    return true;
}

/**
 * @brief 发送按钮/回车触发的发送逻辑
 *
 * 流程: 前置检查 → 文本/HEX解析 → 追加换行符 → sendAndRecord → 记录历史
 * HEX模式支持: "AA 55 01 00 FE", "AA,55,01,00,FE", "AA550100FE", "0xAA 0x55"
 */
void SendController::onSendData()
{
    if (!m_currentConn || m_currentConn->state() != ConnectionState::Connected) {
        emit statusMessage(tr("发送失败: 未连接"));
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
            emit statusMessage(tr("HEX 格式错误: 请输入有效的十六进制数据，如 \"AA 55 01 00 FE\""));
            return;
        }
    } else {
        data = text.toUtf8();
    }

    if (!isHex && m_newlineCombo && m_newlineCombo->currentIndex() > 0) {
        switch (m_newlineCombo->currentIndex()) {
        case 1: data.append("\r\n"); break;
        case 2: data.append("\n"); break;
        case 3: data.append("\r"); break;
        }
    }

    if (sendAndRecord(data)) {
        if (isHex) ++m_totalHexSends;
        m_historyManager->recordHistory(text, isHex);
        m_sendInput->clear();
        m_sendInput->setProperty("hasError", false);
        m_sendInput->style()->unpolish(m_sendInput);
        m_sendInput->style()->polish(m_sendInput);
    }
}

/** @brief 快捷指令触发处理 @param data 预编码的原始字节数据 */
void SendController::onQuickCommand(const QByteArray& data)
{
    ++m_totalMacroExecutions;
    sendAndRecord(data);
}

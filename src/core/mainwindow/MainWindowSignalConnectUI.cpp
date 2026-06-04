/**
 * @file MainWindowSignalConnectUI.cpp
 * @brief 主窗口信号/槽连接 - UI通知信号路由(重连/发送/热插拔/主题)
 *
 * 从 MainWindowSignalConnect.cpp 拆分而来，包含:
 *   connectReconnectSignals()   - 重连尝试/成功/失败状态通知
 *   connectSerialSendSignals()  - 快捷指令→发送控制器
 *   connectPortWatchSignals()   - 热插拔状态栏通知
 *   connectThemeSignals()       - 主题切换 + 连接Toast
 */

#include "core/mainwindow/MainWindow.h"
#include "core/widgets/ToastWidget.h"
#include "serial/port/PortWatcher.h"

/** @brief 自动重连状态指示信号路由（尝试/成功/失败→状态栏+Toast） */
void MainWindow::connectReconnectSignals()
{
    // 重连尝试中: 更新状态栏显示当前尝试次数
    connect(m_connController, &ConnectionController::reconnectAttempt,
            this, [this](int attempt, int maxRetries) {
        QString msg = maxRetries > 0
            ? tr("重连中... (第 %1/%2 次)").arg(attempt).arg(maxRetries)
            : tr("重连中... (第 %1 次)").arg(attempt);
        m_connStatusLbl->setText(msg);
        m_connStatusLbl->setProperty("state", "connecting");
        m_connStatusLbl->style()->unpolish(m_connStatusLbl);
        m_connStatusLbl->style()->polish(m_connStatusLbl);
    });
    // 重连进度通知(含指数退避间隔) -> Info类型吐司提示下次重连倒计时
    connect(m_connController, &ConnectionController::reconnectProgress,
            this, [this](int attempt, int maxRetries, int nextMs) {
        QString msg;
        if (maxRetries > 0) {
            msg = tr("正在重连... 第%1/%2次 (下次%3s后)")
                      .arg(attempt).arg(maxRetries).arg(nextMs / 1000.0, 0, 'f', 1);
        } else {
            msg = tr("正在重连... 第%1次 (下次%2s后)")
                      .arg(attempt).arg(nextMs / 1000.0, 0, 'f', 1);
        }
        ToastWidget::showDebounced(this, msg, ToastWidget::ToastType::Info, 2500);
    });
    // 重连成功: 状态栏由 handleConnectionState(Connected) 自动更新，此处仅显示 Toast
    connect(m_connController, &ConnectionController::reconnectSucceeded,
            this, [this](const QString& connName) {
        ToastWidget::show(this, tr("重连成功: %1").arg(connName),
                          ToastWidget::ToastType::Success);
    });
    // 重连最终失败: 更新状态栏 + 显示 Error 类型 Toast
    connect(m_connController, &ConnectionController::reconnectFailed,
            this, [this](const QString& reason) {
        m_connStatusLbl->setText(tr("重连失败"));
        m_connStatusLbl->setProperty("state", "error");
        m_connStatusLbl->style()->unpolish(m_connStatusLbl);
        m_connStatusLbl->style()->polish(m_connStatusLbl);
        ToastWidget::show(this, tr("重连失败: %1").arg(reason),
                          ToastWidget::ToastType::Error);
    });
}

/** @brief 快捷指令/发送控制器信号路由（发送→状态栏+防抖Toast） */
void MainWindow::connectSerialSendSignals()
{
    // 快捷指令 → 发送控制器
    connect(m_panelManager->quickCmdBar(), &QuickCommandBar::commandTriggered,
            m_sendController, &SendController::onQuickCommand);
    // 发送成功后更新状态栏
    connect(m_sendController, &SendController::dataSent,
            this, [this](qint64) { m_terminalController->updateStatusBar(); });
    // 发送状态消息显示到状态栏
    connect(m_sendController, &SendController::statusMessage,
            this, [this](const QString& msg) {
                statusBar()->showMessage(msg, 3000);
            });
    // 发送状态消息 → 防抖吐司通知（快速连续发送时可能频繁触发）
    connect(m_sendController, &SendController::statusMessage,
            this, [this](const QString& msg) {
                ToastWidget::showDebounced(this, msg);
            });
}

/** @brief 串口热插拔状态栏通知（新端口接入/端口拔出→状态栏提示） */
void MainWindow::connectPortWatchSignals()
{
    // 检测到新串口设备接入时，在状态栏显示提示信息
    connect(m_connController, &ConnectionController::portAdded,
            this, [this](const QString& portName) {
        statusBar()->showMessage(tr("检测到新端口: %1").arg(portName), 4000);
    });

    // 端口物理拔出时，额外在状态栏显示拔出提示
    connect(m_connController->portWatcher(), &PortWatcher::portRemoved,
            this, [this](const QString& portName) {
        statusBar()->showMessage(tr("端口已拔出: %1").arg(portName), 4000);
    });
}

/** @brief 主题切换信号连接 + 连接成功/断开/错误→Toast通知(含防抖策略) */
void MainWindow::connectThemeSignals()
{
    // 连接成功时显示 Success 类型吐司（一次性事件，无需防抖）
    connect(m_connController, &ConnectionController::connectionSucceeded,
            this, [this](const QString& portName) {
        ToastWidget::show(this, tr("已连接: %1").arg(portName),
                          ToastWidget::ToastType::Success);
    });
    // 用户主动断开连接时显示 Info 类型吐司（一次性事件，无需防抖）
    connect(m_connController, &ConnectionController::connectionDisconnected,
            this, [this](const QString& portName) {
        ToastWidget::show(this, tr("已断开: %1").arg(portName),
                          ToastWidget::ToastType::Info);
    });
    // 连接因错误中断 → 防抖吐司（Error 类型）
    connect(m_connController, &ConnectionController::connectionError,
            this, [this](const QString& portName, const QString& error) {
        ToastWidget::showDebounced(this, tr("连接错误: %1\n%2").arg(portName, error),
                                   ToastWidget::ToastType::Error, 3000);
    });
}

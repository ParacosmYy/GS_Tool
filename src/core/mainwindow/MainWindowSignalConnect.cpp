/**
 * @file MainWindowSignalConnect.cpp
 * @brief 主窗口信号/槽连接 - 串口连接/OTA/主题/书签信号路由
 *
 * 从 MainWindow.cpp 拆分，包含 connectSignals() 及以下子方法:
 *   connectSerialSignals()           - 串口连接/断开/DTR/RTS/波特率/数据/错误
 *   connectReconnectSignals()        - 重连尝试/成功/失败状态
 *   connectSerialSendSignals()       - 快捷指令 → 发送控制器
 *   connectPortWatchSignals()        - 热插拔通知
 *   connectThemeSignals()            - 主题切换 + 连接Toast
 *   connectOtaSignals()              - OTA传输通知
 *   connectBookmarkSignals()         - 书签面板CRUD
 *
 * 另见 MainWindowPanelConnect.cpp:
 *   connectToolbarSignals()          - 工具栏/录制状态消息
 *   connectSearchAndProtocolSignals() - 搜索/协议桥/帧编辑/导航树
 */

#include "core/mainwindow/MainWindow.h"
#include "chart/model/ChartModel.h"
#include "serial/port/PortWatcher.h"
#include "core/widgets/ToastWidget.h"
#include "core/theme/ThemeManager.h"
#include "serial/data/BookmarkWidget.h"
#include "ota/widget/OtaWidget.h"
#include "connection/serial_port/SerialConnection.h"

/**
 * @brief 连接所有模块间的信号/槽（调用 8 个子方法按功能分组）
 *
 * 子方法调用顺序:
 *   connectSerialSignals()           - 串口连接/断开/重连
 *   connectSerialSendSignals()       - 快捷指令/发送控制器
 *   connectToolbarSignals()          - 工具栏/录制状态消息
 *   connectSearchAndProtocolSignals() - 搜索/协议桥/帧编辑/导航
 *   connectPortWatchSignals()        - 热插拔通知
 *   connectThemeSignals()            - 主题切换 + Toast
 *   connectOtaSignals()              - OTA传输通知
 *   connectBookmarkSignals()         - 书签面板
 */
void MainWindow::connectSignals()
{
    connectSerialSignals();
    connectReconnectSignals();
    connectSerialSendSignals();
    connectToolbarSignals();
    connectSearchAndProtocolSignals();
    connectPortWatchSignals();
    connectThemeSignals();
    connectOtaSignals();
    connectBookmarkSignals();
}

/**
 * @brief 串口连接/断开/重连相关信号连接
 *
 * 包含: SerialConfigPanel → ConnectionController 的连接/断开/DTR/RTS 控制，
 *       ConnectionController → MainWindow 的状态更新/数据接收/失败通知/自动重连。
 */
void MainWindow::connectSerialSignals()
{
    // ---- 串口连接/断开: 委托 ConnectionController 处理 ----
    connect(m_panelManager->serialConfig(), &SerialConfigPanel::connectRequested,
            this, [this]() {
        QVariantMap params;
        params["portName"] = m_panelManager->serialConfig()->currentPortData();
        params["baudRate"] = m_panelManager->serialConfig()->currentBaudRate();
        params["dataBits"] = m_panelManager->serialConfig()->currentDataBitsIndex() + 5;
        params["parity"] = m_panelManager->serialConfig()->currentParityIndex();
        params["stopBits"] = m_panelManager->serialConfig()->currentStopBitsIndex();
        params["flowControl"] = m_panelManager->serialConfig()->currentFlowControlIndex();
        params["dtr"] = m_panelManager->serialConfig()->dtrEnabled();
        params["rts"] = m_panelManager->serialConfig()->rtsEnabled();
        m_connController->connectSerial(params);
    });
    connect(m_panelManager->serialConfig(), &SerialConfigPanel::disconnectRequested,
            m_connController, &ConnectionController::disconnectCurrent);
    // DTR/RTS 线路控制信号直连
    connect(m_panelManager->serialConfig(), &SerialConfigPanel::dtrChanged,
            m_connController, &ConnectionController::setDtr);
    connect(m_panelManager->serialConfig(), &SerialConfigPanel::rtsChanged,
            m_connController, &ConnectionController::setRts);
    // Break 信号 → 连接控制器
    connect(m_panelManager->serialConfig(), &SerialConfigPanel::breakRequested,
            m_connController, &ConnectionController::sendBreak);
    // 自动重连开关 → 连接控制器
    connect(m_panelManager->serialConfig(), &SerialConfigPanel::autoReconnectToggled,
            this, [this](bool enabled, int intervalMs) {
        m_connController->enableAutoReconnect(enabled, intervalMs);
    });
    // 运行时波特率切换: 用户在连接状态下更改波特率
    connect(m_panelManager->serialConfig(), &SerialConfigPanel::baudRateChanged,
            this, [this](qint32 baud) {
        auto* conn = m_connController->currentConnection();
        if (conn && conn->type() == ConnectionType::Serial) {
            if (auto* serial = qobject_cast<SerialConnection*>(conn)) {
                serial->setBaudRate(baud);
                ToastWidget::show(this, tr("波特率已切换为 %1").arg(baud),
                                  ToastWidget::ToastType::Info);
            }
        }
    });
    // 连接状态变化 -> 更新 UI（状态栏文本、配置面板按钮状态、呼吸动画）
    connect(m_connController, &ConnectionController::connectionStateChanged,
            this, [this](ConnectionState state, const QString& connName) {
        handleConnectionState(state, connName);
    });
    // 通信错误计数更新 -> 数据统计面板（表现层直接连接，避免业务层依赖表现层）
    connect(m_connController, &ConnectionController::errorCountersUpdated,
            this, [this](int framing, int parity, int overrun) {
        m_panelManager->dataStats()->updateErrors(framing, parity, overrun);
    });
    // 连接健康状态 -> 数据统计面板（空闲超10秒时显示提示）
    connect(m_connController, &ConnectionController::connectionHealth,
            this, [this](bool alive, qint64 lastDataAgeMs) {
        m_panelManager->dataStats()->updateConnectionHealth(alive, lastDataAgeMs);
    });
    // 信号线状态变化 -> 更新串口配置面板LED指示灯
    connect(m_connController, &ConnectionController::pinoutSignalsChanged,
            m_panelManager->serialConfig(), &SerialConfigPanel::updatePinoutLeds);

    // 数据流 + 状态/错误通知
    connectSerialDataFlow();
}

/** @brief 串口数据流 + 状态/错误信号连接 */
void MainWindow::connectSerialDataFlow()
{
    // 接收数据 -> 终端模型 + 协议解析 + 日志记录
    connect(m_connController, &ConnectionController::dataReceived,
            this, [this](const QByteArray& data) {
        m_terminalModel->appendReceived(data);
        m_protocolBridgeMgr->feedData(data);
        m_dataLogger->logData(data, DataLogger::Direction::Received);
    });

    // 状态栏更新和连接失败通知
    connect(m_connController, &ConnectionController::statusBarUpdateRequested,
            m_terminalController, &TerminalController::updateStatusBar);
    // 连接失败 → 仅Toast通知（不再使用QMessageBox，避免自动重连时弹窗叠加阻塞UI）
    // 使用 showDebounced 防止同一错误消息在 3 秒内重复弹出
    connect(m_connController, &ConnectionController::connectionFailed,
            this, [this](const QString&, const QString& message) {
        ToastWidget::showDebounced(this, message, ToastWidget::ToastType::Error, 3000);
    });
}

/**
 * @brief 自动重连状态指示信号连接
 *
 * 包含: 重连尝试次数 → 状态栏文本更新，
 *       重连成功/失败 → Toast 通知。
 */
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

/**
 * @brief 快捷指令和发送控制器信号连接
 *
 * 包含: QuickCommandBar → SendController 快捷指令发送，
 *       SendController → TerminalController 状态栏更新，
 *       SendController → ToastWidget 防抖吐司通知。
 */
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

/**
 * @brief 串口热插拔状态栏通知连接
 *
 * 检测串口设备的物理接入/拔出事件，在状态栏显示提示信息。
 */
void MainWindow::connectPortWatchSignals()
{
    // ---- 串口热插拔状态栏通知 ----
    // 检测到新串口设备接入时，在状态栏显示提示信息
    connect(m_connController, &ConnectionController::portAdded,
            this, [this](const QString& portName) {
        statusBar()->showMessage(tr("检测到新端口: %1").arg(portName), 4000);
    });

    // 端口物理拔出时，除了 ConnectionController 自动断开连接外，
    // 额外在状态栏显示拔出提示（通过 PortWatcher 的 portRemoved 信号）
    connect(m_connController->portWatcher(), &PortWatcher::portRemoved,
            this, [this](const QString& portName) {
        statusBar()->showMessage(tr("端口已拔出: %1").arg(portName), 4000);
    });
}

/**
 * @brief 主题切换信号连接
 *
 * 包含: ThemeManager → NavIndicatorWidget 颜色刷新，
 *       连接成功/断开/错误 → Toast 通知（含防抖策略说明）。
 */
void MainWindow::connectThemeSignals()
{
    // ---- NavIndicatorWidget 主题刷新已在构造函数中连接，此处无需重复 ----
    // (移除重复连接，NavIndicatorWidget构造函数已连接themeChanged→updateThemeColor)

    /**
     * @name Toast 防抖策略
     *
     * 吐司通知的 debounce 策略:
     *
     * **使用 showDebounced()（防抖）** — 可能因自动重连、网络抖动等场景快速重复触发的信号:
     *   - connectionFailed: 自动重连失败时会连续触发，3 秒冷却避免重复弹窗
     *   - connectionError:  连接中途因错误断开，重连时可能重复触发，3 秒冷却
     *   - SendController::statusMessage: 快速连续发送时可能频繁触发，2 秒冷却（默认值）
     *   - OtaWidget::transferFailed: 传输失败重试时可能连续触发，3 秒冷却
     *
     * **使用 show()（无防抖）** — 确定性的一次性事件，不会在短时间内重复:
     *   - connectionSucceeded: 连接成功是一次性事件
     *   - connectionDisconnected: 用户主动断开是一次性事件
     *   - OtaWidget::transferStarted: 传输开始是一次性事件
     *   - OtaWidget::transferCompleted: 传输完成是一次性事件（含耗时/大小信息，不应丢弃）
     *   - RecordingController::statusMessage: 录制/回放状态切换是一次性事件
     *
     * 冷却键 = ToastType 编号 + "|" + 消息文本，同一消息+类型在冷却期内静默跳过。
     * 不同消息（如不同端口的错误）互不影响，各自独立计时。
     */
    ///@{

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
    // 连接因错误中断 → 防抖吐司（Error 类型）— 自动重连/网络抖动时可能快速重复触发，
    // 使用 showDebounced 防止同一端口+错误消息在 3 秒内重复弹出
    connect(m_connController, &ConnectionController::connectionError,
            this, [this](const QString& portName, const QString& error) {
        ToastWidget::showDebounced(this, tr("连接错误: %1\n%2").arg(portName, error),
                                   ToastWidget::ToastType::Error, 3000);
    });
    ///@}
}

/**
 * @brief OTA 传输状态信号连接
 *
 * 包含: OtaWidget 传输开始/完成/失败 → Toast 通知（含防抖策略）。
 */
void MainWindow::connectOtaSignals()
{
    // ---- OTA 传输状态 → 吐司通知 ----
    // 传输开始 → 非防抖吐司（Info 类型，一次性事件）
    connect(m_panelManager->otaWidget(), &OtaWidget::transferStarted,
            this, [this](const QString& filename) {
        ToastWidget::show(this, tr("开始传输: %1").arg(filename),
                          ToastWidget::ToastType::Info);
    });
    // 传输完成 → 非防抖吐司（Success 类型，含耗时和文件大小，一次性事件）
    connect(m_panelManager->otaWidget(), &OtaWidget::transferCompleted,
            this, [this](const QString& filename, int elapsed, int size) {
        // 耗时格式化: 秒或毫秒
        QString timeStr = elapsed >= 1000
            ? tr("%1秒").arg(elapsed / 1000.0, 0, 'f', 1)
            : tr("%1毫秒").arg(elapsed);
        // 文件大小格式化: KB 或 字节
        QString sizeStr = size >= 1024
            ? tr("%1 KB").arg(size / 1024.0, 0, 'f', 1)
            : tr("%1 字节").arg(size);
        ToastWidget::show(this, tr("传输完成: %1 (%2, %3)")
                              .arg(filename, timeStr, sizeStr),
                          ToastWidget::ToastType::Success);
    });
    // 传输失败 → 防抖吐司（Error 类型，含错误原因）— 重试或连续传输失败时可能快速重复触发，
    // 使用 showDebounced 防止同一文件+错误消息在 3 秒内重复弹出
    connect(m_panelManager->otaWidget(), &OtaWidget::transferFailed,
            this, [this](const QString& filename, const QString& error) {
        ToastWidget::showDebounced(this, tr("传输失败: %1\n%2").arg(filename, error),
                                   ToastWidget::ToastType::Error, 3000);
    });
    ///@}
}

/**
 * @brief 书签面板信号路由
 *
 * 包含: RecordingController → DataLogger 的书签添加路由 + Toast 反馈，
 *       DataLogger::bookmarksChanged → 状态栏消息，
 *       BookmarkWidget ↔ DataLogger 的书签 CRUD 操作。
 */
void MainWindow::connectBookmarkSignals()
{
    /**
     * @name 书签信号路由（DataBookmark 集成）
     *
     * 书签信号连接负责将 UI 层的书签添加请求路由到数据层，
     * 并通过吐司通知向用户提供即时反馈。
     *
     * 信号流向:
     *   1. RecordingController::addBookmarkRequested(label)
     *        → DataLogger::addBookmark(label)
     *        UI 层书签请求路由到数据层，DataLogger 创建 DataBookmark 并发射 bookmarksChanged
     *
     *   2. RecordingController::addBookmarkRequested(label)
     *        → ToastWidget::show("书签已添加: label", Success)
     *        使用 show() 非防抖吐司，因为书签添加是用户主动触发的确定性一次性事件
     *
     *   3. DataLogger::bookmarksChanged()
     *        → 状态栏消息更新
     *        当书签集合发生变化（增/删/清空）时通知状态栏
     */
    ///@{

    // 书签添加请求 → DataLogger：将 UI 层请求路由到数据层
    // RecordingController 由用户交互（工具栏按钮/快捷键）触发，
    // DataLogger::addBookmark 会自动生成时间戳并发射 bookmarksChanged
    connect(m_recordingController, &RecordingController::addBookmarkRequested,
            m_dataLogger, [this](const QString& label) {
        m_dataLogger->addBookmark(label);
    });

    // 书签添加请求 → Success 吐司：即时 UI 反馈
    // 使用 show() 非防抖，因为用户主动添加书签是确定性的一次性事件
    connect(m_recordingController, &RecordingController::addBookmarkRequested,
            this, [this](const QString& label) {
        ToastWidget::show(this, tr("书签已添加: %1").arg(label),
                          ToastWidget::ToastType::Success);
    });

    // 书签集合变化 → 状态栏消息：通知数据层书签列表已更新
    // DataLogger 在 addBookmark/removeBookmark/clearBookmarks 后发射此信号
    connect(m_dataLogger, &DataLogger::bookmarksChanged,
            this, [this]() {
        statusBar()->showMessage(
            tr("书签列表已更新 (%1)").arg(m_dataLogger->bookmarks().size()), 3000);
    });

    // ---- BookmarkWidget 书签面板信号路由 ----
    // BookmarkWidget 作为面板提供可视化的书签管理界面，
    // 所有书签操作通过信号路由到 DataLogger 执行

    // BookmarkWidget 添加书签请求 → RecordingController 转发 → DataLogger
    // 用户在书签面板点击"添加书签"按钮时触发，通过 RecordingController 转发
    connect(m_panelManager->bookmarkWidget(), &BookmarkWidget::addBookmarkRequested,
            m_recordingController, &RecordingController::addBookmarkRequested);

    // DataLogger 书签变化 → BookmarkWidget 列表刷新
    // 当 DataLogger 的书签集合发生变化时，刷新书签面板的列表显示
    connect(m_dataLogger, &DataLogger::bookmarksChanged,
            this, [this]() {
        m_panelManager->bookmarkWidget()->refreshBookmarks(m_dataLogger->bookmarks());
    });

    // BookmarkWidget 删除请求 → DataLogger 删除指定书签
    connect(m_panelManager->bookmarkWidget(), &BookmarkWidget::removeBookmarkRequested,
            m_dataLogger, &DataLogger::removeBookmark);

    // BookmarkWidget 清空请求 → DataLogger 清空所有书签
    connect(m_panelManager->bookmarkWidget(), &BookmarkWidget::clearBookmarksRequested,
            m_dataLogger, &DataLogger::clearBookmarks);

    // BookmarkWidget 双击书签 → DataLogger 跳转到书签时间点（回放模式下有效）
    connect(m_panelManager->bookmarkWidget(), &BookmarkWidget::bookmarkDoubleClicked,
            m_dataLogger, &DataLogger::seekToBookmark);
    ///@}
}

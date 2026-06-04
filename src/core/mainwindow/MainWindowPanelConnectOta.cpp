/**
 * @file MainWindowPanelConnectOta.cpp
 * @brief 主窗口OTA传输状态与书签面板信号连接
 *
 * 从MainWindowPanelConnect.cpp拆分而来，包含:
 *   - connectOtaSignals(): OTA传输状态信号连接(开始/完成/失败->Toast通知，含防抖策略)
 *   - connectBookmarkSignals(): 书签面板信号路由(添加/删除/清空/跳转->DataLogger+Toast+状态栏)
 *
 * 信号流:
 *   OtaWidget -> ToastWidget(传输状态通知)
 *   RecordingController -> DataLogger(书签CRUD) + ToastWidget(反馈)
 *   BookmarkWidget -> DataLogger(书签操作)
 *
 * 工具栏/搜索/协议桥信号连接保留在MainWindowPanelConnect.cpp中。
 */

#include "core/mainwindow/MainWindow.h"
#include "core/widgets/ToastWidget.h"
#include "serial/data/BookmarkWidget.h"
#include "ota/widget/OtaWidget.h"

/** @brief OTA传输状态信号连接(开始/完成/失败->Toast通知，含防抖策略) */
void MainWindow::connectOtaSignals()
{
    // ---- OTA 传输状态 -> 吐司通知 ----
    // 传输开始 -> 非防抖吐司（Info 类型，一次性事件）
    connect(m_panelManager->otaWidget(), &OtaWidget::transferStarted,
            this, [this](const QString& filename) {
        ToastWidget::show(this, tr("开始传输: %1").arg(filename),
                          ToastWidget::ToastType::Info);
    });
    // 传输完成 -> 非防抖吐司（Success 类型，含耗时和文件大小，一次性事件）
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
    // 传输失败 -> 防抖吐司（Error 类型，含错误原因）— 重试或连续传输失败时可能快速重复触发，
    // 使用 showDebounced 防止同一文件+错误消息在 3 秒内重复弹出
    connect(m_panelManager->otaWidget(), &OtaWidget::transferFailed,
            this, [this](const QString& filename, const QString& error) {
        ToastWidget::showDebounced(this, tr("传输失败: %1\n%2").arg(filename, error),
                                   ToastWidget::ToastType::Error, 3000);
    });
}

/** @brief 书签面板信号路由(添加/删除/清空/跳转->DataLogger+Toast+状态栏) */
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
     *        -> DataLogger::addBookmark(label)
     *        UI 层书签请求路由到数据层，DataLogger 创建 DataBookmark 并发射 bookmarksChanged
     *
     *   2. RecordingController::addBookmarkRequested(label)
     *        -> ToastWidget::show("书签已添加: label", Success)
     *        使用 show() 非防抖吐司，因为书签添加是用户主动触发的确定性一次性事件
     *
     *   3. DataLogger::bookmarksChanged()
     *        -> 状态栏消息更新
     *        当书签集合发生变化（增/删/清空）时通知状态栏
     */
    ///@{

    // 书签添加请求 -> DataLogger：将 UI 层请求路由到数据层
    // RecordingController 由用户交互（工具栏按钮/快捷键）触发，
    // DataLogger::addBookmark 会自动生成时间戳并发射 bookmarksChanged
    connect(m_recordingController, &RecordingController::addBookmarkRequested,
            m_dataLogger, [this](const QString& label) {
        m_dataLogger->addBookmark(label);
    });

    // 书签添加请求 -> Success 吐司：即时 UI 反馈
    // 使用 show() 非防抖，因为用户主动添加书签是确定性的一次性事件
    connect(m_recordingController, &RecordingController::addBookmarkRequested,
            this, [this](const QString& label) {
        ToastWidget::show(this, tr("书签已添加: %1").arg(label),
                          ToastWidget::ToastType::Success);
    });

    // 书签集合变化 -> 状态栏消息：通知数据层书签列表已更新
    // DataLogger 在 addBookmark/removeBookmark/clearBookmarks 后发射此信号
    connect(m_dataLogger, &DataLogger::bookmarksChanged,
            this, [this]() {
        statusBar()->showMessage(
            tr("书签列表已更新 (%1)").arg(m_dataLogger->bookmarks().size()), 3000);
    });

    // ---- BookmarkWidget 书签面板信号路由 ----
    // BookmarkWidget 作为面板提供可视化的书签管理界面，
    // 所有书签操作通过信号路由到 DataLogger 执行

    // BookmarkWidget 添加书签请求 -> RecordingController 转发 -> DataLogger
    // 用户在书签面板点击"添加书签"按钮时触发，通过 RecordingController 转发
    connect(m_panelManager->bookmarkWidget(), &BookmarkWidget::addBookmarkRequested,
            m_recordingController, &RecordingController::addBookmarkRequested);

    // DataLogger 书签变化 -> BookmarkWidget 列表刷新
    // 当 DataLogger 的书签集合发生变化时，刷新书签面板的列表显示
    connect(m_dataLogger, &DataLogger::bookmarksChanged,
            this, [this]() {
        m_panelManager->bookmarkWidget()->refreshBookmarks(m_dataLogger->bookmarks());
    });

    // BookmarkWidget 删除请求 -> DataLogger 删除指定书签
    connect(m_panelManager->bookmarkWidget(), &BookmarkWidget::removeBookmarkRequested,
            m_dataLogger, &DataLogger::removeBookmark);

    // BookmarkWidget 清空请求 -> DataLogger 清空所有书签
    connect(m_panelManager->bookmarkWidget(), &BookmarkWidget::clearBookmarksRequested,
            m_dataLogger, &DataLogger::clearBookmarks);

    // BookmarkWidget 双击书签 -> DataLogger 跳转到书签时间点（回放模式下有效）
    connect(m_panelManager->bookmarkWidget(), &BookmarkWidget::bookmarkDoubleClicked,
            m_dataLogger, &DataLogger::seekToBookmark);
    ///@}
}

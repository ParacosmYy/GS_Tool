/**
 * @file MainWindow.cpp
 * @brief 主窗口实现 - 拆分索引文件
 *
 * MainWindow 的所有方法已拆分到以下文件中，本文件仅作为索引入口:
 *
 *   MainWindowInit.cpp            - 构造函数与功能初始化（依赖注入+UI构建+导航+快捷键）
 *   MainWindowSetupUI.cpp         - UI布局构建（setupUI/createNavigationArea/createContentArea/setupStatusBar）
 *   MainWindowSignalConnect.cpp   - 信号/槽连接（串口/重连/发送/热插拔/主题/OTA/书签）
 *   MainWindowPanelConnect.cpp    - 面板交互信号连接（工具栏/搜索/协议/帧编辑/导航）
 *   MainWindowLifecycle.cpp       - 生命周期与事件处理（restoreUserSession/closeEvent/连接状态）
 */

// 本文件的所有方法实现已迁移到上述拆分文件中。
// MainWindow 构造函数见 MainWindowInit.cpp。

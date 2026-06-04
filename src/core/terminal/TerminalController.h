/** @file TerminalController.h @brief 终端控制器 - 管理终端显示、搜索、导出和数据统计的交互逻辑 */
#ifndef TERMINALCONTROLLER_H
#define TERMINALCONTROLLER_H

#include <QObject>
#include <QTimer>

class TerminalModel;
class TerminalLayoutManager;
class DataExporter;
class DataStatistics;
class QLabel;
class QWidget;

/**
 * @brief 终端控制器 - 管理终端显示、搜索、导出和数据统计的交互逻辑
 *
 * 职责: 显示模式切换/时间戳开关/清屏/搜索高亮/布局切换/状态栏刷新/定时统计刷新/数据导出
 * 协作: TerminalLayoutManager(活动终端列表) / TerminalModel(数据源) / DataExporter(导出) / DataStatistics(统计面板)
 */
class TerminalController : public QObject {
    Q_OBJECT

public:
    /** @brief 构造终端控制器 @param model 终端数据模型 @param exporter 数据导出器 @param parent 父对象 */
    explicit TerminalController(TerminalModel* model, DataExporter* exporter,
                                QObject* parent = nullptr);
    ~TerminalController() override = default;
    void setLayoutManager(TerminalLayoutManager* manager); ///< 注入终端布局管理器
    void setDataStatistics(DataStatistics* stats);         ///< 注入数据统计面板
    /** @brief 注入状态栏字节标签 @param rxLabel 接收字节标签 @param txLabel 发送字节标签 */
    void setStatusBarLabels(QLabel* rxLabel, QLabel* txLabel);
    void setMainTerminal(QWidget* terminal); ///< 注入主终端widget(搜索只作用于主终端)
    void startStatsTimer();                  ///< 启动统计刷新定时器(每500ms)
    void stopStatsTimer();                   ///< 停止统计刷新定时器(窗口关闭时调用)

public slots:
    /** @brief 终端显示模式切换 @param index 0=文本,1=HEX,2=混合,3=十进制 */
    void onDisplayModeChanged(int index);
    void onTimestampToggled(bool checked);   ///< 时间戳显示开关，同步到所有活动终端
    void onDirPrefixToggled(bool checked);   ///< 收发方向前缀开关，同步到所有活动终端
    void onClearTerminal();                  ///< 清空终端内容、重置统计面板、刷新状态栏
    /** @brief 终端搜索请求 @param pattern 搜索模式 @param regex 正则 @param hex HEX字节 @param caseSensitive 区分大小写 @param wholeWord 全词匹配 */
    void onSearchRequested(const QString& pattern, bool regex, bool hex,
                           bool caseSensitive, bool wholeWord);
    void onSearchCleared();                  ///< 清除主终端的搜索高亮
    /** @brief 终端布局模式切换 @param index 0=混合,1=左右分栏,2=上下分栏 */
    void onTerminalLayoutChanged(int index);
    void updateStatusBar();                  ///< 刷新状态栏RX/TX字节数(自动格式化B/KB/MB)
    /** @brief 导出终端数据到文件(TXT/CSV/BIN) @param parent 文件对话框父窗口 */
    void onExportData(QWidget* parent);

signals:
    /** @brief 状态栏消息通知 @param msg 消息文本 @param timeoutMs 显示时长(ms) */
    void statusMessage(const QString& msg, int timeoutMs = 0);

private:
    void updateDataStatistics();             ///< 定时刷新数据统计面板
    // ==================== 核心依赖 ====================
    TerminalModel* m_terminalModel;          ///< 终端数据模型，提供RX/TX字节数和行数据
    DataExporter* m_dataExporter;            ///< 数据导出器(TXT/CSV/BIN流式批量导出)
    TerminalLayoutManager* m_layoutManager;  ///< 终端布局管理器(混合/分栏三种模式)
    DataStatistics* m_dataStats;             ///< 数据统计面板
    // ==================== 主终端引用 ====================
    QWidget* m_mainTerminal;                 ///< 主终端widget(搜索只作用于主终端)
    // ==================== 状态栏组件 ====================
    QLabel* m_rxBytesLbl;                    ///< 接收字节标签 "RX: xxx B/KB/MB"
    QLabel* m_txBytesLbl;                    ///< 发送字节标签 "TX: xxx B/KB/MB"
    // ==================== 定时器 ====================
    QTimer* m_statsTimer;                    ///< 统计刷新定时器(每500ms)
    bool m_statsSignalConnected = false;     ///< 防止重复connect timeout信号
    // ---- 统计计数器 ----
    quint64 m_totalDisplayModeChanges = 0;  ///< 累计显示模式切换次数
    quint64 m_totalClears = 0;              ///< 累计清屏次数
    quint64 m_totalSearches = 0;            ///< 累计搜索次数
    quint64 m_totalExports = 0;             ///< 累计导出次数
    quint64 m_totalScrollToBottom = 0;      ///< 累计滚动到底部次数
    quint64 m_totalBufferOverflows = 0;     ///< 累计缓冲区溢出次数
public:
    quint64 totalDisplayModeChanges() const { return m_totalDisplayModeChanges; }
    quint64 totalClears() const { return m_totalClears; }
    quint64 totalSearches() const { return m_totalSearches; }
    quint64 totalExports() const { return m_totalExports; }
    quint64 totalScrollToBottom() const { return m_totalScrollToBottom; } ///< 累计滚动到底部次数
    quint64 totalBufferOverflows() const { return m_totalBufferOverflows; } ///< 累计缓冲区溢出次数
    void resetTerminalControllerStatistics() { m_totalDisplayModeChanges = 0; m_totalClears = 0; m_totalSearches = 0; m_totalExports = 0; }
    /** @brief 重置所有终端控制器统计计数器(含滚动/缓冲区溢出) */
    void resetStats() { m_totalDisplayModeChanges = 0; m_totalClears = 0; m_totalSearches = 0; m_totalExports = 0; m_totalScrollToBottom = 0; m_totalBufferOverflows = 0; }
};

#endif // TERMINALCONTROLLER_H

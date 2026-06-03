/**
 * @file TerminalController.h
 * @brief 终端控制器 - 管理终端显示、搜索、导出和数据统计的交互逻辑
 */

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
 * 职责:
 *   1. 终端显示模式切换（文本/HEX/混合/十进制），同步到所有活动终端 widget
 *   2. 时间戳/方向前缀开关，同步到所有活动终端 widget
 *   3. 清空终端内容和重置数据统计
 *   4. 终端搜索高亮和清除（搜索只作用于主终端）
 *   5. 终端布局模式切换（混合/左右分栏/上下分栏）
 *   6. 状态栏 RX/TX 字节数格式化和刷新
 *   7. 定时刷新数据统计面板（每 500ms 触发一次）
 *   8. 终端数据导出到文件（支持 TXT/CSV/BIN 格式）
 *
 * 协作关系:
 *   - TerminalLayoutManager: 提供当前活动的终端 widget 列表
 *   - TerminalModel: 终端数据源，提供 RX/TX 字节数和行数据
 *   - DataExporter: 实际的文件导出引擎
 *   - DataStatistics: 统计面板，接收 RX/TX 字节更新
 */
class TerminalController : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 构造终端控制器
     * @param model 终端数据模型，用于读取 RX/TX 字节数和行数据
     * @param exporter 数据导出器，用于将终端数据导出为文件
     * @param parent 父对象
     */
    explicit TerminalController(TerminalModel* model, DataExporter* exporter,
                                QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~TerminalController() override = default;

    /** @brief 注入终端布局管理器，用于获取活动终端列表和切换布局 */
    void setLayoutManager(TerminalLayoutManager* manager);

    /** @brief 注入数据统计面板，用于定时刷新 RX/TX 累计字节数 */
    void setDataStatistics(DataStatistics* stats);

    /**
     * @brief 注入状态栏字节标签，用于格式化显示 RX/TX 字节数
     * @param rxLabel 接收字节计数标签
     * @param txLabel 发送字节计数标签
     */
    void setStatusBarLabels(QLabel* rxLabel, QLabel* txLabel);

    /** @brief 注入主终端 widget，搜索高亮只作用于主终端 */
    void setMainTerminal(QWidget* terminal);

    /** @brief 启动统计刷新定时器（每 500ms 触发一次） */
    void startStatsTimer();

    /** @brief 停止统计刷新定时器（窗口关闭时调用） */
    void stopStatsTimer();

public slots:
    /**
     * @brief 终端显示模式切换
     * @param index 下拉框索引: 0=文本, 1=HEX, 2=混合, 3=十进制
     */
    void onDisplayModeChanged(int index);

    /** @brief 时间戳显示开关，同步到所有活动终端 widget */
    void onTimestampToggled(bool checked);

    /** @brief 收发方向前缀开关，同步到所有活动终端 widget */
    void onDirPrefixToggled(bool checked);

    /** @brief 清空终端内容、重置数据统计面板、刷新状态栏 */
    void onClearTerminal();

    /**
     * @brief 终端搜索请求处理
     * @param pattern 搜索模式字符串
     * @param regex true=使用正则表达式匹配
     * @param hex true=按 HEX 字节搜索
     * @param caseSensitive true=区分大小写
     * @param wholeWord true=全词匹配
     */
    void onSearchRequested(const QString& pattern, bool regex, bool hex,
                           bool caseSensitive, bool wholeWord);

    /** @brief 清除主终端的搜索高亮 */
    void onSearchCleared();

    /**
     * @brief 终端布局模式切换
     * @param index 下拉框索引: 0=混合, 1=左右分栏, 2=上下分栏
     */
    void onTerminalLayoutChanged(int index);

    /** @brief 刷新状态栏 RX/TX 字节数显示，自动格式化为 B/KB/MB */
    void updateStatusBar();

    /**
     * @brief 导出终端数据到文件（TXT/CSV/BIN）
     * 使用批量流式导出，通过 lineProvider 回调分批拉取数据避免深拷贝
     * @param parent 用于定位文件对话框的父窗口
     */
    void onExportData(QWidget* parent);

signals:
    /**
     * @brief 状态栏消息通知信号
     * @param msg 消息文本
     * @param timeoutMs 消息显示时长（毫秒），0=使用默认值
     */
    void statusMessage(const QString& msg, int timeoutMs = 0);

private:
    /** @brief 定时刷新数据统计面板 */
    void updateDataStatistics();

    // ==================== 核心依赖 ====================

    /** @brief 终端数据模型，提供 RX/TX 字节数和行数据 */
    TerminalModel* m_terminalModel;

    /** @brief 数据导出器，支持 TXT/CSV/BIN 格式的流式批量导出 */
    DataExporter* m_dataExporter;

    /** @brief 终端布局管理器，管理混合/分栏三种布局模式 */
    TerminalLayoutManager* m_layoutManager;

    /** @brief 数据统计面板，接收 RX/TX 字节累计更新 */
    DataStatistics* m_dataStats;

    // ==================== 主终端引用 ====================

    /** @brief 主终端 widget（搜索只作用于主终端），实际类型为 TerminalWidget* */
    QWidget* m_mainTerminal;

    // ==================== 状态栏组件 ====================

    /** @brief 接收字节计数标签，格式: "RX: xxx B/KB/MB" */
    QLabel* m_rxBytesLbl;

    /** @brief 发送字节计数标签，格式: "TX: xxx B/KB/MB" */
    QLabel* m_txBytesLbl;

    // ==================== 定时器 ====================

    /** @brief 统计刷新定时器，每 500ms 触发一次 updateDataStatistics() */
    QTimer* m_statsTimer;

    /** @brief 标记 statsTimer 的 timeout 信号是否已连接，防止重复 connect */
    bool m_statsSignalConnected = false;

    // ---- 统计计数器 ----
    quint64 m_totalDisplayModeChanges = 0; ///< 累计显示模式切换次数
    quint64 m_totalClears = 0;             ///< 累计清屏次数
    quint64 m_totalSearches = 0;           ///< 累计搜索次数
    quint64 m_totalExports = 0;            ///< 累计导出次数
public:
    quint64 totalDisplayModeChanges() const { return m_totalDisplayModeChanges; }
    quint64 totalClears() const { return m_totalClears; }
    quint64 totalSearches() const { return m_totalSearches; }
    quint64 totalExports() const { return m_totalExports; }
    void resetTerminalControllerStatistics() { m_totalDisplayModeChanges = 0; m_totalClears = 0; m_totalSearches = 0; m_totalExports = 0; }
};

#endif // TERMINALCONTROLLER_H

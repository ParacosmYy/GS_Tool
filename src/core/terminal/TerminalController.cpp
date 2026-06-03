/**
 * @file TerminalController.cpp
 * @brief 终端控制器实现 - 管理终端显示、搜索、导出和数据统计的交互逻辑
 *
 * 本文件实现 TerminalController 的所有方法，将原先散落在 MainWindow 中的
 * 终端相关业务逻辑集中管理，使 MainWindow 只负责 UI 布局和信号连接。
 */

#include "core/terminal/TerminalController.h"
#include "terminal/model/TerminalModel.h"
#include "terminal/widget/TerminalWidget.h"
#include "terminal/layout/TerminalLayoutManager.h"
#include "serial/data/DataStatistics.h"
#include "utils/export/DataExporter.h"
#include "utils/data/ByteFormat.h"
#include "core/theme/Constants.h"

#include <QFileDialog>
#include "core/widgets/EdDialog.h"
#include <QLabel>

/**
 * @brief 构造终端控制器
 * @param model 终端数据模型，用于读取 RX/TX 字节数和行数据
 * @param exporter 数据导出器，用于将终端数据导出为文件
 * @param parent 父对象
 */
TerminalController::TerminalController(TerminalModel* model, DataExporter* exporter,
                                       QObject* parent)
    : QObject(parent)
    , m_terminalModel(model)
    , m_dataExporter(exporter)
    , m_layoutManager(nullptr)
    , m_dataStats(nullptr)
    , m_mainTerminal(nullptr)
    , m_rxBytesLbl(nullptr)
    , m_txBytesLbl(nullptr)
    , m_statsTimer(new QTimer(this))
{
}

/** @brief 注入终端布局管理器引用 */
void TerminalController::setLayoutManager(TerminalLayoutManager* manager)
{
    m_layoutManager = manager;
}

/** @brief 注入数据统计面板引用 */
void TerminalController::setDataStatistics(DataStatistics* stats)
{
    m_dataStats = stats;
}

/**
 * @brief 注入状态栏字节标签引用
 * @param rxLabel 接收字节计数标签
 * @param txLabel 发送字节计数标签
 */
void TerminalController::setStatusBarLabels(QLabel* rxLabel, QLabel* txLabel)
{
    m_rxBytesLbl = rxLabel;
    m_txBytesLbl = txLabel;
}

/** @brief 注入主终端 widget 引用（搜索只作用于主终端） */
void TerminalController::setMainTerminal(QWidget* terminal)
{
    m_mainTerminal = terminal;
}

/** @brief 启动统计刷新定时器（每 500ms 触发一次） */
void TerminalController::startStatsTimer()
{
    m_statsTimer->setInterval(Timers::kStatsRefreshMs);
    // 仅首次连接信号，防止重复调用导致 updateDataStatistics 多次触发
    // 使用成员变量代替 static bool，支持 TerminalController 销毁重建时正确重连
    if (!m_statsSignalConnected) {
        connect(m_statsTimer, &QTimer::timeout, this, &TerminalController::updateDataStatistics);
        m_statsSignalConnected = true;
    }
    m_statsTimer->start();
}

/** @brief 停止统计刷新定时器 */
void TerminalController::stopStatsTimer()
{
    m_statsTimer->stop();
}

/**
 * @brief 终端显示模式切换
 * @param index 下拉框索引: 0=文本, 1=HEX, 2=混合, 3=十进制
 */
void TerminalController::onDisplayModeChanged(int index)
{
    if (!m_layoutManager) return;
    if (index < 0 || index > 3) return;  // 防御: ComboBox index 越界保护

    DisplayMode modes[] = {DisplayMode::Text, DisplayMode::Hex, DisplayMode::Mixed, DisplayMode::Decimal};
    // 显示模式切换: 同步到所有活动的终端widget（主终端+分栏终端）
    for (auto* tw : m_layoutManager->terminalWidgets()) {
        tw->setDisplayMode(modes[index]);
    }
}

/**
 * @brief 时间戳显示开关
 * @param checked true=在每行终端数据前显示时间戳
 */
void TerminalController::onTimestampToggled(bool checked)
{
    if (!m_layoutManager) return;

    // 时间戳开关: 同步到所有活动的终端widget
    for (auto* tw : m_layoutManager->terminalWidgets()) {
        tw->setShowTimestamp(checked);
    }
}

/**
 * @brief 收发方向前缀开关
 * @param checked true=显示 [TX]/[RX] 方向前缀
 */
void TerminalController::onDirPrefixToggled(bool checked)
{
    if (!m_layoutManager) return;

    // 方向前缀开关: 同步到所有活动的终端widget
    for (auto* tw : m_layoutManager->terminalWidgets()) {
        tw->setShowDirectionPrefix(checked);
    }
}

/** @brief 清空终端内容、重置数据统计面板、刷新状态栏字节数显示 */
void TerminalController::onClearTerminal()
{
    m_terminalModel->clear();
    if (m_dataStats) m_dataStats->reset();
    updateStatusBar();
}

/**
 * @brief 终端搜索请求处理
 * @param pattern 搜索模式字符串
 * @param regex true=使用正则表达式匹配
 * @param hex true=按 HEX 字节搜索
 */
void TerminalController::onSearchRequested(const QString& pattern, bool regex, bool hex)
{
    if (!m_mainTerminal) return;
    // 搜索只作用于主终端（混合模式终端或分栏模式的RX终端）
    auto* terminal = qobject_cast<TerminalWidget*>(m_mainTerminal);
    if (terminal) {
        terminal->setSearchHighlight(pattern, regex, hex);
    }
}

/** @brief 清除主终端的搜索高亮 */
void TerminalController::onSearchCleared()
{
    if (!m_mainTerminal) return;
    auto* terminal = qobject_cast<TerminalWidget*>(m_mainTerminal);
    if (terminal) {
        terminal->clearSearchHighlight();
    }
}

/**
 * @brief 终端布局模式切换
 * @param index 下拉框索引: 0=混合, 1=左右分栏, 2=上下分栏
 */
void TerminalController::onTerminalLayoutChanged(int index)
{
    if (!m_layoutManager) return;
    m_layoutManager->setLayout(index);
}

/**
 * @brief 刷新状态栏中的 RX/TX 字节数显示
 * 自动格式化为 B/KB/MB 单位
 */
void TerminalController::updateStatusBar()
{
    if (!m_terminalModel) return;
    if (!m_rxBytesLbl || !m_txBytesLbl) return;

    auto rx = m_terminalModel->rxBytes();
    auto tx = m_terminalModel->txBytes();
    // 使用公共 ByteFormat::formatSize 替代本地重复实现
    m_rxBytesLbl->setText(tr("接收: ") + ByteFormat::formatSize(rx));
    m_txBytesLbl->setText(tr("发送: ") + ByteFormat::formatSize(tx));
}

/**
 * @brief 导出终端数据到文件
 *
 * 支持三种格式: .txt(纯文本), .csv(表格), .bin(原始二进制)
 * 使用批量流式导出，分批从 TerminalModel 拉取数据，避免一次性深拷贝全部行。
 *
 * @param parent 用于定位文件对话框的父窗口
 */
void TerminalController::onExportData(QWidget* parent)
{
    if (m_terminalModel->lineCount() == 0) {
        EdDialog::error(parent, tr("导出"), tr("没有数据可导出"));
        return;
    }

    QString filter = tr("文本文件 (*.txt);;CSV文件 (*.csv);;二进制文件 (*.bin)");
    QString filePath = QFileDialog::getSaveFileName(parent, tr("导出数据"),
                                                     QString(), filter);
    if (filePath.isEmpty()) return;

    // 根据文件扩展名自动选择导出格式
    DataExporter::Format format = DataExporter::Plain;
    if (filePath.endsWith(".csv", Qt::CaseInsensitive))
        format = DataExporter::Csv;
    else if (filePath.endsWith(".bin", Qt::CaseInsensitive))
        format = DataExporter::Bin;

    // 批量流式导出: 通过 lineProvider 回调分批拉取数据，lines() 内部已加锁保证线程安全
    int totalLines = m_terminalModel->lineCount();
    auto lineProvider = [this](int offset, int count) -> QVector<TerminalLine> {
        return m_terminalModel->lines(offset, count);
    };

    if (m_dataExporter->exportStreamed(filePath, format, lineProvider, totalLines)) {
        emit statusMessage(tr("已导出至 %1").arg(filePath), 3000);
    } else {
        EdDialog::error(parent, tr("导出失败"), tr("无法写入文件"));
    }
}

/** @brief 定时刷新数据统计面板（由 m_statsTimer 每 500ms 触发） */
void TerminalController::updateDataStatistics()
{
    if (m_terminalModel && m_dataStats) {
        m_dataStats->update(m_terminalModel->rxBytes(), m_terminalModel->txBytes());
    }
}

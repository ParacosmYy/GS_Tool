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
#include "shared/AppConstants.h"
#include "shared/TimerConstants.h"

#include <QFileDialog>
#include "core/widgets/EdDialog.h"
#include <QLabel>

/** @brief 构造终端控制器，初始化模型/导出器/定时器等成员 @param model 终端数据模型，用于读取RX/TX字节数和行数据 @param exporter 数据导出器，用于将终端数据导出为文件 @param parent 父对象 */
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

/** @brief 注入终端布局管理器引用 @param manager 布局管理器指针 */
void TerminalController::setLayoutManager(TerminalLayoutManager* manager)
{
    m_layoutManager = manager;
}

/** @brief 注入数据统计面板引用 @param stats 数据统计面板指针 */
void TerminalController::setDataStatistics(DataStatistics* stats)
{
    m_dataStats = stats;
}

/** @brief 注入状态栏字节标签引用 @param rxLabel 接收字节计数标签 @param txLabel 发送字节计数标签 */
void TerminalController::setStatusBarLabels(QLabel* rxLabel, QLabel* txLabel)
{
    m_rxBytesLbl = rxLabel;
    m_txBytesLbl = txLabel;
}

/** @brief 注入主终端widget引用(搜索只作用于主终端) @param terminal 主终端控件指针 */
void TerminalController::setMainTerminal(QWidget* terminal)
{
    m_mainTerminal = terminal;
}

/** @brief 启动统计刷新定时器(每500ms触发一次) */
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

/** @brief 清空终端内容、重置数据统计面板、刷新状态栏字节数显示 */
void TerminalController::onClearTerminal()
{
    ++m_totalClears;
    m_terminalModel->clear();
    if (m_dataStats) m_dataStats->reset();
    updateStatusBar();
}

/** @brief 终端布局模式切换 @param index 下拉框索引: 0=混合, 1=左右分栏, 2=上下分栏 */
void TerminalController::onTerminalLayoutChanged(int index)
{
    if (!m_layoutManager) return;
    m_layoutManager->setLayout(index);
}

/** @brief 刷新状态栏中的RX/TX字节数显示，自动格式化为B/KB/MB单位 */
void TerminalController::updateStatusBar()
{
    if (!m_terminalModel) return;
    if (!m_rxBytesLbl || !m_txBytesLbl) return;

    auto rx = m_terminalModel->rxBytes();
    auto tx = m_terminalModel->txBytes();
    // 使用公共 ByteFormat::formatSize 替代本地重复实现
    m_rxBytesLbl->setText(tr("接收: ") + ByteFormat::formatSize(rx));
    m_txBytesLbl->setText(tr("发送: ") + ByteFormat::formatSize(tx));
    ++m_totalScrollToBottom;  ///< 统计: 每次状态栏刷新视为一次滚动到底部操作
}

/** @brief 定时刷新数据统计面板(由m_statsTimer每500ms触发)，同时检测缓冲区溢出 */
void TerminalController::updateDataStatistics()
{
    if (m_terminalModel && m_dataStats) {
        m_dataStats->update(m_terminalModel->rxBytes(), m_terminalModel->txBytes());
        // 检测缓冲区溢出: 行数达到最大限制时视为溢出
        if (m_terminalModel->lineCount() >= m_terminalModel->maxLines()) {
            ++m_totalBufferOverflows;
        }
    }
}

// 显示模式/搜索/导出方法已移至 TerminalControllerDisplay.cpp

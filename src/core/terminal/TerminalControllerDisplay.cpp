/**
 * @file TerminalControllerDisplay.cpp
 * @brief 终端控制器 — 显示模式、搜索与导出方法实现
 *
 * 从 TerminalController.cpp 拆分而来，包含显示模式切换、
 * 时间戳/方向前缀开关、搜索高亮和流式数据导出等方法。
 */

#include "core/terminal/TerminalController.h"
#include "terminal/model/TerminalModel.h"
#include "terminal/widget/TerminalWidget.h"
#include "terminal/layout/TerminalLayoutManager.h"
#include "serial/data/DataStatistics.h"
#include "utils/export/DataExporter.h"
#include "utils/data/ByteFormat.h"

#include <QFileDialog>
#include "core/widgets/EdDialog.h"

/** @brief 终端显示模式切换，同步到所有活动的终端widget @param index 下拉框索引: 0=文本, 1=HEX, 2=混合, 3=十进制 */
void TerminalController::onDisplayModeChanged(int index)
{
    ++m_totalDisplayModeChanges;
    if (!m_layoutManager) return;
    if (index < 0 || index > 3) return;  // 防御: ComboBox index 越界保护

    DisplayMode modes[] = {DisplayMode::Text, DisplayMode::Hex, DisplayMode::Mixed, DisplayMode::Decimal};
    // 显示模式切换: 同步到所有活动的终端widget（主终端+分栏终端）
    for (auto* tw : m_layoutManager->terminalWidgets()) {
        tw->setDisplayMode(modes[index]);
    }
}

/** @brief 时间戳显示开关，同步到所有活动的终端widget @param checked true=在每行终端数据前显示时间戳 */
void TerminalController::onTimestampToggled(bool checked)
{
    if (!m_layoutManager) return;

    // 时间戳开关: 同步到所有活动的终端widget
    for (auto* tw : m_layoutManager->terminalWidgets()) {
        tw->setShowTimestamp(checked);
    }
}

/** @brief 收发方向前缀开关，同步到所有活动的终端widget @param checked true=显示[TX]/[RX]方向前缀 */
void TerminalController::onDirPrefixToggled(bool checked)
{
    if (!m_layoutManager) return;

    // 方向前缀开关: 同步到所有活动的终端widget
    for (auto* tw : m_layoutManager->terminalWidgets()) {
        tw->setShowDirectionPrefix(checked);
    }
}

/** @brief 终端搜索请求处理，搜索只作用于主终端 @param pattern 搜索模式字符串 @param regex true=使用正则表达式匹配 @param hex true=按HEX字节搜索 @param caseSensitive true=区分大小写 @param wholeWord true=全词匹配 */
void TerminalController::onSearchRequested(const QString& pattern, bool regex, bool hex,
                                           bool caseSensitive, bool wholeWord)
{
    ++m_totalSearches;
    if (!m_mainTerminal) return;
    // 搜索只作用于主终端（混合模式终端或分栏模式的RX终端）
    auto* terminal = qobject_cast<TerminalWidget*>(m_mainTerminal);
    if (terminal) {
        terminal->setSearchHighlight(pattern, regex, hex, caseSensitive, wholeWord);
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

/** @brief 导出终端数据到文件(支持txt/csv/bin格式)，使用批量流式导出避免深拷贝 @param parent 用于定位文件对话框的父窗口 */
void TerminalController::onExportData(QWidget* parent)
{
    ++m_totalExports;
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

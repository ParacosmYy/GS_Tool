/**
 * @file ConverterPanelSlots.cpp
 * @brief 数据转换面板 - 操作槽函数与统计重置实现
 *
 * 从 ConverterPanel.cpp 拆分而来，包含格式转换、交换、复制、
 * 历史选择槽函数和统计计数器重置方法。
 */

#include "utils/converter/ConverterPanel.h"
#include "utils/converter/DataConverter.h"

#include <QApplication>
#include <QClipboard>

/**
 * @brief 执行格式转换
 */
void ConverterPanel::onConvert()
{
    QByteArray input = m_inputEdit->toPlainText().toUtf8();
    if (input.isEmpty()) {
        m_outputEdit->clear();
        m_copyBtn->setEnabled(false);
        return;
    }

    auto from = static_cast<DataConverter::Format>(
        m_fromCombo->currentData().toInt());
    auto to = static_cast<DataConverter::Format>(
        m_toCombo->currentData().toInt());

    QByteArray result = m_converter.convert(input, from, to);
    if (result.isEmpty() && !input.isEmpty()) {
        /* 转换失败，输出为空但输入非空 */
        ++m_totalErrors;
    }
    m_outputEdit->setPlainText(QString::fromUtf8(result));
    m_copyBtn->setEnabled(true);
    ++m_totalConversions;

    /* 添加到历史记录 */
    QString fromName = DataConverter::formatName(from);
    QString toName = DataConverter::formatName(to);
    QString preview = m_inputEdit->toPlainText();
    if (preview.length() > 30) {
        preview = preview.left(30) + QStringLiteral("...");
    }
    auto* histItem = new QListWidgetItem(
        tr("%1 → %2: %3").arg(fromName, toName, preview), m_historyList);
    while (m_historyList->count() > 30) {
        delete m_historyList->takeItem(0);
    }
}

/**
 * @brief 交换源/目标格式并重新转换
 */
void ConverterPanel::onSwap()
{
    ++m_totalFormatSwaps;
    int fromIdx = m_fromCombo->currentIndex();
    int toIdx = m_toCombo->currentIndex();

    m_fromCombo->setCurrentIndex(toIdx);
    m_toCombo->setCurrentIndex(fromIdx);

    // 如果有输出内容，用它作为新的输入
    if (!m_outputEdit->toPlainText().isEmpty()) {
        m_inputEdit->setPlainText(m_outputEdit->toPlainText());
        onConvert();
    }
}

/**
 * @brief 复制输出到剪贴板
 */
void ConverterPanel::onCopy()
{
    ++m_totalCopyActions;
    ++m_totalClipboardOps; ///< 统计: 剪贴板操作递增
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(m_outputEdit->toPlainText());
}

/**
 * @brief 清除转换历史列表
 */
void ConverterPanel::onClearHistory()
{
    m_historyList->clear();
}

/**
 * @brief 从历史记录选择恢复输入
 */
void ConverterPanel::onHistorySelected()
{
    QListWidgetItem* item = m_historyList->currentItem();
    if (!item) {
        return;
    }
    ++m_totalHistorySelections;
    /* 历史只记录了摘要，不做恢复操作 */
}

/**
 * @brief 重置所有统计计数器
 */
void ConverterPanel::resetStatistics()
{
    m_totalConversions = 0;
    m_totalCopyActions = 0;
    m_totalFormatSwaps = 0;
    m_totalInputChanges = 0;
    m_totalErrors = 0;
    m_totalFormatSwitches = 0;
    m_totalClipboardOps = 0;
    m_totalPasteActions = 0;
    m_totalHistorySelections = 0;
}

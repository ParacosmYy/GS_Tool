/**
 * @file ChecksumPanelSlots.cpp
 * @brief 校验和面板 - UI槽函数与拖放事件实现
 *
 * 从 ChecksumPanel.cpp 拆分而来，包含计算/复制/清除/历史选择槽函数
 * 和拖放事件处理(dragEnter/dragMove/drop)。
 */

#include "utils/checksum/ChecksumPanel.h"

#include <QApplication>
#include <QClipboard>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>

/** @brief 执行校验和计算 */
void ChecksumPanel::onCalculate()
{
    QByteArray data = inputData();
    if (data.isEmpty()) {
        m_resultLabel->setText(tr("结果：无数据"));
        m_copyBtn->setEnabled(false);
        return;
    }

    ++m_totalCalculations;
    auto alg = selectedAlgorithm();
    m_result = m_calculator.calculate(data, alg);
    QString name = ChecksumCalculator::algorithmName(alg);

    // 根据结果位宽选择合适的显示格式
    QString hexResult;
    if (m_result <= 0xFF) {
        hexResult = QStringLiteral("%1").arg(m_result, 2, 16, QLatin1Char('0'));
    } else if (m_result <= 0xFFFF) {
        hexResult = QStringLiteral("%1").arg(m_result, 4, 16, QLatin1Char('0'));
    } else {
        hexResult = QStringLiteral("%1").arg(m_result, 8, 16, QLatin1Char('0'));
    }
    hexResult = hexResult.toUpper();

    m_resultLabel->setText(tr("结果：0x%1 (%2) [%3]")
                               .arg(hexResult)
                               .arg(m_result)
                               .arg(name));

    m_copyBtn->setEnabled(true);
    emit calculated(m_result, name);

    /* 添加到历史记录 */
    QString inputHex = data.toHex(' ').toUpper();
    if (inputHex.length() > 32) {
        inputHex = inputHex.left(32) + QStringLiteral("...");
    }
    addHistoryEntry(m_result, name, inputHex);
}

/** @brief 复制校验和结果到剪贴板 */
void ChecksumPanel::onCopyResult()
{
    ++m_totalCopyActions;
    ++m_totalCopyToClipboard;
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(QString::number(m_result, 16).toUpper());
}

/** @brief 清除计算历史列表 */
void ChecksumPanel::onClearHistory()
{
    m_historyList->clear();
}

/** @brief 从历史记录中选择并恢复显示结果 */
void ChecksumPanel::onHistoryItemSelected()
{
    QListWidgetItem* item = m_historyList->currentItem();
    if (!item) {
        return;
    }
    /* 从 item data 中恢复结果值 */
    bool ok = false;
    quint64 value = item->data(Qt::UserRole).toULongLong(&ok);
    if (ok) {
        m_result = value;
        m_resultLabel->setText(item->text());
        m_copyBtn->setEnabled(true);
    }
}

/** @brief 拖拽进入事件 — 接受文件拖入 */
void ChecksumPanel::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

/** @brief 拖拽移动事件 */
void ChecksumPanel::dragMoveEvent(QDragMoveEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

/** @brief 放下事件 — 将拖入文件路径填入输入框并切换为文件模式 */
void ChecksumPanel::dropEvent(QDropEvent* event)
{
    const QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty()) {
        return;
    }

    const QString filePath = urls.first().toLocalFile();
    if (filePath.isEmpty()) {
        return;
    }

    /* 切换到文件模式并填入路径 */
    for (int i = 0; i < m_inputModeCombo->count(); ++i) {
        if (m_inputModeCombo->itemData(i).toInt() == 2) {
            m_inputModeCombo->setCurrentIndex(i);
            break;
        }
    }
    m_inputEdit->setPlainText(filePath);
    event->acceptProposedAction();
}

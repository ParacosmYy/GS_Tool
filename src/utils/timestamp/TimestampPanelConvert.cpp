/**
 * @file TimestampPanelConvert.cpp
 * @brief 时间戳面板 - 转换逻辑与操作槽函数实现
 *
 * 从 TimestampPanel.cpp 拆分而来，包含格式转换、当前时间填入、
 * 复制/清除历史和统计重置方法。
 */

#include "utils/timestamp/TimestampPanel.h"

#include <QApplication>
#include <QClipboard>

/** @brief 执行时间戳转换（入口） */
void TimestampPanel::onConvert()
{
    ++m_totalConversions;
    ++m_totalAnalyses;
    QString text = m_timestampEdit->text().trimmed();
    if (text.isEmpty()) {
        m_resultLabel->setText(tr("结果：无输入"));
        m_copyBtn->setEnabled(false);
        return;
    }

    int fmt = m_formatCombo->currentData().toInt();
    convertByFormat(text, fmt);
}

/** @brief 根据格式执行转换 */
void TimestampPanel::convertByFormat(const QString &text, int formatIndex)
{
    QDateTime dt;
    bool inputIsTimestamp = false;
    qint64 secs = 0;
    qint64 millis = 0;

    switch (formatIndex) {
    case 1: {
        bool ok = false;
        secs = text.toLongLong(&ok);
        if (ok) {
            dt = m_analyzer.unixToDatetime(secs, false);
            inputIsTimestamp = true;
        }
        break;
    }
    case 2: {
        bool ok = false;
        millis = text.toLongLong(&ok);
        if (ok) {
            dt = m_analyzer.unixToDatetime(millis, true);
            secs = millis / 1000;
            inputIsTimestamp = true;
        }
        break;
    }
    case 3: {
        dt = QDateTime::fromString(text, Qt::ISODate);
        if (!dt.isValid()) {
            dt = QDateTime::fromString(text,
                QStringLiteral("yyyy-MM-dd HH:mm:ss"));
        }
        break;
    }
    default: {
        dt = m_analyzer.parseTimestamp(text);
        break;
    }
    }

    if (!dt.isValid()) {
        m_resultLabel->setText(tr("结果：无法解析"));
        m_copyBtn->setEnabled(false);
        return;
    }

    ++m_totalTimestampParses;

    if (!inputIsTimestamp) {
        secs = m_analyzer.datetimeToUnix(dt, false);
    }
    millis = secs * 1000;

    QString result = tr(
        "日期时间：%1\n"
        "ISO 格式：%2\n"
        "Unix 秒：%3\n"
        "Unix 毫秒：%4")
        .arg(dt.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")))
        .arg(dt.toString(Qt::ISODate))
        .arg(secs)
        .arg(millis);

    m_resultLabel->setText(result);
    m_copyBtn->setEnabled(true);

    /* 添加到历史记录 */
    auto* histItem = new QListWidgetItem(
        tr("%1 → %2").arg(text, dt.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"))),
        m_historyList);
    histItem->setData(Qt::UserRole, text);
    while (m_historyList->count() > 30) {
        delete m_historyList->takeItem(0);
    }
}

/** @brief 填入当前时间戳 */
void TimestampPanel::onNow()
{
    qint64 now = TimestampAnalyzer::currentUnix(false);
    m_timestampEdit->setText(QString::number(now));
    onConvert();
}

/** @brief 复制结果到剪贴板 */
void TimestampPanel::onCopy()
{
    ++m_totalCopyActions;
    ++m_totalCopies;
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(m_resultLabel->text());
}

/** @brief 清除转换历史列表 */
void TimestampPanel::onClearHistory()
{
    m_historyList->clear();
}

/** @brief 从历史记录选择恢复输入 */
void TimestampPanel::onHistorySelected()
{
    QListWidgetItem* item = m_historyList->currentItem();
    if (!item) {
        return;
    }
    const QString text = item->data(Qt::UserRole).toString();
    m_timestampEdit->setText(text);
    onConvert();
}

/** @brief 重置时间戳面板统计计数器 */
void TimestampPanel::resetTimestampPanelStatistics()
{
    m_totalConversions = 0;
    m_totalCopyActions = 0;
    m_totalAnalyses = 0;
    m_totalFormatChanges = 0;
    m_totalTimestampParses = 0;
    m_totalFormatsSelected = 0;
    m_totalCopies = 0;
}

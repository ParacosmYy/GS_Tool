/**
 * @file ProtocolViewData.cpp
 * @brief 协议视图 - 帧数据管理与错误处理实现
 *
 * 从 ProtocolView.cpp 拆分而来，包含帧添加、错误帧处理、
 * 数据清除和统计重置方法。
 */

#include "protocol/view/ProtocolView.h"
#include "utils/crypto/HexConverter.h"
#include "core/theme/ThemeManager.h"
#include <QDateTime>

/** @brief 添加帧数据到表格(更新列头+着色+保存+超限移除+自动调整列宽) @param fields 解析后的字段映射 */
void ProtocolView::addFrame(const QVariantMap& fields)
{
    updateColumnHeaders(fields);
    int row = m_model->rowCount();
    m_totalFrames++;

    /* 序号 */
    auto* idxItem = new QStandardItem(QString::number(m_totalFrames));
    idxItem->setTextAlignment(Qt::AlignCenter);
    m_model->setItem(row, 0, idxItem);
    /* 时间 */
    auto* timeItem = new QStandardItem(fields.value("_frameTime").toString());
    timeItem->setTextAlignment(Qt::AlignCenter);
    m_model->setItem(row, 1, timeItem);
    /* 字段值(带着色) */
    for (int i = 0; i < m_fieldNames.size(); ++i) {
        QString value = fields.value(m_fieldNames[i]).toString();
        auto* item = createColoredItem(m_fieldNames[i], value);
        item->setTextAlignment(Qt::AlignCenter);
        m_model->setItem(row, kFixedColumns + i, item);
    }

    m_frames.append(fields);
    while (m_model->rowCount() > m_maxRows) {
        m_model->removeRow(0);
        if (!m_frames.isEmpty()) m_frames.removeFirst();
    }
    m_table->scrollToBottom();
    /* 自动调整列宽(每50帧或前3帧) */
    if (m_totalFrames % 50 == 0 || m_totalFrames <= 3) autoResizeColumns();
    m_statusLabel->setText(tr("帧数: %1 | 错误: %2").arg(m_totalFrames).arg(m_totalErrors));
    ++m_totalFramesDisplayed;
}

/** @brief 清除表格所有行并重置帧计数器 */
void ProtocolView::clear()
{
    m_model->removeRows(0, m_model->rowCount());
    m_frames.clear();
    m_totalFrames = 0;
    m_totalErrors = 0;
    m_statusLabel->setText(tr("暂无数据"));
}

/** @brief 帧解析成功回调，添加到表格 @param fields 解析后的字段映射 @param rawFrame 原始帧数据(未使用) */
void ProtocolView::onFrameParsed(const QVariantMap& fields, const QByteArray& rawFrame)
{
    Q_UNUSED(rawFrame);
    addFrame(fields);
}

/** @brief 帧解析错误回调，添加错误行(红色高亮+动态Error/RawData列) @param reason 错误原因描述 @param rawFrame 导致错误的原始帧数据 */
void ProtocolView::onFrameError(const QString& reason, const QByteArray& rawFrame)
{
    m_totalErrors++;
    QVariantMap errorFields;
    errorFields["_frameTime"] = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    errorFields["Error"] = reason;
    errorFields["RawData"] = HexConverter::toHexString(rawFrame);

    /* 动态添加Error/RawData列 */
    if (!m_fieldNames.contains("Error")) {
        m_fieldNames.append("Error");
        m_model->setHorizontalHeaderItem(kFixedColumns + m_fieldNames.indexOf("Error"),
                                          new QStandardItem(tr("错误")));
    }
    if (!m_fieldNames.contains("RawData")) {
        m_fieldNames.append("RawData");
        m_model->setHorizontalHeaderItem(kFixedColumns + m_fieldNames.indexOf("RawData"),
                                          new QStandardItem(tr("原始数据")));
    }

    int row = m_model->rowCount();
    m_totalFrames++;
    auto* idxItem = new QStandardItem(QString::number(m_totalFrames));
    idxItem->setTextAlignment(Qt::AlignCenter);
    m_model->setItem(row, 0, idxItem);
    auto* timeItem = new QStandardItem(errorFields["_frameTime"].toString());
    timeItem->setTextAlignment(Qt::AlignCenter);
    m_model->setItem(row, 1, timeItem);

    /* 整行标红 */
    QColor errorColor = ThemeManager::instance().color(ThemeManager::SemanticColor::Error);
    for (int i = 0; i < m_fieldNames.size(); ++i) {
        QString value = errorFields.value(m_fieldNames[i]).toString();
        auto* item = new QStandardItem(value);
        item->setTextAlignment(Qt::AlignCenter);
        item->setForeground(errorColor);
        m_model->setItem(row, kFixedColumns + i, item);
    }
    m_frames.append(errorFields);
    while (m_model->rowCount() > m_maxRows) {
        m_model->removeRow(0);
        if (!m_frames.isEmpty()) m_frames.removeFirst();
    }
    m_table->scrollToBottom();
    m_statusLabel->setText(tr("帧数: %1 | 错误: %2").arg(m_totalFrames).arg(m_totalErrors));
}

/** @brief 重置协议视图统计计数器 */
void ProtocolView::resetViewStatistics()
{
    m_totalFramesDisplayed = 0;
    m_totalExports = 0;
    m_totalContextMenuActions = 0;
}

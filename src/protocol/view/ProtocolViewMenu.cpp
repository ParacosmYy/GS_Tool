/**
 * @file ProtocolViewMenu.cpp
 * @brief ProtocolView 右键菜单和导出功能
 *
 * 从 ProtocolView.cpp 拆分而来，包含右键菜单构建与回调、
 * 复制行/原始数据、CSV/JSON导出。
 */

#include "protocol/view/ProtocolView.h"
#include "utils/crypto/HexConverter.h"
#include "core/theme/ThemeManager.h"
#include "core/widgets/EdDialog.h"

#include <QFileDialog>
#include <QFile>
#include <QTextStream>

// ============================================================
// 右键菜单
// ============================================================

/** @brief 右键菜单弹出回调，根据选中状态启用/禁用菜单项 @param pos 点击位置 */
void ProtocolView::onCustomContextMenu(const QPoint& pos)
{
    if (!m_table->selectionModel()) return;
    bool hasSelection = m_table->selectionModel()->hasSelection();
    m_copyRowAction->setEnabled(hasSelection);
    m_copyRawAction->setEnabled(hasSelection);
    m_exportJsonAction->setEnabled(!m_frames.isEmpty());
    ++m_totalContextMenuActions;
    m_contextMenu->popup(m_table->viewport()->mapToGlobal(pos));
}

/** @brief 复制选中行文本到剪贴板(制表符分隔) */
void ProtocolView::copyRow()
{
    if (!m_table->selectionModel()) return;
    QModelIndexList selected = m_table->selectionModel()->selectedRows();
    if (selected.isEmpty()) return;
    int row = selected.first().row();
    if (row < 0 || row >= m_frames.size()) return;
    QStringList cols;
    cols << m_model->data(m_model->index(row, 0)).toString();
    cols << m_model->data(m_model->index(row, 1)).toString();
    for (int i = 0; i < m_fieldNames.size(); ++i)
        cols << m_model->data(m_model->index(row, kFixedColumns + i)).toString();
    QApplication::clipboard()->setText(cols.join("\t"));
    m_statusLabel->setText(tr("已复制行"));
}

/** @brief 复制选中帧的原始HEX数据到剪贴板 */
void ProtocolView::copyRaw()
{
    if (!m_table->selectionModel()) return;
    QModelIndexList selected = m_table->selectionModel()->selectedRows();
    if (selected.isEmpty()) return;
    int row = selected.first().row();
    if (row < 0 || row >= m_frames.size()) return;
    const QVariantMap& frame = m_frames[row];
    QString rawData = frame.value("RawData").toString();
    if (rawData.isEmpty()) {
        QStringList parts;
        for (const auto& name : m_fieldNames) parts << frame.value(name).toString();
        rawData = parts.join(" ");
    }
    QApplication::clipboard()->setText(rawData);
    m_statusLabel->setText(tr("已复制原始数据"));
}

/** @brief 导出所有帧为JSON文件(含frames数组+导出时间+统计) */
void ProtocolView::exportJson()
{
    if (m_frames.isEmpty()) {
        EdDialog::error(this, tr("导出JSON"), tr("无数据可导出"));
        return;
    }
    QString filePath = QFileDialog::getSaveFileName(
        this, tr("导出JSON"), QString(), tr("JSON 文件 (*.json)"));
    if (filePath.isEmpty()) return;

    QJsonObject root;
    QJsonArray framesArray;
    for (int i = 0; i < m_frames.size(); ++i) {
        const auto& frame = m_frames[i];
        QJsonObject frameObj;
        frameObj["#"] = i + 1;
        frameObj["Time"] = frame.value("_frameTime").toString();
        for (const auto& name : m_fieldNames) {
            QString value = frame.value(name).toString();
            bool ok = false;
            double numVal = value.toDouble(&ok);
            if (ok) frameObj[name] = numVal; else frameObj[name] = value;
        }
        framesArray.append(frameObj);
    }
    root["frames"] = framesArray;
    root["exportTime"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    root["totalFrames"] = static_cast<qint64>(m_totalFrames);
    root["totalErrors"] = static_cast<qint64>(m_totalErrors);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        EdDialog::error(this, tr("导出JSON"), tr("无法写入文件"));
        return;
    }
    QByteArray json = QJsonDocument(root).toJson(QJsonDocument::Indented);
    if (file.write(json) != json.size()) {
        EdDialog::error(this, tr("导出JSON"), tr("写入文件失败，磁盘可能已满"));
        return;
    }
    file.close();
    m_statusLabel->setText(tr("已导出 %1 帧到JSON").arg(m_frames.size()));
    ++m_totalExports;
}

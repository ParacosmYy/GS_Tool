/**
 * @file OtaWidgetDragDrop.cpp
 * @brief OTA升级面板 - 拖放事件处理实现
 *
 * 从 OtaWidget.cpp 拆分而来，包含:
 *   - dragEnterEvent: 检查MIME类型(.bin/.hex/.ihex/.fw文件)
 *   - dragMoveEvent: 持续接受有效拖放
 *   - dropEvent: 提取文件路径并加载
 *   - dragLeaveEvent: 恢复拖放提示标签
 *   - handleDroppedFile: 处理拖入的固件文件
 *
 * 构造/UI/内部方法见 OtaWidget.cpp。
 */

#include "ota/widget/OtaWidget.h"

#include <QMimeData>
#include <QUrl>
#include <QFileInfo>

#include "utils/data/ByteFormat.h"

/** @brief 拖入事件: 检查MIME类型是否包含文件URL，且文件后缀为.bin/.hex @param event 拖入事件 */
void OtaWidget::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        const QList<QUrl> urls = event->mimeData()->urls();
        if (!urls.isEmpty()) {
            QString suffix = QFileInfo(urls.first().toLocalFile()).suffix().toLower();
            if (suffix == "bin" || suffix == "hex" || suffix == "ihex" || suffix == "fw") {
                event->acceptProposedAction();
                m_dragHovering = true;
                // 视觉反馈: 显示拖放提示标签高亮
                if (m_dropHintLbl) {
                    m_dropHintLbl->setText(tr("释放以选择此固件文件"));
                }
                return;
            }
        }
    }
    event->ignore();
}

/** @brief 拖动事件: 持续接受有效拖放 @param event 拖动事件 */
void OtaWidget::dragMoveEvent(QDragMoveEvent* event)
{
    if (m_dragHovering) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

/** @brief 放下事件: 提取第一个文件路径并加载 @param event 放下事件 */
void OtaWidget::dropEvent(QDropEvent* event)
{
    m_dragHovering = false;
    if (m_dropHintLbl) {
        m_dropHintLbl->setText(tr("拖放固件文件到此处"));
    }

    const QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty()) return;

    QString filePath = urls.first().toLocalFile();
    if (!filePath.isEmpty()) {
        handleDroppedFile(filePath);
    }
    event->acceptProposedAction();
}

/** @brief 拖离事件: 恢复拖放提示标签文字 @param event 拖离事件 */
void OtaWidget::dragLeaveEvent(QDragLeaveEvent* event)
{
    Q_UNUSED(event)
    m_dragHovering = false;
    if (m_dropHintLbl) {
        m_dropHintLbl->setText(tr("拖放固件文件到此处"));
    }
}

/** @brief 处理拖入的固件文件: 更新路径输入框、文件信息标签并记录日志 @param filePath 拖入的文件路径 */
void OtaWidget::handleDroppedFile(const QString& filePath)
{
    // 传输中不允许切换文件
    if (m_manager->isTransferring()) {
        appendLog(tr("警告: 传输进行中，无法更换文件"));
        return;
    }

    m_filePathEdit->setText(filePath);
    QFileInfo info(filePath);
    QString sizeStr = ByteFormat::formatSize(info.size());
    m_fileInfoLbl->setText(tr("类型: %1 | 大小: %2").arg(info.suffix().toUpper(), sizeStr));
    appendLog(tr("已拖入文件: %1 (%2)").arg(filePath, sizeStr));
}

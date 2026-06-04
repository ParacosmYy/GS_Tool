/**
 * @file OtaWidgetSlots2.cpp
 * @brief OTA升级面板 -- 用户交互槽函数实现(文件浏览/开始传输/取消传输)
 *
 * 本文件从 OtaWidget.cpp 拆分而来，包含用户主动触发的槽函数:
 *   - onBrowseFile:     文件浏览按钮回调，打开文件对话框选择固件
 *   - onStartTransfer:  开始传输按钮回调，校验文件后启动OTA传输
 *   - onCancelTransfer: 取消传输按钮回调，中止当前OTA传输
 */

#include "ota/widget/OtaWidget.h"
#include "core/widgets/AnimatedButton.h"
#include "shared/LayoutConstants.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QDateTime>

#include "utils/data/ByteFormat.h"

// ============================================================================
// 槽函数 -- 文件浏览 / 传输控制
// ============================================================================

/** @brief 浏览文件按钮回调: 打开文件对话框选择固件文件(.bin/.hex/.fw) */
void OtaWidget::onBrowseFile()
{
    ++m_stats.browseClicks;  ///< 统计: 浏览按钮点击
    QString filter = tr("固件文件 (*.bin *.hex);;二进制文件 (*.bin);;Intel HEX (*.hex);;所有文件 (*.*)");
    QString path = QFileDialog::getOpenFileName(this, tr("选择固件文件"), QString(), filter);
    if (path.isEmpty()) return;
    m_filePathEdit->setText(path);
    QFileInfo info(path);
    QString sizeStr = ByteFormat::formatSize(info.size());
    m_fileInfoLbl->setText(tr("类型: %1 | 大小: %2").arg(info.suffix().toUpper(), sizeStr));
    appendLog(tr("已选择文件: %1 (%2)").arg(path, sizeStr));
}

/** @brief 开始传输按钮回调: 校验文件路径后调用OtaManager启动传输 */
void OtaWidget::onStartTransfer()
{
    QString filePath = m_filePathEdit->text().trimmed();
    if (filePath.isEmpty()) { appendLog(tr("错误: 未选择固件文件")); return; }

    QString errorMsg;
    if (!m_manager->validateFilePath(filePath, errorMsg)) {
        appendLog(tr("文件验证失败: %1").arg(errorMsg)); return;
    }

    QFileInfo fileInfo(filePath);
    m_currentFileName = fileInfo.fileName();
    m_currentProtocol = m_protocolCombo->currentData().toString();
    m_currentFileSize = fileInfo.size();
    m_transferStartTime = QDateTime::currentDateTime();
    QString protocol = m_protocolCombo->currentData().toString();

    auto fwType = m_manager->detectFirmwareType(filePath);
    QString typeStr = (fwType == OtaManager::FirmwareType::IntelHex) ? tr("HEX->BIN") : tr("BIN");
    appendLog(tr("开始传输: %1 [%2, %3], 协议: %4")
        .arg(m_currentFileName, typeStr, ByteFormat::formatSize(m_currentFileSize), protocol));

    m_transferTimer.start();
    m_lastBytesSent = 0;
    setTransferring(true);
    ++m_stats.transfersStarted;  ///< 统计: 传输启动

    // 清除上次校验和显示
    m_checksumLbl->setText("");

    // 发射传输开始信号，供Toast通知使用
    emit transferStarted(m_currentFileName);

    if (!m_manager->startTransfer(filePath, protocol)) {
        setTransferring(false);
        appendLog(tr("传输启动失败"));
        ++m_stats.transfersFailed;  ///< 统计: 传输启动失败
        // 启动失败也通知Toast
        emit transferFailed(m_currentFileName, tr("传输启动失败"));
    }
}

/** @brief 取消传输按钮回调: 中止当前OTA传输并恢复UI状态 */
void OtaWidget::onCancelTransfer()
{
    ++m_stats.cancelOps;  ///< 统计: 取消操作
    m_manager->cancelTransfer();
    appendLog(tr("用户已取消传输"));
    setTransferring(false);
}

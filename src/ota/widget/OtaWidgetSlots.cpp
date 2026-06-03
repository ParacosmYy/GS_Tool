/**
 * @file OtaWidgetSlots.cpp
 * @brief OTA升级面板 -- 槽函数实现（进度/速率/状态/完成/错误）
 *
 * 本文件从 OtaWidget.cpp 拆分而来，包含所有响应 OtaManager 信号的槽函数:
 *   - onProgress:         进度条平滑动画填充
 *   - onTransferStats:    速率和ETA实时更新
 *   - onOtaStateChanged:  OTA状态变化日志记录
 *   - onTransferComplete: 传输完成处理（变色动画 + 历史记录）
 *   - onTransferError:    传输错误处理（状态重置 + 失败历史）
 *   - startCompletionAnimation: 进度条完成变色动画（accent -> success）
 */

#include "ota/widget/OtaWidget.h"
#include "core/theme/Constants.h"

#include "core/theme/ThemeManager.h"
#include "utils/data/ByteFormat.h"

// ============================================================================
// 槽函数 -- 进度 / 速率 / 状态
// ============================================================================

/** @brief 进度更新 -- 使用 QPropertyAnimation 实现平滑填充，避免跳变 */
void OtaWidget::onProgress(int percent, qint64 bytesSent, qint64 totalBytes)
{
    Q_UNUSED(bytesSent)
    Q_UNUSED(totalBytes)

    if (m_progressAnim && m_progressAnim->state() == QAbstractAnimation::Running) m_progressAnim->stop();
    // stop()触发DeleteWhenStopped自动deleteLater，断开旧动画的destroyed信号避免nullify新动画
    m_progressAnim = nullptr;

    int oldValue = m_progressBar->value();
    if (oldValue != percent) {
        m_progressAnim = new QPropertyAnimation(m_progressBar, "value");
        m_progressAnim->setStartValue(oldValue);
        m_progressAnim->setEndValue(percent);
        m_progressAnim->setDuration(qMin(qAbs(percent - oldValue) * 10, Timers::kProgressAnimMaxMs));
        m_progressAnim->setEasingCurve(QEasingCurve::OutCubic);
        m_progressAnim->start(QAbstractAnimation::DeleteWhenStopped);
    }
    m_statusLbl->setText(tr("传输中: %1%").arg(percent));
}

/** @brief 速率和ETA更新 -- 由 OtaManager::transferStats 信号驱动 */
void OtaWidget::onTransferStats(double rateBytesPerSec, double etaSec)
{
    qint64 rate = static_cast<qint64>(rateBytesPerSec);
    m_speedLbl->setText(ByteFormat::formatRate(rate, 1.0));

    if (etaSec > 0) {
        qint64 etaMs = static_cast<qint64>(etaSec * 1000.0);
        m_etaLbl->setText(tr("预计剩余: %1").arg(ByteFormat::formatDuration(etaMs)));
    } else {
        m_etaLbl->setText(tr("预计剩余: --"));
    }
}

/** @brief OTA状态变化 -- 记录关键状态到日志 */
void OtaWidget::onOtaStateChanged(OtaManager::OtaState state)
{
    switch (state) {
    case OtaManager::OtaState::Selecting:  appendLog(tr("正在验证固件文件...")); break;
    case OtaManager::OtaState::Verifying:  appendLog(tr("正在校验传输数据...")); break;
    case OtaManager::OtaState::Complete:   m_statusLbl->setText(tr("传输完成")); break;
    default: break;
    }
}

// ============================================================================
// 槽函数 -- 传输完成 / 错误
// ============================================================================

/** @brief 传输完成 -- 进度条100% + accent->success变色动画 + 记录成功历史 */
void OtaWidget::onTransferComplete()
{
    setTransferring(false);
    ++m_totalTransfersCompleted;  ///< 统计: 传输完成
    m_totalBytesTransferred += static_cast<quint64>(m_currentFileSize);  ///< 统计: 累计字节
    m_progressBar->setValue(100);
    m_statusLbl->setText(tr("传输完成"));
    m_speedLbl->setText("");
    m_etaLbl->setText("");

    qint64 elapsed = m_transferTimer.elapsed();
    appendLog(tr("传输完成，耗时 %1").arg(ByteFormat::formatDuration(elapsed)));
    startCompletionAnimation();

    // 发射传输完成信号，供Toast通知使用
    emit transferCompleted(m_currentFileName, elapsed, m_currentFileSize);

    // 记录成功传输到历史模型
    OtaRecord rec;
    rec.fileName = m_currentFileName;
    rec.protocol = m_currentProtocol;
    rec.fileSize = m_currentFileSize;
    rec.startTime = m_transferStartTime;
    rec.durationMs = elapsed;
    rec.success = true;
    m_historyModel->addRecord(rec);
}

/** @brief 传输错误 -- 重置UI状态 + 记录失败历史 */
void OtaWidget::onTransferError(const QString& reason)
{
    setTransferring(false);
    ++m_totalTransfersFailed;  ///< 统计: 传输失败
    m_statusLbl->setText(tr("错误: %1").arg(reason));
    appendLog(tr("错误: %1").arg(reason));

    // 发射传输失败信号，供Toast通知使用
    emit transferFailed(m_currentFileName, reason);

    // 记录失败传输到历史模型
    OtaRecord rec;
    rec.fileName = m_currentFileName;
    rec.protocol = m_currentProtocol;
    rec.fileSize = m_currentFileSize;
    rec.startTime = m_transferStartTime;
    rec.durationMs = m_transferTimer.elapsed();
    rec.success = false;
    rec.errorMessage = reason;
    m_historyModel->addRecord(rec);
}

// ============================================================================
// 内部方法 -- 进度条完成变色动画
// ============================================================================

/**
 * @brief 启动进度条完成变色动画（accent -> success，400ms OutCubic）
 *
 * 使用 QPropertyAnimation 对 AnimatedProgressBar 的 chunkColor 属性做颜色插值，
 * 从 ThemeManager 获取 accent 和 success 语义色，实现平滑渐变过渡。
 * 符合 CLAUDE.md §6.5 规范: 400ms OutCubic 缓动曲线。
 */
void OtaWidget::startCompletionAnimation()
{
    auto& theme = ThemeManager::instance();
    QColor accent = theme.color(ThemeManager::SemanticColor::Accent);
    QColor success = theme.color(ThemeManager::SemanticColor::Success);

    // 停止并清理旧的变色动画（防止重复触发）
    if (m_colorAnim && m_colorAnim->state() == QAbstractAnimation::Running) {
        m_colorAnim->stop();
    }
    // stop()触发DeleteWhenStopped自动deleteLater，无需手动deleteLater
    m_colorAnim = nullptr;

    // 使用 QPropertyAnimation 对 chunkColor 属性进行颜色插值动画
    // Qt 的 QPropertyAnimation 支持对 QColor 类型做逐通道线性插值
    m_colorAnim = new QPropertyAnimation(m_progressBar, "chunkColor");
    m_colorAnim->setStartValue(accent);
    m_colorAnim->setEndValue(success);
    m_colorAnim->setDuration(Timers::kCompletionDelayMs);
    m_colorAnim->setEasingCurve(QEasingCurve::OutCubic);
    m_colorAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

/**
 * @file CanBusMonitorFrame.cpp
 * @brief CAN总线监控面板 — 帧添加实现
 *
 * 本文件拆分自 CanBusMonitor.cpp，包含:
 *   - addFrame: 添加帧到监控表格(含过滤、颜色编码、DBC信号解码)
 *
 * 帧颜色编码与清空操作见 CanBusMonitorColor.cpp。
 * 统计方法见 CanBusMonitorStats.cpp。
 */

#include "connection/can/CanBusMonitor.h"
#include "protocol/can/DbcParser.h"
#include "core/theme/ThemeManager.h"
#include <QTime>
#include <QBrush>
#include <QColor>

/** @brief 添加一帧CAN数据到监控表格，带颜色编码和自动滚动 @param frame CAN帧数据结构 */
void CanBusMonitor::addFrame(const CanFrame& frame)
{
    /* 帧类型过滤 */
    const int typeFilter = m_typeFilterCombo->currentData().toInt();
    if (typeFilter != 0) {
        bool match = false;
        switch (typeFilter) {
        case 1: match = !frame.extended && !frame.rtr && !frame.fd; break;
        case 2: match = frame.extended; break;
        case 3: match = frame.rtr; break;
        case 4: match = frame.fd; break;
        default: break;
        }
        if (!match) return;
    }

    /* 帧ID文本过滤 */
    if (!m_frameIdFilter.isEmpty()) {
        const QString idStr = frame.extended
            ? QStringLiteral("%1").arg(frame.id, 8, 16, QLatin1Char('0')).toUpper()
            : QStringLiteral("%1").arg(frame.id, 3, 16, QLatin1Char('0')).toUpper();
        if (!idStr.contains(m_frameIdFilter, Qt::CaseInsensitive)) {
            return;
        }
    }

    /* 超过上限时移除最旧行 */
    if (m_frameTable->rowCount() >= kMaxRows) {
        m_frameTable->removeRow(0);
    }

    const int row = m_frameTable->rowCount();
    m_frameTable->insertRow(row);
    ++m_frameCount;
    ++m_totalFramesMonitored;
    ++m_rateFrameCount;
    m_idFrequency[frame.id]++;

    /* 更新帧类型统计 */
    if (frame.extended) {
        ++m_totalExtendedFrames;
    } else {
        ++m_totalStandardFrames;
    }
    if (frame.rtr) {
        ++m_totalRtrFrames;
    }
    if (frame.fd) {
        ++m_totalFdFrames;
    }
    if (frame.error) {
        ++m_totalErrors;
    }
    m_totalBytesReceived += static_cast<quint64>(frame.data.size());

    /* 时间 */
    auto* timeItem = new QTableWidgetItem(
        QTime::currentTime().toString("HH:mm:ss.zzz"));
    m_frameTable->setItem(row, 0, timeItem);

    /* 帧ID */
    QString idStr = frame.extended
        ? QStringLiteral("0x%1").arg(frame.id, 8, 16, QLatin1Char('0')).toUpper()
        : QStringLiteral("0x%1").arg(frame.id, 3, 16, QLatin1Char('0')).toUpper();

    /* 追加DBC消息名(如果可用) */
    if (m_dbcParser) {
        DbcMessage msg = m_dbcParser->messageById(frame.id);
        if (!msg.name.isEmpty()) {
            idStr += QStringLiteral(" (%1)").arg(msg.name);
        }
    }
    m_frameTable->setItem(row, 1, new QTableWidgetItem(idStr));

    /* DLC */
    m_frameTable->setItem(row, 2, new QTableWidgetItem(QString::number(frame.dlc)));

    /* 数据 */
    m_frameTable->setItem(row, 3,
        new QTableWidgetItem(QString::fromUtf8(frame.data.toHex(' ').toUpper())));

    /* 扩展帧标志 */
    m_frameTable->setItem(row, 4,
        new QTableWidgetItem(frame.extended ? tr("是") : tr("否")));

    /* RTR标志 */
    m_frameTable->setItem(row, 5,
        new QTableWidgetItem(frame.rtr ? tr("是") : tr("否")));

    /* 累计计数 */
    m_frameTable->setItem(row, 6,
        new QTableWidgetItem(QString::number(m_idFrequency[frame.id])));

    /* 颜色编码 */
    QBrush bg = rowBrush(frame);
    for (int col = 0; col < 7; ++col) {
        if (m_frameTable->item(row, col)) {
            m_frameTable->item(row, col)->setBackground(bg);
        }
    }

    /* 自动滚动 */
    if (m_autoScrollCheck->isChecked()) {
        m_frameTable->scrollToBottom();
    }

    /* 更新DBC信号解码面板 */
    if (m_dbcParser && !frame.rtr) {
        QMap<QString, double> decodedSignals = m_dbcParser->decodeFrame(frame.id, frame.data);
        if (!decodedSignals.isEmpty()) {
            m_signalTable->setRowCount(static_cast<int>(decodedSignals.size()));
            int sigRow = 0;
            for (auto it = decodedSignals.constBegin(); it != decodedSignals.constEnd(); ++it) {
                m_signalTable->setItem(sigRow, 0,
                    new QTableWidgetItem(it.key()));

                /* 格式化物理值 */
                DbcMessage msg = m_dbcParser->messageById(frame.id);
                QString unitStr;
                QString descStr;
                for (const DbcSignal& sig : msg.signalList) {
                    if (sig.name == it.key()) {
                        unitStr = sig.unit;
                        /* 值表翻译 */
                        QString formatted = m_dbcParser->formatSignalValue(
                            frame.id, it.key(), it.value());
                        if (formatted != QString::number(it.value(), 'f', 2)
                            && formatted != QString::number(static_cast<int>(it.value()))) {
                            descStr = formatted;
                        }
                        break;
                    }
                }

                m_signalTable->setItem(sigRow, 1,
                    new QTableWidgetItem(QString::number(it.value(), 'f', 4)));
                m_signalTable->setItem(sigRow, 2,
                    new QTableWidgetItem(unitStr));
                m_signalTable->setItem(sigRow, 3,
                    new QTableWidgetItem(descStr));
                ++sigRow;
            }
        }
    }

    /* 更新计数标签 */
    m_countLabel->setText(tr("帧数: %1 | ID数: %2")
        .arg(m_frameCount).arg(m_idFrequency.size()));
    updateStatsDisplay();
}

// clearFrames/rowBrush见 CanBusMonitorColor.cpp
// statisticsSummary/frameRate/updateStatsDisplay/resetStatistics见 CanBusMonitorStats.cpp

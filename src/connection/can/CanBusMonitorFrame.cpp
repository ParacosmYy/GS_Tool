/**
 * @file CanBusMonitorFrame.cpp
 * @brief CAN总线监控面板 — 帧处理/显示/统计方法实现
 *
 * 本文件拆分自 CanBusMonitor.cpp，包含:
 *   - addFrame: 添加帧到监控表格(含过滤、颜色编码、DBC信号解码)
 *   - clearFrames: 清空帧记录
 *   - rowBrush: 根据帧类型获取背景色
 *   - statisticsSummary: 统计摘要文本
 *   - frameRate: 帧率计算
 *   - updateStatsDisplay: 统计面板更新
 *   - resetStatistics: 重置统计计数器
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
        ++m_totalErrors;
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

/** @brief 清空所有帧记录并重置帧计数器和频率统计 */
void CanBusMonitor::clearFrames()
{
    m_frameTable->setRowCount(0);
    m_signalTable->setRowCount(0);
    m_frameCount = 0;
    m_rateFrameCount = 0;
    m_idFrequency.clear();
    m_rateTimer.restart();
    m_countLabel->setText(tr("帧数: 0"));
    updateStatsDisplay();
}

/** @brief 根据帧类型获取行背景色(FD浅绿/RTR黄色/扩展帧浅蓝/错误帧红色/标准帧白色) @param frame CAN帧 @return 背景QBrush */
QBrush CanBusMonitor::rowBrush(const CanFrame& frame) const
{
    if (frame.error) {
        /* 错误帧 — 使用ThemeManager Error色 */
        QColor errColor = ThemeManager::instance().color(
            ThemeManager::SemanticColor::Error);
        return QBrush(errColor.lighter(160));
    }
    if (frame.fd) {
        /* CAN-FD帧 — 使用ThemeManager Success色(浅绿) */
        QColor fdColor = ThemeManager::instance().color(
            ThemeManager::SemanticColor::Success);
        return QBrush(fdColor.lighter(160));
    }
    if (frame.rtr) {
        /* RTR帧 — 使用调色板Midlight作为警告色 */
        return QBrush(palette().color(QPalette::Midlight));
    }
    if (frame.extended) {
        /* 扩展帧 — 使用调色板AlternateBase作为信息色 */
        return QBrush(palette().color(QPalette::AlternateBase));
    }
    /* 标准帧 — 使用调色板的Base色(跟随主题) */
    return QBrush(palette().color(QPalette::Base));
}

/** @brief 获取统计摘要文本 @return 格式化的CAN帧统计信息 */
QString CanBusMonitor::statisticsSummary() const
{
    QString summary;
    summary += tr("总帧数: %1\n").arg(m_totalFramesMonitored);
    summary += tr("标准帧: %1 | 扩展帧: %2\n")
        .arg(m_totalStandardFrames).arg(m_totalExtendedFrames);
    summary += tr("CAN-FD帧: %1 | RTR帧: %2\n")
        .arg(m_totalFdFrames).arg(m_totalRtrFrames);
    summary += tr("总字节: %1 | 帧率: %2 fps\n")
        .arg(m_totalBytesReceived).arg(frameRate(), 0, 'f', 1);
    summary += tr("不同帧ID: %1\n").arg(m_idFrequency.size());

    if (!m_idFrequency.isEmpty()) {
        quint32 topId = 0;
        int topCount = 0;
        for (auto it = m_idFrequency.constBegin();
             it != m_idFrequency.constEnd(); ++it) {
            if (it.value() > topCount) {
                topId = it.key();
                topCount = it.value();
            }
        }
        summary += tr("最频繁帧ID: 0x%1 (%2次)")
                      .arg(topId, 0, 16).arg(topCount);
    }

    return summary.trimmed();
}

/** @brief 计算帧率(fps) @return 每秒帧数 */
double CanBusMonitor::frameRate() const
{
    const qint64 elapsed = m_rateTimer.elapsed();
    if (elapsed <= 0) return 0.0;
    return (static_cast<double>(m_rateFrameCount) * 1000.0)
           / static_cast<double>(elapsed);
}

/** @brief 更新统计面板标签文本 */
void CanBusMonitor::updateStatsDisplay()
{
    m_statsLabel->setText(
        tr("STD: %1 | EXT: %2 | FD: %3 | RTR: %4 | ERR: %5 | Bytes: %6 | %7 fps")
            .arg(m_totalStandardFrames)
            .arg(m_totalExtendedFrames)
            .arg(m_totalFdFrames)
            .arg(m_totalRtrFrames)
            .arg(m_totalErrors)
            .arg(m_totalBytesReceived)
            .arg(frameRate(), 0, 'f', 1));
}

/** @brief 重置所有统计计数器 */
void CanBusMonitor::resetStatistics()
{
    m_totalFramesMonitored = 0;
    m_totalErrors = 0;
    m_totalStandardFrames = 0;
    m_totalExtendedFrames = 0;
    m_totalFdFrames = 0;
    m_totalRtrFrames = 0;
    m_totalBytesReceived = 0;
    m_rateFrameCount = 0;
    m_rateTimer.restart();
    updateStatsDisplay();
}

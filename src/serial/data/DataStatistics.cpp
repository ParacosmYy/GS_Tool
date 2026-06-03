/**
 * @file DataStatistics.cpp
 * @brief 数据统计面板实现 — 实时显示收发字节/速率/包计数/平均速率
 *
 * 提供RX/TX字节数、实时速率、平均速率、包计数和连接运行时间的
 * 实时统计显示。使用ByteFormat进行字节格式化。
 */
#include "serial/data/DataStatistics.h"
#include "core/theme/Constants.h"
#include "utils/data/ByteFormat.h"

#include <QFormLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QLatin1Char>

/** @brief 构造函数，初始化UI和1秒刷新定时器 @param parent 父控件 */
DataStatistics::DataStatistics(QWidget* parent)
    : QWidget(parent)
{
    setupUI();

    // 启动1秒定时器，用于刷新速率显示和持续时间
    connect(&m_refreshTimer, &QTimer::timeout,
            this, &DataStatistics::onRefreshTimer);
    m_refreshTimer.setInterval(Timers::kDataStatsRefreshMs);

    // 启动采样间隔计时器（用于计算精确速率）
    m_sampleTimer.start();

    // 启动持续时间计时器
    m_stopwatch.start();
    m_refreshTimer.start();
}

/** @brief 初始化统计面板UI(收发字节/速率/峰值/均值/时间标签) */
void DataStatistics::setupUI()
{
    // 主布局：上下排列，紧凑边距
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(8);

    // ---- 收发统计区域(使用统一框架) ----
    mainLayout->addWidget(createStatsFrame(tr("接收:"), m_rxTotalLabel, "rxTotalLabel", "statsRxFrame"));
    mainLayout->addWidget(createStatsFrame(tr("速率:"), m_rxRateLabel, "rxRateLabel", "statsRxFrame"));
    mainLayout->addWidget(createStatsFrame(tr("发送:"), m_txTotalLabel, "txTotalLabel", "statsTxFrame"));
    mainLayout->addWidget(createStatsFrame(tr("速率:"), m_txRateLabel, "txRateLabel", "statsTxFrame"));
    mainLayout->addWidget(createStatsFrame(tr("峰值:"), m_peakRateLabel, "peakRateLabel", "statsPeakFrame"));
    mainLayout->addWidget(createStatsFrame(tr("均值:"), m_avgRateLabel, "avgRateLabel", "statsAvgFrame"));
    mainLayout->addWidget(createStatsFrame(tr("时间:"), m_elapsedLabel, "elapsedLabel", "statsTimeFrame"));

    // ---- 串口错误计数（默认隐藏，有错误时显示） ----
    m_errorLabel = new QLabel;
    m_errorLabel->setObjectName("errorLabel");
    m_errorLabel->setWordWrap(true);
    m_errorLabel->hide();  // 无错误时不占用空间
    mainLayout->addWidget(m_errorLabel);

    // ---- 连接健康状态（默认隐藏，空闲超10秒时显示） ----
    m_healthLabel = new QLabel;
    m_healthLabel->setObjectName("healthStatusLabel");
    m_healthLabel->setWordWrap(true);
    m_healthLabel->hide();  // 数据正常流动时隐藏
    mainLayout->addWidget(m_healthLabel);

    // 底部弹性空间
    mainLayout->addStretch();
}

/** @brief 创建统计项框架(标签+值标签，用于FormLayout行) @param label 左侧标签文字 @param valueLabel 值标签引用(输出) @param objectName 值标签objectName @param frameName 框架objectName @return QFrame指针 */
QFrame* DataStatistics::createStatsFrame(const QString& label, QLabel*& valueLabel, const QString& objectName, const QString& frameName)
{
    auto* frame = new QFrame;
    frame->setObjectName(frameName.isEmpty() ? QStringLiteral("statsFrame") : frameName);
    frame->setFrameShape(QFrame::StyledPanel);
    auto* layout = new QFormLayout(frame);
    layout->setContentsMargins(8, 6, 8, 6);
    layout->setSpacing(4);
    layout->setLabelAlignment(Qt::AlignRight);

    valueLabel = new QLabel(QStringLiteral("0 B"));
    valueLabel->setObjectName(objectName);
    layout->addRow(label, valueLabel);
    return frame;
}

/** @brief 用最新收发字节总数更新统计(计算增量速率和包计数) @param rxBytes 接收累计字节 @param txBytes 发送累计字节 */
void DataStatistics::update(quint64 rxBytes, quint64 txBytes)
{
    // 计算自上次采样以来的实际时间间隔（毫秒）
    qint64 elapsedMs = m_sampleTimer.elapsed();
    m_sampleTimer.restart();

    // 计算增量字节(防下溢: 计数器重置时归零，避免产生天文数字速率)
    quint64 rxDelta = (rxBytes >= m_lastRxBytes) ? (rxBytes - m_lastRxBytes) : 0;
    quint64 txDelta = (txBytes >= m_lastTxBytes) ? (txBytes - m_lastTxBytes) : 0;

    // 按实际时间间隔换算为 bytes/s，避免调用间隔不等于 1s 时速率失真
    double seconds = qMax(elapsedMs, static_cast<qint64>(1)) / 1000.0;
    m_rxRate = static_cast<double>(rxDelta) / seconds;
    m_txRate = static_cast<double>(txDelta) / seconds;

    // 记录本次采样值，供下次计算增量
    m_lastRxBytes = rxBytes;
    m_lastTxBytes = txBytes;

    // 更新累计字节数显示
    m_rxTotalLabel->setText(ByteFormat::formatSize(rxBytes));
    m_txTotalLabel->setText(ByteFormat::formatSize(txBytes));

    // 累加数据包计数（每次调用视为一次数据包到达）
    if (rxDelta > 0) ++m_rxPackets;
    if (txDelta > 0) ++m_txPackets;

    // 统计计数器递增
    ++m_totalUpdates;  ///< 统计: update()调用次数
    m_totalBytesCounted += static_cast<quint64>(rxDelta) + static_cast<quint64>(txDelta);  ///< 统计: 累计字节数
}

/** @brief 重置所有统计值和UI显示，重启计时器 */
void DataStatistics::reset()
{
    // 重置累计值和速率
    m_lastRxBytes = 0;
    m_lastTxBytes = 0;
    m_rxRate = 0.0;
    m_txRate = 0.0;
    m_peakRxRate = 0.0;
    m_peakTxRate = 0.0;
    m_avgRxRate = 0.0;
    m_avgTxRate = 0.0;
    m_rxPackets = 0;
    m_txPackets = 0;

    // 重置统计计数器
    m_totalUpdates = 0;
    m_totalBytesCounted = 0;
    m_totalPeakUpdates = 0;

    // 重置错误计数
    m_framingErrors = 0;
    m_parityErrors = 0;
    m_overrunErrors = 0;

    // 重置UI显示
    m_rxTotalLabel->setText(tr("0 B"));
    m_txTotalLabel->setText(tr("0 B"));
    m_rxRateLabel->setText(tr("0 B/s"));
    m_txRateLabel->setText(tr("0 B/s"));
    m_peakRateLabel->setText(tr("0 B/s"));
    m_avgRateLabel->setText(tr("0 B/s"));
    m_elapsedLabel->setText(tr("00:00:00"));
    m_errorLabel->hide();
    m_healthLabel->hide();

    // 重启计时器
    m_stopwatch.restart();
}

/** @brief 返回当前接收速率(bytes/s) @return 接收速率 */
double DataStatistics::rxRate() const
{
    return m_rxRate;
}

/** @brief 返回当前发送速率(bytes/s) @return 发送速率 */
double DataStatistics::txRate() const
{
    return m_txRate;
}

/** @brief 1秒定时器回调：刷新持续时间、速率(含衰减)、峰值、均值显示 */
void DataStatistics::onRefreshTimer()
{
    // 更新持续时间显示
    qint64 elapsedSec = m_stopwatch.elapsed() / 1000;
    int hours   = static_cast<int>(elapsedSec / 3600);
    int minutes = static_cast<int>((elapsedSec % 3600) / 60);
    int seconds = static_cast<int>(elapsedSec % 60);
    m_elapsedLabel->setText(
        QString("%1:%2:%3")
            .arg(hours,   2, 10, QLatin1Char('0'))
            .arg(minutes, 2, 10, QLatin1Char('0'))
            .arg(seconds, 2, 10, QLatin1Char('0'))
    );

    // 速率衰减: 如果超过2秒未收到update()调用，将速率归零
    // 避免连接空闲时仍显示非零速率误导用户
    qint64 sampleAge = m_sampleTimer.elapsed();
    if (sampleAge > 2000) {
        m_rxRate = 0.0;
        m_txRate = 0.0;
    }

    // 更新速率显示
    m_rxRateLabel->setText(formatRate(m_rxRate));
    m_txRateLabel->setText(formatRate(m_txRate));

    // 更新峰值速率（取历史最大值）
    if (m_rxRate > m_peakRxRate) { m_peakRxRate = m_rxRate; ++m_totalPeakUpdates; }
    if (m_txRate > m_peakTxRate) { m_peakTxRate = m_txRate; ++m_totalPeakUpdates; }
    double peakRate = qMax(m_peakRxRate, m_peakTxRate);
    m_peakRateLabel->setText(formatRate(peakRate));

    // 计算会话平均速率（总字节/总时间）
    double elapsedSecF = qMax(m_stopwatch.elapsed() / 1000.0, 1.0);
    m_avgRxRate = static_cast<double>(m_lastRxBytes) / elapsedSecF;
    m_avgTxRate = static_cast<double>(m_lastTxBytes) / elapsedSecF;
    double avgRate = m_avgRxRate + m_avgTxRate;
    m_avgRateLabel->setText(formatRate(avgRate));
}

/** @brief 更新串口通信错误计数(帧/校验/溢出)，有错误时显示面板 @param framingErrors 帧错误数 @param parityErrors 校验错误数 @param overrunErrors 溢出错误数 */
void DataStatistics::updateErrors(int framingErrors, int parityErrors, int overrunErrors)
{
    // 更新内部计数器
    m_framingErrors = framingErrors;
    m_parityErrors = parityErrors;
    m_overrunErrors = overrunErrors;

    // 有错误时显示错误面板
    int totalErrors = framingErrors + parityErrors + overrunErrors;
    if (totalErrors > 0) {
        QStringList parts;
        if (framingErrors > 0)
            parts << tr("帧错误: %1").arg(framingErrors);
        if (parityErrors > 0)
            parts << tr("校验错误: %1").arg(parityErrors);
        if (overrunErrors > 0)
            parts << tr("溢出: %1").arg(overrunErrors);
        m_errorLabel->setText(tr("⚠ 通信错误 - ") + parts.join(" | "));
        m_errorLabel->show();
    } else {
        m_errorLabel->hide();
    }
}

/** @brief 格式化速率为人类可读字符串(B/s, KB/s, MB/s, GB/s) @param bytesPerSec 每秒字节数 @return 格式化字符串 */
QString DataStatistics::formatRate(double bytesPerSec) const
{
    if (bytesPerSec < 1024.0) {
        // 小于1KB/s
        return tr("%1 B/s").arg(bytesPerSec, 0, 'f', 0);
    } else if (bytesPerSec < 1024.0 * 1024) {
        // 小于1MB/s
        return tr("%1 KB/s").arg(bytesPerSec / 1024.0, 0, 'f', 1);
    } else if (bytesPerSec < 1024.0 * 1024 * 1024) {
        // 小于1GB/s
        return tr("%1 MB/s").arg(bytesPerSec / (1024.0 * 1024.0), 0, 'f', 2);
    } else {
        // 大于等于1GB/s
        return tr("%1 GB/s").arg(bytesPerSec / (1024.0 * 1024.0 * 1024.0), 0, 'f', 2);
    }
}

/** @brief 更新连接健康状态(空闲超10秒显示警告) @param alive 连接是否存活 @param lastDataAgeMs 距上次收到数据的毫秒数 */
void DataStatistics::updateConnectionHealth(bool alive, qint64 lastDataAgeMs)
{
    // 连接不存活时直接隐藏健康标签（由连接状态UI负责显示断开信息）
    if (!alive) {
        m_healthLabel->hide();
        return;
    }

    // 数据正常流动（10秒内收到过数据）时隐藏空闲提示
    if (lastDataAgeMs < 0 || lastDataAgeMs < 10000) {
        m_healthLabel->hide();
        return;
    }

    // 空闲超过10秒，显示空闲时长提示
    int idleSec = static_cast<int>(lastDataAgeMs / 1000);
    if (idleSec >= 60) {
        int min = idleSec / 60;
        int sec = idleSec % 60;
        m_healthLabel->setText(tr("⚠ 空闲 %1m%2s").arg(min).arg(sec));
    } else {
        m_healthLabel->setText(tr("⚠ 空闲 %1s").arg(idleSec));
    }
    m_healthLabel->show();
}

/** @brief 生成会话统计摘要(持续时间/收发/峰值/均值/错误)，用于Toast或导出 @return 多行统计文本 */
QString DataStatistics::sessionSummary() const
{
    // 生成多行会话统计摘要，用于导出或Toast通知
    qint64 elapsedMs = m_stopwatch.isValid() ? m_stopwatch.elapsed() : 0;
    int sec = static_cast<int>(elapsedMs / 1000);
    QString timeStr = QString("%1:%2:%3")
                          .arg(sec / 3600, 2, 10, QChar('0'))
                          .arg((sec % 3600) / 60, 2, 10, QChar('0'))
                          .arg(sec % 60, 2, 10, QChar('0'));

    double peakRate = qMax(m_peakRxRate, m_peakTxRate);
    int totalErrors = m_framingErrors + m_parityErrors + m_overrunErrors;

    QString summary;
    summary += tr("会话统计\n");
    summary += tr("─────────────────\n");
    summary += tr("持续时间: %1\n").arg(timeStr);
    summary += tr("接收: %1\n").arg(ByteFormat::formatSize(m_lastRxBytes));
    summary += tr("发送: %1\n").arg(ByteFormat::formatSize(m_lastTxBytes));
    summary += tr("峰值速率: %1\n").arg(formatRate(peakRate));
    summary += tr("平均速率: %1\n").arg(formatRate(m_avgRxRate + m_avgTxRate));
    summary += tr("数据包: RX %1 / TX %2\n").arg(m_rxPackets).arg(m_txPackets);
    if (totalErrors > 0) {
        summary += tr("错误: %1 (帧%2 校验%3 溢出%4)")
                       .arg(totalErrors)
                       .arg(m_framingErrors)
                       .arg(m_parityErrors)
                       .arg(m_overrunErrors);
    }
    return summary;
}

/** @brief 返回接收累计字节数 @return 接收字节总数 */
quint64 DataStatistics::totalRxBytes() const
{
    return m_lastRxBytes;
}

/** @brief 返回发送累计字节数 @return 发送字节总数 */
quint64 DataStatistics::totalTxBytes() const
{
    return m_lastTxBytes;
}

// ── 统计计数器实现 ──

/** @brief 获取update()调用总次数 @return 累计更新次数 */
quint64 DataStatistics::totalUpdates() const
{
    return m_totalUpdates;
}

/** @brief 获取历史峰值速率(RX/TX中较大者) @return 峰值速率(bytes/s) */
double DataStatistics::peakRate() const
{
    return qMax(m_peakRxRate, m_peakTxRate);
}

/** @brief 获取所有update()调用传入的字节总数(RX+TX) @return 累计字节数 */
quint64 DataStatistics::totalBytesCounted() const
{
    return m_totalBytesCounted;
}

/** @brief 重置数据统计计数器(不影响面板显示) */
void DataStatistics::resetDataStatistics()
{
    m_totalUpdates = 0;
    m_totalBytesCounted = 0;
    m_totalPeakUpdates = 0;
}

/** @brief 获取峰值速率更新总次数 @return 峰值更新次数 */
quint64 DataStatistics::totalPeakUpdates() const
{
    return m_totalPeakUpdates;
}

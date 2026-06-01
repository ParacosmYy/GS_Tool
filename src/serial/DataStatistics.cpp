#include "serial/DataStatistics.h"

#include <QFormLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QLatin1Char>

DataStatistics::DataStatistics(QWidget* parent)
    : QWidget(parent)
{
    setupUI();

    // 启动1秒定时器，用于刷新速率显示和持续时间
    connect(&m_refreshTimer, &QTimer::timeout,
            this, &DataStatistics::onRefreshTimer);
    m_refreshTimer.setInterval(1000);

    // 启动采样间隔计时器（用于计算精确速率）
    m_sampleTimer.start();

    // 启动持续时间计时器
    m_stopwatch.start();
    m_refreshTimer.start();
}

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
    mainLayout->addWidget(createStatsFrame(tr("时间:"), m_elapsedLabel, "elapsedLabel", "statsTimeFrame"));

    // ---- 串口错误计数（默认隐藏，有错误时显示） ----
    m_errorLabel = new QLabel;
    m_errorLabel->setObjectName("errorLabel");
    m_errorLabel->setWordWrap(true);
    m_errorLabel->hide();  // 无错误时不占用空间
    mainLayout->addWidget(m_errorLabel);

    // ---- 连接健康状态（默认隐藏，空闲超10秒时显示） ----
    m_healthLabel = new QLabel;
    m_healthLabel->setObjectName("healthLabel");
    m_healthLabel->setWordWrap(true);
    m_healthLabel->hide();  // 数据正常流动时隐藏
    mainLayout->addWidget(m_healthLabel);

    // 底部弹性空间
    mainLayout->addStretch();
}

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

void DataStatistics::update(quint64 rxBytes, quint64 txBytes)
{
    // 计算自上次采样以来的实际时间间隔（毫秒）
    qint64 elapsedMs = m_sampleTimer.elapsed();
    m_sampleTimer.restart();

    // 计算增量字节
    quint64 rxDelta = rxBytes - m_lastRxBytes;
    quint64 txDelta = txBytes - m_lastTxBytes;

    // 按实际时间间隔换算为 bytes/s，避免调用间隔不等于 1s 时速率失真
    double seconds = qMax(elapsedMs, static_cast<qint64>(1)) / 1000.0;
    m_rxRate = static_cast<double>(rxDelta) / seconds;
    m_txRate = static_cast<double>(txDelta) / seconds;

    // 记录本次采样值，供下次计算增量
    m_lastRxBytes = rxBytes;
    m_lastTxBytes = txBytes;

    // 更新累计字节数显示
    m_rxTotalLabel->setText(formatBytes(rxBytes));
    m_txTotalLabel->setText(formatBytes(txBytes));
}

void DataStatistics::reset()
{
    // 重置累计值和速率
    m_lastRxBytes = 0;
    m_lastTxBytes = 0;
    m_rxRate = 0.0;
    m_txRate = 0.0;
    m_peakRxRate = 0.0;
    m_peakTxRate = 0.0;

    // 重置错误计数
    m_framingErrors = 0;
    m_parityErrors = 0;
    m_overrunErrors = 0;

    // 重置UI显示
    m_rxTotalLabel->setText("0 B");
    m_txTotalLabel->setText("0 B");
    m_rxRateLabel->setText("0 B/s");
    m_txRateLabel->setText("0 B/s");
    m_peakRateLabel->setText("0 B/s");
    m_elapsedLabel->setText("00:00:00");
    m_errorLabel->hide();
    m_healthLabel->hide();

    // 重启计时器
    m_stopwatch.restart();
}

double DataStatistics::rxRate() const
{
    return m_rxRate;
}

double DataStatistics::txRate() const
{
    return m_txRate;
}

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
    if (m_rxRate > m_peakRxRate) m_peakRxRate = m_rxRate;
    if (m_txRate > m_peakTxRate) m_peakTxRate = m_txRate;
    double peakRate = qMax(m_peakRxRate, m_peakTxRate);
    m_peakRateLabel->setText(formatRate(peakRate));
}

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

QString DataStatistics::formatBytes(quint64 bytes) const
{
    if (bytes < 1024) {
        // 小于1KB，直接显示字节
        return QString("%1 B").arg(bytes);
    } else if (bytes < 1024ULL * 1024) {
        // 小于1MB，显示KB，保留1位小数
        return QString("%1 KB").arg(bytes / 1024.0, 0, 'f', 1);
    } else if (bytes < 1024ULL * 1024 * 1024) {
        // 小于1GB，显示MB，保留2位小数
        return QString("%1 MB").arg(bytes / (1024.0 * 1024.0), 0, 'f', 2);
    } else {
        // 大于等于1GB，显示GB，保留2位小数
        return QString("%1 GB").arg(bytes / (1024.0 * 1024.0 * 1024.0), 0, 'f', 2);
    }
}

QString DataStatistics::formatRate(double bytesPerSec) const
{
    if (bytesPerSec < 1024.0) {
        // 小于1KB/s
        return QString("%1 B/s").arg(bytesPerSec, 0, 'f', 0);
    } else if (bytesPerSec < 1024.0 * 1024) {
        // 小于1MB/s
        return QString("%1 KB/s").arg(bytesPerSec / 1024.0, 0, 'f', 1);
    } else if (bytesPerSec < 1024.0 * 1024 * 1024) {
        // 小于1GB/s
        return QString("%1 MB/s").arg(bytesPerSec / (1024.0 * 1024.0), 0, 'f', 2);
    } else {
        // 大于等于1GB/s
        return QString("%1 GB/s").arg(bytesPerSec / (1024.0 * 1024.0 * 1024.0), 0, 'f', 2);
    }
}

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
    summary += tr("接收: %1\n").arg(formatBytes(m_lastRxBytes));
    summary += tr("发送: %1\n").arg(formatBytes(m_lastTxBytes));
    summary += tr("峰值速率: %1\n").arg(formatRate(peakRate));
    if (totalErrors > 0) {
        summary += tr("错误: %1 (帧%2 校验%3 溢出%4)")
                       .arg(totalErrors)
                       .arg(m_framingErrors)
                       .arg(m_parityErrors)
                       .arg(m_overrunErrors);
    }
    return summary;
}

quint64 DataStatistics::totalRxBytes() const
{
    return m_lastRxBytes;
}

quint64 DataStatistics::totalTxBytes() const
{
    return m_lastTxBytes;
}

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
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(4);

    // ---- 接收（RX）统计区域 ----
    auto* rxFrame = new QFrame;
    rxFrame->setObjectName("statsFrame");
    rxFrame->setFrameShape(QFrame::StyledPanel);
    auto* rxLayout = new QFormLayout(rxFrame);
    rxLayout->setContentsMargins(8, 6, 8, 6);
    rxLayout->setSpacing(4);
    rxLayout->setLabelAlignment(Qt::AlignRight);

    // RX累计标签
    m_rxTotalLabel = new QLabel(QStringLiteral("0 B"));
    m_rxTotalLabel->setObjectName("rxTotalLabel");
    rxLayout->addRow(tr("RX:"), m_rxTotalLabel);

    // RX速率标签
    m_rxRateLabel = new QLabel(QStringLiteral("0 B/s"));
    m_rxRateLabel->setObjectName("rxRateLabel");
    rxLayout->addRow(tr("Rate:"), m_rxRateLabel);

    mainLayout->addWidget(rxFrame);

    // ---- 发送（TX）统计区域 ----
    auto* txFrame = new QFrame;
    txFrame->setObjectName("statsFrame");
    txFrame->setFrameShape(QFrame::StyledPanel);
    auto* txLayout = new QFormLayout(txFrame);
    txLayout->setContentsMargins(8, 6, 8, 6);
    txLayout->setSpacing(4);
    txLayout->setLabelAlignment(Qt::AlignRight);

    // TX累计标签
    m_txTotalLabel = new QLabel(QStringLiteral("0 B"));
    m_txTotalLabel->setObjectName("txTotalLabel");
    txLayout->addRow(tr("TX:"), m_txTotalLabel);

    // TX速率标签
    m_txRateLabel = new QLabel(QStringLiteral("0 B/s"));
    m_txRateLabel->setObjectName("txRateLabel");
    txLayout->addRow(tr("Rate:"), m_txRateLabel);

    mainLayout->addWidget(txFrame);

    // ---- 连接持续时间 ----
    auto* elapsedFrame = new QFrame;
    elapsedFrame->setObjectName("statsFrame");
    elapsedFrame->setFrameShape(QFrame::StyledPanel);
    auto* elapsedLayout = new QFormLayout(elapsedFrame);
    elapsedLayout->setContentsMargins(8, 6, 8, 6);
    elapsedLayout->setSpacing(4);
    elapsedLayout->setLabelAlignment(Qt::AlignRight);

    m_elapsedLabel = new QLabel(QStringLiteral("00:00:00"));
    m_elapsedLabel->setObjectName("elapsedLabel");
    elapsedLayout->addRow(tr("Time:"), m_elapsedLabel);

    mainLayout->addWidget(elapsedFrame);

    // 底部弹性空间
    mainLayout->addStretch();
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

    // 重置UI显示
    m_rxTotalLabel->setText("0 B");
    m_txTotalLabel->setText("0 B");
    m_rxRateLabel->setText("0 B/s");
    m_txRateLabel->setText("0 B/s");
    m_elapsedLabel->setText("00:00:00");

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
    // 读取当前累计值（从标签文本无法直接获取数值，所以从label无法计算）
    // 这里需要外部通过update()传入累计值，我们存储上次采样值来计算速率
    // 注意：实际的速率计算依赖于update()被调用时保存的lastRxBytes/lastTxBytes
    //       这里我们用一个简单的方案：每次定时器触发时，速率 = 上次update传来的累计值 - 上次采样值

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

    // 更新速率显示
    m_rxRateLabel->setText(formatRate(m_rxRate));
    m_txRateLabel->setText(formatRate(m_txRate));
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

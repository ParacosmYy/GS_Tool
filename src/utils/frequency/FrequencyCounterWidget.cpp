/**
 * @file FrequencyCounterWidget.cpp
 * @brief 频率计数器控件实现 — 构造函数、UI布局、定时器回调、显示刷新
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 统计查询与重置方法见：@see FrequencyCounterWidgetStats.cpp
 */

#include "utils/frequency/FrequencyCounterWidget.h"

#include <QComboBox>
#include <QFont>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

// ──────────────────────────────────────────────
// 构造与 UI
// ──────────────────────────────────────────────

/**
 * @brief 构造函数，初始化面板、启动定时器
 */
FrequencyCounterWidget::FrequencyCounterWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();

    // 窗口定时器：每秒触发一次，用于计算频率
    connect(&m_windowTimer, &QTimer::timeout,
            this, &FrequencyCounterWidget::onWindowTick);
    m_windowTimer.start(1000);

    // 运行计时器
    m_elapsedTimer.start();

    // 控件信号
    connect(m_resetBtn, &QPushButton::clicked,
            this, &FrequencyCounterWidget::onResetClicked);
    connect(m_windowCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &FrequencyCounterWidget::onWindowChanged);
}

/**
 * @brief 初始化界面布局，所有控件设置 objectName 供 QSS 匹配
 */
void FrequencyCounterWidget::setupUI()
{
    setObjectName(QStringLiteral("FrequencyCounterWidget"));

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(8, 8, 8, 8);
    root->setSpacing(6);

    // ── 顶部：窗口选择 + 重置 ──
    auto *topBar = new QHBoxLayout();
    topBar->setSpacing(8);

    m_windowCombo = new QComboBox(this);
    m_windowCombo->setObjectName("fcWindowCombo");
    m_windowCombo->addItem(tr("1 秒"),  static_cast<int>(WindowSize::Sec1));
    m_windowCombo->addItem(tr("5 秒"),  static_cast<int>(WindowSize::Sec5));
    m_windowCombo->addItem(tr("10 秒"), static_cast<int>(WindowSize::Sec10));
    m_windowCombo->addItem(tr("60 秒"), static_cast<int>(WindowSize::Sec60));
    topBar->addWidget(new QLabel(tr("测量窗口：")));
    topBar->addWidget(m_windowCombo);
    topBar->addStretch();

    m_resetBtn = new QPushButton(tr("重置统计"), this);
    m_resetBtn->setObjectName("fcResetBtn");
    topBar->addWidget(m_resetBtn);
    root->addLayout(topBar);

    // ── 大数字频率显示 ──
    auto *freqLayout = new QHBoxLayout();
    freqLayout->setSpacing(16);

    // 数据包频率
    auto *pktBox = new QVBoxLayout();
    auto *pktTitle = new QLabel(tr("数据包频率"), this);
    pktTitle->setObjectName("fcPktTitle");
    pktTitle->setAlignment(Qt::AlignCenter);
    m_packetFreqLabel = new QLabel(QStringLiteral("0.00 Hz"), this);
    m_packetFreqLabel->setObjectName("fcPacketFreqLabel");
    m_packetFreqLabel->setAlignment(Qt::AlignCenter);
    QFont bigFont;
    bigFont.setPointSize(20);
    bigFont.setBold(true);
    m_packetFreqLabel->setFont(bigFont);
    pktBox->addWidget(pktTitle);
    pktBox->addWidget(m_packetFreqLabel);
    freqLayout->addLayout(pktBox);

    // 字节率
    auto *byteBox = new QVBoxLayout();
    auto *byteTitle = new QLabel(tr("字节吞吐率"), this);
    byteTitle->setObjectName("fcByteTitle");
    byteTitle->setAlignment(Qt::AlignCenter);
    m_byteRateLabel = new QLabel(QStringLiteral("0 B/s"), this);
    m_byteRateLabel->setObjectName("fcByteRateLabel");
    m_byteRateLabel->setAlignment(Qt::AlignCenter);
    m_byteRateLabel->setFont(bigFont);
    byteBox->addWidget(byteTitle);
    byteBox->addWidget(m_byteRateLabel);
    freqLayout->addLayout(byteBox);

    root->addLayout(freqLayout);

    // ── 峰值/平均/累计行 ──
    auto *statsLayout = new QHBoxLayout();
    statsLayout->setSpacing(12);

    m_peakLabel = new QLabel(tr("峰值: 0.00 Hz / 0 B/s"), this);
    m_peakLabel->setObjectName("fcPeakLabel");
    statsLayout->addWidget(m_peakLabel);

    m_avgLabel = new QLabel(tr("平均: 0.00 Hz"), this);
    m_avgLabel->setObjectName("fcAvgLabel");
    statsLayout->addWidget(m_avgLabel);

    m_totalLabel = new QLabel(tr("累计: 0 包 / 0 B"), this);
    m_totalLabel->setObjectName("fcTotalLabel");
    statsLayout->addWidget(m_totalLabel);

    statsLayout->addStretch();
    root->addLayout(statsLayout);

    // ── 模式频率表 ──
    m_patternTable = new QTableWidget(0, 4, this);
    m_patternTable->setObjectName("fcPatternTable");
    m_patternTable->setHorizontalHeaderLabels(
        {tr("模式"), tr("当前频率"), tr("峰值频率"), tr("累计次数")});
    m_patternTable->horizontalHeader()->setStretchLastSection(true);
    m_patternTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_patternTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_patternTable->setMaximumHeight(200);
    root->addWidget(m_patternTable);
}

// ──────────────────────────────────────────────
// 数据记录接口
// ──────────────────────────────────────────────

/**
 * @brief 记录一次数据包到达事件，递增窗口内数据包计数器
 */
void FrequencyCounterWidget::recordPacket()
{
    ++m_windowPackets;
    ++m_totalPackets;
}

/**
 * @brief 记录接收到的字节数 @param bytes 字节数
 */
void FrequencyCounterWidget::recordBytes(qint64 bytes)
{
    m_windowBytes += static_cast<quint64>(bytes);
    m_totalBytes += static_cast<quint64>(bytes);
}

/**
 * @brief 记录特定模式的一次匹配 @param pattern 模式名称
 */
void FrequencyCounterWidget::recordPattern(const QString &pattern)
{
    ++m_windowPatterns[pattern];
}

/**
 * @brief 设置测量窗口大小 @param size 窗口大小
 */
void FrequencyCounterWidget::setWindowSize(WindowSize size)
{
    m_windowSize = size;
}

// ──────────────────────────────────────────────
// 定时器回调
// ──────────────────────────────────────────────

/**
 * @brief 每秒触发，根据窗口内累计计数计算频率并刷新显示
 */
void FrequencyCounterWidget::onWindowTick()
{
    const double ws = static_cast<double>(static_cast<int>(m_windowSize));

    // 计算数据包频率
    m_lastPacketFreq = static_cast<double>(m_windowPackets) / ws;
    if (m_lastPacketFreq > m_peakPacketFreq) {
        m_peakPacketFreq = m_lastPacketFreq;
    }

    // 计算字节率
    m_lastByteRate = static_cast<double>(m_windowBytes) / ws;
    if (m_lastByteRate > m_peakByteRateLocal) {
        m_peakByteRateLocal = m_lastByteRate;
    }

    // 累计窗口统计
    ++m_windowCount;
    m_freqSum += m_lastPacketFreq;

    // 更新全局统计
    m_stats.totalEvents = m_totalPackets;
    m_stats.totalBytesProcessed = m_totalBytes;
    m_stats.totalWindows = static_cast<quint64>(m_windowCount);
    if (m_peakPacketFreq > m_stats.peakPacketRate) {
        m_stats.peakPacketRate = m_peakPacketFreq;
    }
    if (m_peakByteRateLocal > m_stats.peakByteRate) {
        m_stats.peakByteRate = m_peakByteRateLocal;
    }
    m_stats.activeChannels = 2 + m_windowPatterns.size();

    // 发射频率更新信号
    emit frequencyUpdated(tr("数据包"), m_lastPacketFreq);
    emit frequencyUpdated(tr("字节率"), m_lastByteRate);

    // 更新模式频率表
    updatePatternTable(ws);

    // 重置窗口计数器
    m_windowPackets = 0;
    m_windowBytes = 0;
    m_windowPatterns.clear();

    emit windowExpired();
    updateDisplay();
}

// ──────────────────────────────────────────────
// 显示刷新
// ──────────────────────────────────────────────

/**
 * @brief 将数值格式化为带合适单位的字符串
 * @param value 原始值 @param unit 基本单位 @return 格式化后的字符串
 */
static QString formatFreq(double value, const QString &unit)
{
    if (value >= 1e6) {
        return QString::number(value / 1e6, 'f', 2) + QStringLiteral(" M") + unit;
    }
    if (value >= 1e3) {
        return QString::number(value / 1e3, 'f', 2) + QStringLiteral(" k") + unit;
    }
    return QString::number(value, 'f', 2) + QStringLiteral(" ") + unit;
}

/**
 * @brief 刷新所有频率/统计标签的显示内容
 */
void FrequencyCounterWidget::updateDisplay()
{
    // 数据包频率
    m_packetFreqLabel->setText(formatFreq(m_lastPacketFreq, tr("Hz")));

    // 字节率
    m_byteRateLabel->setText(formatFreq(m_lastByteRate, tr("B/s")));

    // 峰值
    m_peakLabel->setText(
        tr("峰值: %1 / %2")
            .arg(formatFreq(m_peakPacketFreq, tr("Hz")))
            .arg(formatFreq(m_peakByteRateLocal, tr("B/s"))));

    // 平均
    const double avgHz = (m_windowCount > 0)
                             ? m_freqSum / m_windowCount
                             : 0.0;
    m_avgLabel->setText(tr("平均: %1").arg(formatFreq(avgHz, tr("Hz"))));

    // 累计
    m_totalLabel->setText(
        tr("累计: %1 包 / %2 B")
            .arg(m_totalPackets)
            .arg(m_totalBytes));
}

// ──────────────────────────────────────────────
// 模式频率表更新
// ──────────────────────────────────────────────

/** @brief 模式历史追踪，保存峰值频率和累计次数 */
static QMap<QString, double>  s_patternPeakFreq;
static QMap<QString, quint64> s_patternTotal;

/**
 * @brief 更新模式频率表 @param ws 当前窗口大小(秒)
 */
void FrequencyCounterWidget::updatePatternTable(double ws)
{
    const QStringList patterns = m_windowPatterns.keys();
    m_patternTable->setRowCount(patterns.size());

    for (int i = 0; i < patterns.size(); ++i) {
        const QString &pat = patterns[i];
        const quint64 count = m_windowPatterns[pat];
        const double freq = static_cast<double>(count) / ws;

        // 更新峰值
        if (freq > s_patternPeakFreq.value(pat, 0.0)) {
            s_patternPeakFreq[pat] = freq;
        }
        // 累计总数
        s_patternTotal[pat] += count;

        auto *nameItem = new QTableWidgetItem(pat);
        auto *freqItem = new QTableWidgetItem(formatFreq(freq, tr("Hz")));
        auto *peakItem = new QTableWidgetItem(
            formatFreq(s_patternPeakFreq[pat], tr("Hz")));
        auto *totalItem = new QTableWidgetItem(
            QString::number(s_patternTotal[pat]));

        m_patternTable->setItem(i, 0, nameItem);
        m_patternTable->setItem(i, 1, freqItem);
        m_patternTable->setItem(i, 2, peakItem);
        m_patternTable->setItem(i, 3, totalItem);
    }
}

// ──────────────────────────────────────────────
// 槽函数
// ──────────────────────────────────────────────

/**
 * @brief 重置按钮点击 — 清零所有计数器和统计
 */
void FrequencyCounterWidget::onResetClicked()
{
    resetStatistics();
    s_patternPeakFreq.clear();
    s_patternTotal.clear();
    m_patternTable->setRowCount(0);
    updateDisplay();
}

/**
 * @brief 窗口选择变更 @param index 组合框当前索引
 */
void FrequencyCounterWidget::onWindowChanged(int index)
{
    if (index < 0) return;
    const int val = m_windowCombo->itemData(index).toInt();
    m_windowSize = static_cast<WindowSize>(val);
}

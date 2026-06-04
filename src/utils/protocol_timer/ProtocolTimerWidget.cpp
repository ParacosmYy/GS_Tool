/**
 * @file ProtocolTimerWidget.cpp
 * @brief 协议定时分析器控件实现 -- 构造函数、UI布局、事件记录、统计计算、导出
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 统计重置方法见：@see ProtocolTimerWidgetStats.cpp
 */

#include "utils/protocol_timer/ProtocolTimerWidget.h"

#include <QComboBox>
#include <QDateTime>
#include <QFile>
#include <QFileDialog>
#include <QFont>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QTextStream>
#include <QVBoxLayout>

#include <cmath>

// ── 构造与 UI ──

/**
 * @brief 构造函数，初始化面板和定时器
 */
ProtocolTimerWidget::ProtocolTimerWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();

    // 突发检测定时器：每50ms清理过期窗口事件
    m_burstTimer = new QTimer(this);
    m_burstTimer->setInterval(50);
    connect(m_burstTimer, &QTimer::timeout,
            this, &ProtocolTimerWidget::detectBurst);

    // 运行计时器
    m_elapsedTimer.start();

    // 按钮信号
    connect(m_startBtn, &QPushButton::clicked,
            this, &ProtocolTimerWidget::onStartClicked);
    connect(m_stopBtn, &QPushButton::clicked,
            this, &ProtocolTimerWidget::onStopClicked);
    connect(m_resetBtn, &QPushButton::clicked,
            this, &ProtocolTimerWidget::onResetClicked);
    connect(m_exportBtn, &QPushButton::clicked,
            this, &ProtocolTimerWidget::onExportClicked);
    connect(m_modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ProtocolTimerWidget::onModeChanged);

    // 初始状态：停止按钮禁用
    m_stopBtn->setEnabled(false);
}

/**
 * @brief 初始化界面布局，所有控件设置 objectName 供 QSS 匹配
 */
void ProtocolTimerWidget::setupUI()
{
    setObjectName(QStringLiteral("ProtocolTimerWidget"));

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(8, 8, 8, 8);
    root->setSpacing(6);

    // ── 顶部栏：模式选择 + 操作按钮 ──
    auto *topBar = new QHBoxLayout();
    topBar->setSpacing(8);

    auto *modeLabel = new QLabel(tr("测量模式："), this);
    modeLabel->setObjectName("ptModeLabel");
    topBar->addWidget(modeLabel);

    m_modeCombo = new QComboBox(this);
    m_modeCombo->setObjectName("ptModeCombo");
    m_modeCombo->addItem(tr("帧间延迟"),   static_cast<int>(TimingMode::InterFrame));
    m_modeCombo->addItem(tr("响应延迟"),   static_cast<int>(TimingMode::ResponseLatency));
    m_modeCombo->addItem(tr("突发检测"),   static_cast<int>(TimingMode::Burst));
    m_modeCombo->addItem(tr("手动触发"),   static_cast<int>(TimingMode::Manual));
    topBar->addWidget(m_modeCombo);

    topBar->addStretch();

    m_startBtn = new QPushButton(tr("开始"), this);
    m_startBtn->setObjectName("ptStartBtn");
    topBar->addWidget(m_startBtn);

    m_stopBtn = new QPushButton(tr("停止"), this);
    m_stopBtn->setObjectName("ptStopBtn");
    topBar->addWidget(m_stopBtn);

    m_resetBtn = new QPushButton(tr("重置"), this);
    m_resetBtn->setObjectName("ptResetBtn");
    topBar->addWidget(m_resetBtn);

    m_exportBtn = new QPushButton(tr("导出CSV"), this);
    m_exportBtn->setObjectName("ptExportBtn");
    topBar->addWidget(m_exportBtn);

    root->addLayout(topBar);

    // ── 大数字时序显示 ──
    auto *currentBox = new QVBoxLayout();
    auto *currentTitle = new QLabel(tr("当前间隔 (ms)"), this);
    currentTitle->setObjectName("ptCurrentTitle");
    currentTitle->setAlignment(Qt::AlignCenter);
    m_currentLabel = new QLabel(QStringLiteral("---"), this);
    m_currentLabel->setObjectName("ptCurrentLabel");
    m_currentLabel->setAlignment(Qt::AlignCenter);
    QFont bigFont;
    bigFont.setPointSize(22);
    bigFont.setBold(true);
    m_currentLabel->setFont(bigFont);
    currentBox->addWidget(currentTitle);
    currentBox->addWidget(m_currentLabel);
    root->addLayout(currentBox);

    // ── 统计行：min / max / avg / jitter ──
    auto *statsLayout = new QHBoxLayout();
    statsLayout->setSpacing(16);

    m_minLabel = new QLabel(tr("最小: ---"), this);
    m_minLabel->setObjectName("ptMinLabel");
    statsLayout->addWidget(m_minLabel);

    m_maxLabel = new QLabel(tr("最大: ---"), this);
    m_maxLabel->setObjectName("ptMaxLabel");
    statsLayout->addWidget(m_maxLabel);

    m_avgLabel = new QLabel(tr("平均: ---"), this);
    m_avgLabel->setObjectName("ptAvgLabel");
    statsLayout->addWidget(m_avgLabel);

    m_jitterLabel = new QLabel(tr("抖动: ---"), this);
    m_jitterLabel->setObjectName("ptJitterLabel");
    statsLayout->addWidget(m_jitterLabel);

    statsLayout->addStretch();
    root->addLayout(statsLayout);

    // ── 突发指示标签 ──
    m_burstLabel = new QLabel(QString(), this);
    m_burstLabel->setObjectName("ptBurstLabel");
    root->addWidget(m_burstLabel);

    // ── 事件列表表格 ──
    m_eventTable = new QTableWidget(0, 4, this);
    m_eventTable->setObjectName("ptEventTable");
    m_eventTable->setHorizontalHeaderLabels(
        {tr("时间戳 (ns)"), tr("数据"), tr("标签"), tr("间隔 (ms)")});
    m_eventTable->horizontalHeader()->setStretchLastSection(true);
    m_eventTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_eventTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_eventTable->setMaximumHeight(250);
    root->addWidget(m_eventTable);
}

// ── 测量控制 ──

/**
 * @brief 开始测量，启动定时器和突发检测
 */
void ProtocolTimerWidget::startMeasurement()
{
    m_measuring = true;
    m_elapsedTimer.restart();
    m_events.clear();
    m_burstWindow.clear();
    m_eventTable->setRowCount(0);
    m_burstTimer->start();

    m_startBtn->setEnabled(false);
    m_stopBtn->setEnabled(true);
    m_currentLabel->setText(QStringLiteral("---"));
    refreshStatsLabels();
    m_burstLabel->setText(QString());
}

/**
 * @brief 停止测量，发射测量完成信号
 */
void ProtocolTimerWidget::stopMeasurement()
{
    m_measuring = false;
    m_burstTimer->stop();

    m_startBtn->setEnabled(true);
    m_stopBtn->setEnabled(false);

    // 发射最终统计
    const TimingStats ts = timingStatistics();
    emit measurementComplete(ts);
}

/**
 * @brief 设置测量模式 @param mode 目标模式
 */
void ProtocolTimerWidget::setMode(TimingMode mode)
{
    m_mode = mode;
}

// ── 事件记录 ──

/**
 * @brief 记录一个协议事件，计算与前一事件的间隔并更新显示
 * @param data 事件关联数据
 * @param label 事件标签
 */
void ProtocolTimerWidget::recordEvent(const QByteArray &data, const QString &label)
{
    if (!m_measuring) return;

    const qint64 ns = m_elapsedTimer.nsecsElapsed();

    TimingEvent evt;
    evt.timestampNs = ns;
    evt.data        = data;
    evt.label       = label;
    m_events.append(evt);

    // 更新全局统计
    ++m_stats.totalEvents;

    // 计算与前一事件的间隔
    double deltaMs = 0.0;
    if (m_events.size() >= 2) {
        const qint64 prevNs = m_events[m_events.size() - 2].timestampNs;
        deltaMs = static_cast<double>(ns - prevNs) / 1e6;
        ++m_stats.totalMeasurements;

        emit timingUpdated(deltaMs);
    }

    // 添加到突发检测窗口
    m_burstWindow.append(ns);

    // 添加表格行
    const int row = m_eventTable->rowCount();
    m_eventTable->insertRow(row);
    addEventRow(row, evt, deltaMs);
    m_eventTable->scrollToBottom();

    // 更新大数字显示
    if (m_events.size() >= 2) {
        m_currentLabel->setText(QString::number(deltaMs, 'f', 3));
    }
    refreshStatsLabels();
}

/**
 * @brief 向表格添加一行数据 @param row 行号 @param evt 事件 @param deltaMs 间隔
 */
void ProtocolTimerWidget::addEventRow(int row, const TimingEvent &evt, double deltaMs)
{
    auto *tsItem = new QTableWidgetItem(QString::number(evt.timestampNs));
    tsItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    // 数据列：以十六进制显示，限制长度
    QString hexStr;
    if (evt.data.size() <= 32) {
        hexStr = QString::fromUtf8(evt.data.toHex(' '));
    } else {
        hexStr = QString::fromUtf8(evt.data.left(32).toHex(' ')) + QStringLiteral("...");
    }
    auto *dataItem = new QTableWidgetItem(hexStr);
    dataItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    auto *labelItem = new QTableWidgetItem(evt.label);
    labelItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    auto *deltaItem = new QTableWidgetItem(
        (row > 0) ? QString::number(deltaMs, 'f', 3) : QStringLiteral("---"));
    deltaItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_eventTable->setItem(row, 0, tsItem);
    m_eventTable->setItem(row, 1, dataItem);
    m_eventTable->setItem(row, 2, labelItem);
    m_eventTable->setItem(row, 3, deltaItem);
}

// ── 时序统计计算 ──

/**
 * @brief 计算当前所有事件的时序统计 @return 统计结果
 */
ProtocolTimerWidget::TimingStats ProtocolTimerWidget::timingStatistics() const
{
    TimingStats ts;
    if (m_events.size() < 2) return ts;

    // 计算所有间隔
    const int n = m_events.size() - 1;
    QList<double> deltas;
    deltas.reserve(n);
    double sum = 0.0;
    double minVal = 1e18;
    double maxVal = 0.0;

    for (int i = 1; i < m_events.size(); ++i) {
        const double d = static_cast<double>(
            m_events[i].timestampNs - m_events[i - 1].timestampNs) / 1e6;
        deltas.append(d);
        sum += d;
        if (d < minVal) minVal = d;
        if (d > maxVal) maxVal = d;
    }

    const double avg = sum / static_cast<double>(n);

    // 计算标准差
    double variance = 0.0;
    for (const double d : deltas) {
        const double diff = d - avg;
        variance += diff * diff;
    }
    variance /= static_cast<double>(n);

    // 计算抖动: 相邻间隔差的均值
    double jitterSum = 0.0;
    for (int i = 1; i < deltas.size(); ++i) {
        jitterSum += std::abs(deltas[i] - deltas[i - 1]);
    }
    const double jitter = (deltas.size() > 1)
                              ? jitterSum / static_cast<double>(deltas.size() - 1)
                              : 0.0;

    ts.minMs    = minVal;
    ts.maxMs    = maxVal;
    ts.avgMs    = avg;
    ts.jitterMs = jitter;
    ts.stddevMs = std::sqrt(variance);
    ts.count    = static_cast<quint64>(n);
    return ts;
}

// ── 突发帧检测 ──

/**
 * @brief 检测滑动窗口内的突发帧，超过阈值时发射 burstDetected 信号
 */
void ProtocolTimerWidget::detectBurst()
{
    if (m_burstWindow.isEmpty()) return;

    const qint64 nowNs = m_elapsedTimer.nsecsElapsed();
    const qint64 cutoffNs = nowNs - m_burstWindowNs;

    // 移除窗口外的旧事件
    while (!m_burstWindow.isEmpty() && m_burstWindow.first() < cutoffNs) {
        m_burstWindow.removeFirst();
    }

    // 更新峰值率
    const double windowSec = static_cast<double>(m_burstWindowNs) / 1e9;
    const double rateHz = static_cast<double>(m_burstWindow.size()) / windowSec;
    if (rateHz > m_stats.peakRateHz) {
        m_stats.peakRateHz = rateHz;
    }

    // 突发判定
    if (static_cast<quint64>(m_burstWindow.size()) >= m_burstThreshold) {
        m_burstLabel->setText(
            tr("突发检测: %1 帧 / %2 ms")
                .arg(m_burstWindow.size())
                .arg(static_cast<double>(m_burstWindowNs) / 1e6, 0, 'f', 0));
        emit burstDetected(static_cast<quint64>(m_burstWindow.size()));
    } else {
        m_burstLabel->setText(QString());
    }
}

// ── 导出 ──

/**
 * @brief 导出时序数据为CSV文件 @param filePath 目标文件路径
 *
 * CSV列: timestamp_ns, data, label, delta_ms
 */
void ProtocolTimerWidget::exportTiming(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;

    QTextStream out(&file);
    // Qt 6 默认UTF-8编码，无需setCodec

    // 表头
    out << "timestamp_ns,data,label,delta_ms\n";
    // 数据行
    for (int i = 0; i < m_events.size(); ++i) {
        const TimingEvent &evt = m_events[i];
        out << evt.timestampNs << ",";
        out << QString::fromUtf8(evt.data.toHex()) << ",";
        // 标签: 如包含逗号或引号则加引号转义
        QString label = evt.label;
        if (label.contains(QLatin1Char(',')) || label.contains(QLatin1Char('"'))) {
            label.replace(QLatin1Char('"'), QStringLiteral("\\\""));
            label = QStringLiteral("\"") + label + QStringLiteral("\"");
        }
        out << label << ",";

        // 间隔
        if (i > 0) {
            const double delta = static_cast<double>(
                evt.timestampNs - m_events[i - 1].timestampNs) / 1e6;
            out << QString::number(delta, 'f', 6);
        }
        out << "\n";
    }

    file.close();
    ++m_stats.totalExports;
}

// ── 显示刷新 ──

/**
 * @brief 刷新统计标签内容
 */
void ProtocolTimerWidget::refreshStatsLabels()
{
    const TimingStats ts = timingStatistics();
    if (ts.count == 0) {
        m_minLabel->setText(tr("最小: ---"));
        m_maxLabel->setText(tr("最大: ---"));
        m_avgLabel->setText(tr("平均: ---"));
        m_jitterLabel->setText(tr("抖动: ---"));
        return;
    }

    m_minLabel->setText(tr("最小: %1 ms").arg(ts.minMs, 0, 'f', 3));
    m_maxLabel->setText(tr("最大: %1 ms").arg(ts.maxMs, 0, 'f', 3));
    m_avgLabel->setText(tr("平均: %1 ms").arg(ts.avgMs, 0, 'f', 3));
    m_jitterLabel->setText(tr("抖动: %1 ms").arg(ts.jitterMs, 0, 'f', 3));
}

// ── 槽函数 ──

/**
 * @brief 开始按钮点击
 */
void ProtocolTimerWidget::onStartClicked()
{
    startMeasurement();
}

/**
 * @brief 停止按钮点击
 */
void ProtocolTimerWidget::onStopClicked()
{
    stopMeasurement();
}

/**
 * @brief 重置按钮点击 -- 清空所有记录和统计
 */
void ProtocolTimerWidget::onResetClicked()
{
    resetStatistics();
    m_events.clear();
    m_burstWindow.clear();
    m_eventTable->setRowCount(0);
    m_currentLabel->setText(QStringLiteral("---"));
    m_burstLabel->setText(QString());
    refreshStatsLabels();
}

/**
 * @brief 导出按钮点击 -- 弹出文件对话框导出CSV
 */
void ProtocolTimerWidget::onExportClicked()
{
    if (m_events.isEmpty()) return;

    const QString defaultPath = QStringLiteral("protocol_timing_%1.csv")
        .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss")));

    const QString filePath = QFileDialog::getSaveFileName(
        this, tr("导出时序数据"), defaultPath,
        tr("CSV 文件 (*.csv);;所有文件 (*)"));

    if (!filePath.isEmpty()) {
        exportTiming(filePath);
    }
}

/**
 * @brief 模式选择变更 @param index 组合框当前索引
 */
void ProtocolTimerWidget::onModeChanged(int index)
{
    if (index < 0) return;
    const int val = m_modeCombo->itemData(index).toInt();
    m_mode = static_cast<TimingMode>(val);
}

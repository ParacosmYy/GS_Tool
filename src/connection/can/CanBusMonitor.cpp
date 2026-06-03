/**
 * @file CanBusMonitor.cpp
 * @brief CAN总线监控面板实现 — 表格显示、颜色编码、自动滚动、信号解码、统计面板
 *
 * 支持帧列表显示(含过滤)、DBC信号解码面板、帧类型统计面板。
 * 颜色编码: 标准帧白色、扩展帧浅蓝、RTR帧黄色、CAN-FD帧浅绿、错误帧红色。
 */

#include "connection/can/CanBusMonitor.h"
#include "protocol/can/DbcParser.h"
#include "core/theme/ThemeManager.h"
#include <QHeaderView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QTime>
#include <QBrush>
#include <QColor>

/** @brief 构造CAN总线监控面板，初始化表格/信号面板/统计面板 @param parent 父控件 */
CanBusMonitor::CanBusMonitor(QWidget* parent)
    : QWidget(parent)
    , m_frameTable(new QTableWidget(this))
    , m_signalTable(new QTableWidget(this))
    , m_countLabel(new QLabel(tr("帧数: 0"), this))
    , m_clearBtn(new QPushButton(tr("清空"), this))
    , m_autoScrollCheck(new QCheckBox(tr("自动滚动"), this))
    , m_filterEdit(new QLineEdit(this))
    , m_typeFilterCombo(new QComboBox(this))
    , m_statsLabel(new QLabel(this))
{
    setObjectName("CanBusMonitor");

    /* ── 帧列表表格 ── */
    m_frameTable->setObjectName("canFrameTable");
    m_frameTable->setColumnCount(7);
    m_frameTable->setHorizontalHeaderLabels({
        tr("时间"), tr("帧ID"), tr("DLC"), tr("数据"),
        tr("扩展帧"), tr("RTR"), tr("计数")
    });
    m_frameTable->horizontalHeader()->setStretchLastSection(true);
    m_frameTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_frameTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_frameTable->setAlternatingRowColors(true);

    /* ── 信号解码面板 ── */
    m_signalTable->setObjectName("canSignalTable");
    m_signalTable->setColumnCount(4);
    m_signalTable->setHorizontalHeaderLabels({
        tr("信号名"), tr("物理值"), tr("单位"), tr("描述")
    });
    m_signalTable->horizontalHeader()->setStretchLastSection(true);
    m_signalTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_signalTable->setMaximumHeight(200);

    /* ── 工具栏 ── */
    m_countLabel->setObjectName("canCountLabel");
    m_clearBtn->setObjectName("canClearBtn");
    m_autoScrollCheck->setObjectName("canAutoScrollCheck");
    m_autoScrollCheck->setChecked(true);

    m_filterEdit->setObjectName("canFilterEdit");
    m_filterEdit->setPlaceholderText(tr("帧ID过滤(如 0x123)"));
    m_filterEdit->setMaximumWidth(160);

    m_typeFilterCombo->setObjectName("canTypeFilterCombo");
    m_typeFilterCombo->addItem(tr("全部"), QVariant::fromValue(0));
    m_typeFilterCombo->addItem(tr("标准帧"), QVariant::fromValue(1));
    m_typeFilterCombo->addItem(tr("扩展帧"), QVariant::fromValue(2));
    m_typeFilterCombo->addItem(tr("RTR帧"), QVariant::fromValue(3));
    m_typeFilterCombo->addItem(tr("CAN-FD帧"), QVariant::fromValue(4));
    m_typeFilterCombo->setMaximumWidth(100);

    auto toolbar = new QHBoxLayout();
    toolbar->addWidget(m_countLabel, 1);
    toolbar->addWidget(m_filterEdit);
    toolbar->addWidget(m_typeFilterCombo);
    toolbar->addWidget(m_autoScrollCheck);
    toolbar->addWidget(m_clearBtn);

    /* ── 统计面板 ── */
    m_statsLabel->setObjectName("canStatsLabel");
    m_statsLabel->setWordWrap(true);
    m_statsLabel->setFrameShape(QFrame::StyledPanel);

    /* ── 使用分割器布局 ── */
    auto splitter = new QSplitter(Qt::Vertical);
    splitter->addWidget(m_frameTable);
    splitter->addWidget(m_signalTable);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 1);

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->addLayout(toolbar);
    layout->addWidget(splitter, 1);
    layout->addWidget(m_statsLabel);

    m_rateTimer.start();

    /* ── 信号连接 ── */
    connect(m_clearBtn, &QPushButton::clicked, this, &CanBusMonitor::clearFrames);
    connect(m_filterEdit, &QLineEdit::textChanged,
            this, &CanBusMonitor::setFrameIdFilter);

    /* 帧类型过滤: 联合ID过滤 */
    connect(m_typeFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this]() { updateStatsDisplay(); });
}

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

/** @brief 获取当前帧总数 @return 已记录的帧数量 */
int CanBusMonitor::frameCount() const
{
    return m_frameCount;
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

/** @brief 获取唯一帧ID数量 @return 不同帧ID的数量 */
int CanBusMonitor::uniqueFrameIdCount() const
{
    return m_idFrequency.size();
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

/** @brief 设置帧ID过滤器 @param filterText 帧ID过滤文本(如"0x123")，空字符串清除过滤 */
void CanBusMonitor::setFrameIdFilter(const QString& filterText)
{
    m_frameIdFilter = filterText.trimmed();
}

/** @brief 设置DBC解析器用于信号解码显示 @param parser DBC解析器指针 */
void CanBusMonitor::setDbcParser(DbcParser* parser)
{
    m_dbcParser = parser;
}

/** @brief 获取累计监控帧总数 */
quint64 CanBusMonitor::totalFramesMonitored() const
{
    return m_totalFramesMonitored;
}

/** @brief 获取累计错误次数 */
quint64 CanBusMonitor::totalErrors() const
{
    return m_totalErrors;
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

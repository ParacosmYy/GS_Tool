/**
 * @file CanBusMonitor.cpp
 * @brief CAN总线监控面板实现 — 表格显示、颜色编码、自动滚动
 */

#include "connection/can/CanBusMonitor.h"
#include <QHeaderView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTime>
#include <QBrush>
#include <QColor>

CanBusMonitor::CanBusMonitor(QWidget* parent)
    : QWidget(parent)
    , m_frameTable(new QTableWidget(this))
    , m_countLabel(new QLabel(tr("帧数: 0"), this))
    , m_clearBtn(new QPushButton(tr("清空"), this))
    , m_autoScrollCheck(new QCheckBox(tr("自动滚动"), this))
{
    setObjectName("CanBusMonitor");

    /* 表格配置: Time, ID, DLC, Data, Extended, RTR, Count */
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

    /* 顶部工具栏 */
    m_countLabel->setObjectName("canCountLabel");
    m_clearBtn->setObjectName("canClearBtn");
    m_autoScrollCheck->setObjectName("canAutoScrollCheck");
    m_autoScrollCheck->setChecked(true);

    auto toolbar = new QHBoxLayout();
    toolbar->addWidget(m_countLabel, 1);
    toolbar->addWidget(m_autoScrollCheck);
    toolbar->addWidget(m_clearBtn);

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->addLayout(toolbar);
    layout->addWidget(m_frameTable, 1);

    connect(m_clearBtn, &QPushButton::clicked, this, &CanBusMonitor::clearFrames);
}

void CanBusMonitor::addFrame(const CanFrame& frame)
{
    /* 超过上限时移除最旧行 */
    if (m_frameTable->rowCount() >= kMaxRows) {
        m_frameTable->removeRow(0);
    }

    const int row = m_frameTable->rowCount();
    m_frameTable->insertRow(row);
    ++m_frameCount;
    m_idFrequency[frame.id]++;

    /* 时间 */
    auto* timeItem = new QTableWidgetItem(QTime::currentTime().toString("HH:mm:ss.zzz"));
    m_frameTable->setItem(row, 0, timeItem);

    /* 帧ID */
    QString idStr = frame.extended
        ? QStringLiteral("0x%1").arg(frame.id, 8, 16, QLatin1Char('0')).toUpper()
        : QStringLiteral("0x%1").arg(frame.id, 3, 16, QLatin1Char('0')).toUpper();
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
    m_frameTable->setItem(row, 6, new QTableWidgetItem(QString::number(m_frameCount)));

    /* 颜色编码: 扩展帧浅蓝，RTR帧黄色，标准帧白色 */
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

    /* 更新计数标签 */
    m_countLabel->setText(tr("帧数: %1").arg(m_frameCount));
}

void CanBusMonitor::clearFrames()
{
    m_frameTable->setRowCount(0);
    m_frameCount = 0;
    m_idFrequency.clear();
    m_countLabel->setText(tr("帧数: 0"));
}

int CanBusMonitor::frameCount() const
{
    return m_frameCount;
}

QBrush CanBusMonitor::rowBrush(const CanFrame& frame) const
{
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

/**
 * @brief 获取唯一帧ID数量
 * @return 不同帧ID的数量
 */
int CanBusMonitor::uniqueFrameIdCount() const
{
    return m_idFrequency.size();
}

/**
 * @brief 获取统计摘要文本
 * @return 格式化的CAN帧统计信息
 */
QString CanBusMonitor::statisticsSummary() const
{
    QString summary;
    summary += tr("总帧数: %1\n").arg(m_frameCount);
    summary += tr("不同帧ID: %1\n").arg(m_idFrequency.size());

    if (!m_idFrequency.isEmpty()) {
        /* 找到最频繁的帧ID */
        quint32 topId = 0;
        int topCount = 0;
        for (auto it = m_idFrequency.constBegin(); it != m_idFrequency.constEnd(); ++it) {
            if (it.value() > topCount) {
                topId = it.key();
                topCount = it.value();
            }
        }
        summary += tr("最频繁帧ID: 0x%1 (%2次)\n")
                      .arg(topId, 0, 16).arg(topCount);
    }

    return summary.trimmed();
}

/**
 * @brief 设置帧ID过滤器
 * @param filterText 帧ID过滤文本（如 "0x123"），空字符串清除过滤
 */
void CanBusMonitor::setFrameIdFilter(const QString& filterText)
{
    m_frameIdFilter = filterText.trimmed();
}

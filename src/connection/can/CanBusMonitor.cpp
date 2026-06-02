/**
 * @file CanBusMonitor.cpp
 * @brief CAN总线监控面板实现
 */

#include "connection/can/CanBusMonitor.h"
#include <QHeaderView>
#include <QVBoxLayout>
#include <QTime>

CanBusMonitor::CanBusMonitor(QWidget* parent)
    : QWidget(parent)
    , m_frameTable(new QTableWidget(this))
{
    setObjectName("CanBusMonitor");

    m_frameTable->setColumnCount(6);
    m_frameTable->setHorizontalHeaderLabels({
        tr("序号"), tr("帧ID"), tr("DLC"), tr("数据"), tr("类型"), tr("时间")
    });
    m_frameTable->horizontalHeader()->setStretchLastSection(true);
    m_frameTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_frameTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_frameTable);
}

void CanBusMonitor::addFrame(const CanFrame& frame)
{
    const int row = m_frameTable->rowCount();
    m_frameTable->insertRow(row);
    m_frameTable->setItem(row, 0, new QTableWidgetItem(QString::number(++m_frameCount)));
    m_frameTable->setItem(row, 1, new QTableWidgetItem(QStringLiteral("0x%1").arg(frame.id, 0, 16).toUpper()));
    m_frameTable->setItem(row, 2, new QTableWidgetItem(QString::number(frame.dlc)));
    m_frameTable->setItem(row, 3, new QTableWidgetItem(frame.data.toHex(' ').toUpper()));

    QString type;
    if (frame.fd) type += tr("FD ");
    if (frame.extended) type += tr("EXT ");
    if (frame.rtr) type += tr("RTR");
    if (type.isEmpty()) type = tr("标准");
    m_frameTable->setItem(row, 4, new QTableWidgetItem(type));

    m_frameTable->setItem(row, 5, new QTableWidgetItem(QTime::currentTime().toString("HH:mm:ss.zzz")));
    m_frameTable->scrollToBottom();
}

void CanBusMonitor::clearFrames()
{
    m_frameTable->setRowCount(0);
    m_frameCount = 0;
}

int CanBusMonitor::frameCount() const
{
    return m_frameCount;
}

/**
 * @file ModbusScanWidget.cpp
 * @brief Modbus从站地址扫描器实现
 *
 * 逐地址发送探测请求，收集在线从站列表并更新UI。
 */
#include "protocol/modbus/ModbusScanWidget.h"
#include <QVBoxLayout>
#include <QLabel>

ModbusScanWidget::ModbusScanWidget(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("ModbusScanWidget");

    auto* layout = new QVBoxLayout(this);

    // 进度条
    m_progressBar = new QProgressBar(this);
    m_progressBar->setObjectName("scanProgressBar");
    m_progressBar->setRange(0, 247);
    m_progressBar->setTextVisible(true);
    layout->addWidget(m_progressBar);

    // 扫描按钮
    m_scanBtn = new QPushButton(tr("开始扫描"), this);
    m_scanBtn->setObjectName("scanButton");
    layout->addWidget(m_scanBtn);

    // 结果列表
    m_resultList = new QListWidget(this);
    m_resultList->setObjectName("scanResultList");
    layout->addWidget(m_resultList);

    connect(m_scanBtn, &QPushButton::clicked, this, [this]() {
        if (m_scanning) {
            stopScan();
        } else {
            startScan(m_scanFrom, m_scanTo);
        }
    });
}

void ModbusScanWidget::setModbusMaster(ModbusMaster* master) {
    if (m_master) {
        disconnect(m_master, &ModbusMaster::responseReceived,
                   this, &ModbusScanWidget::onResponseReceived);
        disconnect(m_master, &ModbusMaster::timeout,
                   this, &ModbusScanWidget::onScanTimeout);
    }
    m_master = master;
    if (m_master) {
        connect(m_master, &ModbusMaster::responseReceived,
                this, &ModbusScanWidget::onResponseReceived);
        connect(m_master, &ModbusMaster::timeout,
                this, &ModbusScanWidget::onScanTimeout);
    }
}

void ModbusScanWidget::startScan(int from, int to) {
    m_scanFrom    = from;
    m_scanTo      = to;
    m_currentAddr = from;
    m_scanning    = true;

    m_resultList->clear();
    m_progressBar->setRange(from, to);
    m_progressBar->setValue(from);
    updateScanButtonState(true);

    scanNext();
}

void ModbusScanWidget::stopScan() {
    m_scanning = false;
    updateScanButtonState(false);
    emit scanCompleted();
}

QList<int> ModbusScanWidget::foundSlaves() const {
    QList<int> slaves;
    for (int i = 0; i < m_resultList->count(); ++i) {
        QListWidgetItem* item = m_resultList->item(i);
        slaves.append(item->data(Qt::UserRole).toInt());
    }
    return slaves;
}

void ModbusScanWidget::onResponseReceived(const ModbusFrame& frame) {
    int addr = static_cast<int>(frame.slaveAddress);
    QString desc = tr("从站 %1 (功能码: %2)")
                       .arg(addr)
                       .arg(static_cast<int>(frame.function));
    m_resultList->addItem(desc);
    m_resultList->item(m_resultList->count() - 1)
        ->setData(Qt::UserRole, addr);
    emit slaveFound(addr, desc);

    if (m_scanning) { scanNext(); }
}

void ModbusScanWidget::onScanTimeout(int slave, int function) {
    Q_UNUSED(slave)
    Q_UNUSED(function)
    if (m_scanning) { scanNext(); }
}

void ModbusScanWidget::scanNext() {
    if (m_currentAddr > m_scanTo || !m_scanning) {
        stopScan();
        return;
    }

    m_progressBar->setValue(m_currentAddr);
    emit scanProgress(m_currentAddr);

    // 发送探测请求（读1个保持寄存器）
    if (m_master) {
        m_master->readRegisters(m_currentAddr, 0, 1);
    }
    m_currentAddr++;
}

void ModbusScanWidget::updateScanButtonState(bool scanning) {
    if (scanning) {
        m_scanBtn->setText(tr("停止扫描"));
    } else {
        m_scanBtn->setText(tr("开始扫描"));
    }
    m_scanBtn->setEnabled(true);
}
